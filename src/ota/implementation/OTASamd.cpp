/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <AIoTC_Config.h>

#if defined(ARDUINO_ARCH_SAMD) && OTA_ENABLED
#include "OTASamd.h"

#include <Arduino_DebugUtils.h>
#if OTA_STORAGE_SNU
#  include <SNU.h>
#  include <WiFiNINA.h> /* WiFiStorage */
#endif

SAMDOTACloudProcess::SAMDOTACloudProcess(MessageStream *ms)
: OTACloudProcessInterface(ms){

}

OTACloudProcessInterface::State SAMDOTACloudProcess::resume(Message* msg) {
  return OtaBegin;
}

OTACloudProcessInterface::State SAMDOTACloudProcess::startOTA() {
  reset();
  return Fetch;
}

OTACloudProcessInterface::State SAMDOTACloudProcess::fetch() {
#if OTA_STORAGE_SNU
  uint8_t nina_ota_err_code = 0;
  if (!WiFiStorage.downloadOTA(this->context->url, &nina_ota_err_code)) {
    DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s error download to nina: %d", __FUNCTION__, nina_ota_err_code);
    switch(static_cast<ninaOTAError>(nina_ota_err_code)) {
    case ninaOTAError::Open:
      return ErrorOpenUpdateFileFail;
    case ninaOTAError::Length:
      return OtaDownloadFail;
    case ninaOTAError::CRC:
      return OtaHeaderCrcFail;
    case ninaOTAError::Rename:
      return ErrorRenameFail;
    default:
      return OtaDownloadFail;
    }
  }
#endif // OTA_STORAGE_SNU

  return FlashOTA;
}

OTACloudProcessInterface::State SAMDOTACloudProcess::flashOTA() {
  return Reboot;
}

OTACloudProcessInterface::State SAMDOTACloudProcess::reboot() {
  NVIC_SystemReset();
}

void SAMDOTACloudProcess::reset() {
#if OTA_STORAGE_SNU
  WiFiStorage.remove("/fs/UPDATE.BIN.LZSS");
  WiFiStorage.remove("/fs/UPDATE.BIN.LZSS.TMP");
#endif // OTA_STORAGE_SNU
}

bool SAMDOTACloudProcess::isOtaCapable() {
#if OTA_STORAGE_SNU
  if (strcmp(WiFi.firmwareVersion(), "1.4.1") < 0) {
    DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s In order to be ready for cloud OTA, NINA firmware needs to be >= 1.4.1, current %s", __FUNCTION__, WiFi.firmwareVersion());
    return false;
  } else {
    return true;
  }
#endif
  return false;
}

extern void* __text_start__;
extern void* __etext;
extern void* __data_end__;
extern void* __data_start__;

void* SAMDOTACloudProcess::appStartAddress() { return &__text_start__; }

uint32_t SAMDOTACloudProcess::appSize() {
  return ((&__etext - &__text_start__) + (&__data_end__ - &__data_start__))*sizeof(void*);
}

#endif // defined(ARDUINO_ARCH_SAMD) && OTA_ENABLED