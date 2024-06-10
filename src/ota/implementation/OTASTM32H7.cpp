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
#include <STM32H747_System.h>

STM32H7OTACloudProcess::STM32H7OTACloudProcess(MessageStream *ms, Client* client)
: OTADefaultCloudProcessInterface(ms, client)
, decompressed(nullptr)
, _bd_raw_qspi(nullptr)
, _bd(nullptr)
, _fs(nullptr)
, _filename("/" + String(STM32H747OTA::FOLDER) + "/" + String(STM32H747OTA::NAME)) {

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
  remove(_filename.c_str());

  decompressed = fopen(_filename.c_str(), "wb");

  if(decompressed == nullptr) {
    return ErrorOpenUpdateFileFail;
  }
  // start the download if the setup for ota storage is successful
  return OTADefaultCloudProcessInterface::startOTA();
}


OTACloudProcessInterface::State STM32H7OTACloudProcess::flashOTA() {
  fclose(decompressed);
  decompressed = nullptr;

  uint32_t updateLength = 0;

  /* Schedule the firmware update. */
  if(!findProgramLength(updateLength)) {
    return OtaStorageOpenFail;
  }

  storageClean();

  // this sets the registries in RTC to load the firmware from the storage selected at the next reboot
  STM32H747::writeBackupRegister(RTCBackup::DR0, STM32H747OTA::MAGIC);
  STM32H747::writeBackupRegister(RTCBackup::DR1, STM32H747OTA::STORAGE_TYPE);
  STM32H747::writeBackupRegister(RTCBackup::DR2, STM32H747OTA::PARTITION);
  STM32H747::writeBackupRegister(RTCBackup::DR3, updateLength);

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

  remove(_filename.c_str());

  storageClean();
}

void STM32H7OTACloudProcess::storageClean() {
  DEBUG_VERBOSE(F("storage clean"));

  if(decompressed != nullptr) {
    int res = fclose(decompressed);
    DEBUG_VERBOSE("error on fclose %d", res);

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

  _bd = new mbed::MBRBlockDevice(_bd_raw_qspi, STM32H747OTA::PARTITION);
  _fs = new mbed::FATFileSystem(STM32H747OTA::FOLDER);
  err_mount = _fs->mount(_bd);

  if (!err_mount) {
    return true;
  }
  DEBUG_VERBOSE(F("Error while mounting the filesystem. Err = %d"), err_mount);
  return false;
}

bool STM32H7OTACloudProcess::findProgramLength(uint32_t & program_length) {
  DIR * dir = NULL;
  struct dirent * entry = NULL;
  String dirName = "/" + String(STM32H747OTA::FOLDER);
  bool found = false;

  if ((dir = opendir(dirName.c_str())) == NULL) {
    return false;
  }

  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, STM32H747OTA::NAME) == 0) {
      struct stat stat_buf;
      stat(_filename.c_str(), &stat_buf);
      program_length = stat_buf.st_size;
      found = true;
    }
  }
  closedir(dir);
  return found;
}

extern uint32_t __etext;
extern uint32_t _sdata;
extern uint32_t _edata;

void* STM32H7OTACloudProcess::appStartAddress() {
  return (void*)0x8040000;
}

uint32_t STM32H7OTACloudProcess::appSize() {
  return ((&__etext - (uint32_t*)appStartAddress()) + (&_edata - &_sdata))*sizeof(void*);
}


#endif // defined(BOARD_STM32H7) && OTA_ENABLED