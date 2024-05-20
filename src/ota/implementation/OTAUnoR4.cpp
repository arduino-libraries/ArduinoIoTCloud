/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <AIoTC_Config.h>

#if defined(ARDUINO_UNOR4_WIFI) && OTA_ENABLED
#include "OTAUnoR4.h"

#include <Arduino_DebugUtils.h>
#include "tls/utility/SHA256.h"
#include "fsp_common_api.h"
#include "r_flash_lp.h"
#include "WiFi.h"

/******************************************************************************
 * DEFINES
 ******************************************************************************/

const char UNOR4OTACloudProcess::UPDATE_FILE_NAME[] = "/update.bin";

static OTACloudProcessInterface::State convertUnor4ErrorToState(int error_code);

UNOR4OTACloudProcess::UNOR4OTACloudProcess(MessageStream *ms)
: OTACloudProcessInterface(ms){

}

OTACloudProcessInterface::State UNOR4OTACloudProcess::resume(Message* msg) {
  return OtaBegin;
}

OTACloudProcessInterface::State UNOR4OTACloudProcess::startOTA() {
  int ota_err = OTAUpdate::OTA_ERROR_NONE;

  // Open fs for ota
  if((ota_err = ota.begin(UPDATE_FILE_NAME)) != OTAUpdate::OTA_ERROR_NONE) {
    DEBUG_VERBOSE("OTAUpdate::begin() failed with %d", ota_err);
    return convertUnor4ErrorToState(ota_err);
  }

  return Fetch;
}

OTACloudProcessInterface::State UNOR4OTACloudProcess::fetch() {
  int ota_err = OTAUpdate::OTA_ERROR_NONE;

  int const ota_download = ota.download(this->context->url,UPDATE_FILE_NAME);
  if (ota_download <= 0) {
    DEBUG_VERBOSE("OTAUpdate::download() failed with %d", ota_download);
    return convertUnor4ErrorToState(ota_download);
  }
  DEBUG_VERBOSE("OTAUpdate::download() %d bytes downloaded", ota_download);

  if ((ota_err = ota.verify()) != OTAUpdate::OTA_ERROR_NONE) {
    DEBUG_VERBOSE("OTAUpdate::verify() failed with %d", ota_err);
    return convertUnor4ErrorToState(ota_err);
  }

  return FlashOTA;
}

OTACloudProcessInterface::State UNOR4OTACloudProcess::flashOTA() {
  int ota_err = OTAUpdate::OTA_ERROR_NONE;

  /* Flash new firmware */
  if ((ota_err = ota.update(UPDATE_FILE_NAME)) != OTAUpdate::OTA_ERROR_NONE) { // This reboots the MCU
    DEBUG_VERBOSE("OTAUpdate::update() failed with %d", ota_err);
    return convertUnor4ErrorToState(ota_err);
  }
}

OTACloudProcessInterface::State UNOR4OTACloudProcess::reboot() {
}

void UNOR4OTACloudProcess::reset() {
}

bool UNOR4OTACloudProcess::isOtaCapable() {
  String const fv = WiFi.firmwareVersion();
  if (fv < String("0.3.0")) {
    return false;
  }
  return true;
}

extern void* __ROM_Start;
extern void* __etext;
extern void* __data_end__;
extern void* __data_start__;

constexpr void* UNOR4OTACloudProcess::appStartAddress() { return &__ROM_Start; }
uint32_t UNOR4OTACloudProcess::appSize() {
  return ((&__etext - &__ROM_Start) + (&__data_end__ - &__data_start__))*sizeof(void*);
}

bool UNOR4OTACloudProcess::appFlashOpen() {
  cfg.data_flash_bgo         = false;
  cfg.p_callback             = nullptr;
  cfg.p_context              = nullptr;
  cfg.p_extend               = nullptr;
  cfg.ipl                    = (BSP_IRQ_DISABLED);
  cfg.irq                    = FSP_INVALID_VECTOR;
  cfg.err_ipl                = (BSP_IRQ_DISABLED);
  cfg.err_irq                = FSP_INVALID_VECTOR;

  fsp_err_t rv = FSP_ERR_UNSUPPORTED;

  rv = R_FLASH_LP_Open(&ctrl,&cfg);
  DEBUG_VERBOSE("Flash open %X", rv);

  return rv == FSP_SUCCESS;
}

bool UNOR4OTACloudProcess::appFlashClose() {
  fsp_err_t rv = FSP_ERR_UNSUPPORTED;
  rv = R_FLASH_LP_Close(&ctrl);
  DEBUG_VERBOSE("Flash close %X", rv);

  return rv == FSP_SUCCESS;
}

static OTACloudProcessInterface::State convertUnor4ErrorToState(int error_code) {
  switch(error_code) {
  case -2:
    return OTACloudProcessInterface::NoOtaStorageFail;
  case -3:
    return OTACloudProcessInterface::OtaStorageInitFail;
  case -4:
    return OTACloudProcessInterface::OtaStorageEndFail;
  case -5:
    return OTACloudProcessInterface::UrlParseErrorFail;
  case -6:
    return OTACloudProcessInterface::ServerConnectErrorFail;
  case -7:
    return OTACloudProcessInterface::HttpHeaderErrorFail;
  case -8:
    return OTACloudProcessInterface::ParseHttpHeaderFail;
  case -9:
    return OTACloudProcessInterface::OtaHeaderLengthFail;
  case -10:
    return OTACloudProcessInterface::OtaHeaderCrcFail;
  case -11:
    return OTACloudProcessInterface::OtaHeaderMagicNumberFail;
  case -12:
    return OTACloudProcessInterface::OtaDownloadFail;
  case -13:
    return OTACloudProcessInterface::OtaHeaderTimeoutFail;
  case -14:
    return OTACloudProcessInterface::HttpResponseFail;
  case -25:
    return OTACloudProcessInterface::LibraryFail;
  case -26:
    return OTACloudProcessInterface::ModemFail;
  default:
    DEBUG_VERBOSE("Unrecognized error code %d", error_code);
    return OTACloudProcessInterface::Fail;
  }
}

#endif // defined(ARDUINO_UNOR4_WIFI) && OTA_ENABLED
