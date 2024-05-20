/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <AIoTC_Config.h>

#if defined(ARDUINO_NANO_RP2040_CONNECT) && OTA_ENABLED
#include <SFU.h>
#include "OTANanoRP2040.h"
#include <Arduino_DebugUtils.h>
#include "mbed.h"
#include "utility/watchdog/Watchdog.h"

#define SD_MOUNT_PATH           "ota"
#define FULL_UPDATE_FILE_PATH   "/ota/UPDATE.BIN"

const char NANO_RP2040OTACloudProcess::UPDATE_FILE_NAME[] = FULL_UPDATE_FILE_PATH;


NANO_RP2040OTACloudProcess::NANO_RP2040OTACloudProcess(MessageStream *ms, Client* client)
: OTADefaultCloudProcessInterface(ms, client)
, flash((uint32_t)appStartAddress() + 0xF00000, 0x100000) // TODO make this numbers a constant
, decompressed(nullptr)
, fs(nullptr) {
}

NANO_RP2040OTACloudProcess::~NANO_RP2040OTACloudProcess() {
  close_fs();
}

OTACloudProcessInterface::State NANO_RP2040OTACloudProcess::resume(Message* msg) {
  return OtaBegin;
}

int NANO_RP2040OTACloudProcess::writeFlash(uint8_t* const buffer, size_t len) {
  if(decompressed == nullptr) {
    DEBUG_VERBOSE("writing on a file that is not open"); // FIXME change log message
    return 0;
  }
  return fwrite(buffer, sizeof(uint8_t), len, decompressed);
}

OTACloudProcessInterface::State NANO_RP2040OTACloudProcess::startOTA() {
  int err = -1;
  if ((err = flash.init()) < 0) {
    DEBUG_VERBOSE("%s: flash.init() failed with %d", __FUNCTION__, err);
    return OtaStorageInitFail;
  }

  flash.erase((uint32_t)appStartAddress() + 0xF00000, 0x100000);

  fs = new mbed::FATFileSystem(SD_MOUNT_PATH); // FIXME can this be allocated in the stack?
  if ((err = fs->reformat(&flash)) != 0) {
    DEBUG_VERBOSE("%s: fs.reformat() failed with %d", __FUNCTION__, err);
    return ErrorReformatFail;
  }

  decompressed = fopen(UPDATE_FILE_NAME, "wb"); // TODO make this a constant
  if (!decompressed) {
    DEBUG_VERBOSE("%s: fopen() failed", __FUNCTION__);
    fclose(decompressed);
    return ErrorOpenUpdateFileFail;
  }

  // we start the download here
  return OTADefaultCloudProcessInterface::startOTA();;
}


OTACloudProcessInterface::State NANO_RP2040OTACloudProcess::flashOTA() {
  int err = 0;
  if((err = close_fs()) != 0) {
    return ErrorUnmountFail;
  }

  return Reboot;
}

OTACloudProcessInterface::State NANO_RP2040OTACloudProcess::reboot() {
  mbed_watchdog_trigger_reset();
  /* If watchdog is enabled we should not reach this point */
  NVIC_SystemReset();

  return Resume; // This won't ever be reached
}

void NANO_RP2040OTACloudProcess::reset() {
  OTADefaultCloudProcessInterface::reset();

  close_fs();
}

int NANO_RP2040OTACloudProcess::close_fs() {
  int err = 0;

  if(decompressed != nullptr) {
    fclose(decompressed);
    decompressed = nullptr;
  }

  if (fs != nullptr && (err = fs->unmount()) != 0) {
    DEBUG_VERBOSE("%s: fs.unmount() failed with %d", __FUNCTION__, err);
  } else {
    delete fs;
    fs = nullptr;
  }

  return err;
}

bool NANO_RP2040OTACloudProcess::isOtaCapable() {
  return true;
}

// extern void* __stext;
extern uint32_t __flash_binary_end;


void* NANO_RP2040OTACloudProcess::appStartAddress() {
  // return &__flash_binary_start;
  return (void*)XIP_BASE;
}
uint32_t NANO_RP2040OTACloudProcess::appSize() {
  return (&__flash_binary_end - (uint32_t*)appStartAddress())*sizeof(void*);
}

#endif // defined(ARDUINO_NANO_RP2040_CONNECT) && OTA_ENABLED