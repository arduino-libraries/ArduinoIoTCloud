/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "AIoTC_Config.h"
#if defined(BOARD_STM32H7) && OTA_ENABLED
#include "OTASTM32H7.h"

#include "utility/watchdog/Watchdog.h"
#include <STM32H747_System.h>

static bool findProgramLength(DIR * dir, uint32_t & program_length);

const char STM32H7OTACloudProcess::UPDATE_FILE_NAME[] = "/fs/UPDATE.BIN";

STM32H7OTACloudProcess::STM32H7OTACloudProcess(MessageStream *ms, Client* client)
: OTADefaultCloudProcessInterface(ms, client)
, decompressed(nullptr)
, _bd_raw_qspi(nullptr)
, _program_length(0)
, _bd(nullptr)
, _fs(nullptr) {

}

STM32H7OTACloudProcess::~STM32H7OTACloudProcess() {
  if(decompressed != nullptr) {
    fclose(decompressed);
    decompressed = nullptr;
  }

  storageClean();
}

OTACloudProcessInterface::State STM32H7OTACloudProcess::resume(Message* msg) {
  return OtaBegin;
}

void STM32H7OTACloudProcess::update() {
  OTADefaultCloudProcessInterface::update();
  watchdog_reset(); // FIXME this should npot be performed here
}

int STM32H7OTACloudProcess::writeFlash(uint8_t* const buffer, size_t len) {
  if (decompressed == nullptr) {
    return -1;
  }
  return fwrite(buffer, sizeof(uint8_t), len, decompressed);
}

OTACloudProcessInterface::State STM32H7OTACloudProcess::startOTA() {
  if (!isOtaCapable()) {
    return NoCapableBootloaderFail;
  }

  /* Initialize the QSPI memory for OTA handling. */
  if (!storageInit()) {
    return OtaStorageInitFail;
  }

  // this could be useless, since we are writing over it
  remove(UPDATE_FILE_NAME);

  decompressed = fopen(UPDATE_FILE_NAME, "wb");

  // start the download if the setup for ota storage is successful
  return OTADefaultCloudProcessInterface::startOTA();
}


OTACloudProcessInterface::State STM32H7OTACloudProcess::flashOTA() {
  fclose(decompressed);
  decompressed = nullptr;

  /* Schedule the firmware update. */
  if(!storageOpen()) {
    return OtaStorageOpenFail;
  }

  // this sets the registries in RTC to load the firmware from the storage selected at the next reboot
  STM32H747::writeBackupRegister(RTCBackup::DR0, 0x07AA);
  STM32H747::writeBackupRegister(RTCBackup::DR1, storage);
  STM32H747::writeBackupRegister(RTCBackup::DR2, data_offset);
  STM32H747::writeBackupRegister(RTCBackup::DR3, _program_length);

  return Reboot;
}

OTACloudProcessInterface::State STM32H7OTACloudProcess::reboot() {
  // TODO save information about the progress reached in the ota

  // This command reboots the mcu
  NVIC_SystemReset();

  return Resume; // This won't ever be reached
}

void STM32H7OTACloudProcess::reset() {
  OTADefaultCloudProcessInterface::reset();

  remove(UPDATE_FILE_NAME);

  storageClean();
}

void STM32H7OTACloudProcess::storageClean() {
  DEBUG_VERBOSE(F("storage clean"));

  if(decompressed != nullptr) {
    fclose(decompressed);
    decompressed = nullptr;
  }

  if(_fs != nullptr) {
    _fs->unmount();
    delete _fs;
    _fs = nullptr;
  }

  if(_bd != nullptr) {
    delete _bd;
    _bd = nullptr;
  }
}

bool STM32H7OTACloudProcess::isOtaCapable() {
  #define BOOTLOADER_ADDR   (0x8000000)
  uint32_t bootloader_data_offset = 0x1F000;
  uint8_t* bootloader_data = (uint8_t*)(BOOTLOADER_ADDR + bootloader_data_offset);
  uint8_t currentBootloaderVersion = bootloader_data[1];
  if (currentBootloaderVersion < 22)
    return false;
  else
    return true;
}

bool STM32H7OTACloudProcess::storageInit() {
  int err_mount=1;

  if(_bd_raw_qspi == nullptr) {
    _bd_raw_qspi = mbed::BlockDevice::get_default_instance();

    if (_bd_raw_qspi->init() != QSPIF_BD_ERROR_OK) {
      DEBUG_VERBOSE(F("Error: QSPI init failure."));
      return false;
    }
  }

  if (storage == portenta::QSPI_FLASH_FATFS) {
    _fs = new mbed::FATFileSystem("fs");
    err_mount = _fs->mount(_bd_raw_qspi);
  } else if (storage == portenta::QSPI_FLASH_FATFS_MBR) {
    _bd = new mbed::MBRBlockDevice(_bd_raw_qspi, data_offset);
    _fs = new mbed::FATFileSystem("fs");
    err_mount = _fs->mount(_bd);
  }

  if (!err_mount) {
    return true;
  }
  DEBUG_VERBOSE(F("Error while mounting the filesystem. Err = %d"), err_mount);
  return false;
}

bool STM32H7OTACloudProcess::storageOpen() {
  DIR * dir = NULL;
  if ((dir = opendir("/fs")) != NULL)
  {
    if (findProgramLength(dir, _program_length))
    {
      closedir(dir);
      return true;
    }
    closedir(dir);
  }

  return false;
}

bool findProgramLength(DIR * dir, uint32_t & program_length) {
  struct dirent * entry = NULL;
  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, "UPDATE.BIN") == 0) { // FIXME use constants
      struct stat stat_buf;
      stat("/fs/UPDATE.BIN", &stat_buf);
      program_length = stat_buf.st_size;
      return true;
    }
  }

  return false;
}

// extern uint32_t __stext = ~0;
extern uint32_t __etext;
extern uint32_t _sdata;
extern uint32_t _edata;

void* STM32H7OTACloudProcess::appStartAddress() {
  return (void*)0x8040000;
  // return &__stext;
}

uint32_t STM32H7OTACloudProcess::appSize() {
  return ((&__etext - (uint32_t*)appStartAddress()) + (&_edata - &_sdata))*sizeof(void*);
}


#endif // defined(BOARD_STM32H7) && OTA_ENABLED