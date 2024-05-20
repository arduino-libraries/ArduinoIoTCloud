/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "AIoTC_Config.h"
#if defined(ARDUINO_ARCH_ESP32) && OTA_ENABLED
#include "OTAEsp32.h"
#include <esp_ota_ops.h>
#include <Update.h>

ESP32OTACloudProcess::ESP32OTACloudProcess(MessageStream *ms, Client* client)
: OTADefaultCloudProcessInterface(ms), rom_partition(nullptr) {

}


OTACloudProcessInterface::State ESP32OTACloudProcess::resume(Message* msg) {
  return OtaBegin;
}

OTACloudProcessInterface::State ESP32OTACloudProcess::startOTA() {
  if(Update.isRunning()) {
    Update.abort();
    DEBUG_VERBOSE("%s: Aborting running update", __FUNCTION__);
  }

  if(!Update.begin(UPDATE_SIZE_UNKNOWN)) {
    DEBUG_VERBOSE("%s: failed to initialize flash update", __FUNCTION__);
    return OtaStorageInitFail;
  }

  return OTADefaultCloudProcessInterface::startOTA();
}

OTACloudProcessInterface::State ESP32OTACloudProcess::flashOTA() {

  if (!Update.end(true)) {
    DEBUG_VERBOSE("%s: Failure to apply OTA update", __FUNCTION__);
    return OtaStorageEndFail;
  }

  return Reboot;
}

OTACloudProcessInterface::State ESP32OTACloudProcess::reboot() {
  ESP.restart();

  return Idle; // we won't reach this
}

int ESP32OTACloudProcess::writeFlash(uint8_t* const buffer, size_t len) {
  return Update.write(buffer, len);
}

bool ESP32OTACloudProcess::isOtaCapable() {
  const esp_partition_t * ota_0  = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, NULL);
  const esp_partition_t * ota_1  = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_1, NULL);
  return ((ota_0 != nullptr) && (ota_1 != nullptr));
}

void* ESP32OTACloudProcess::appStartAddress() {
  return nullptr;
}
uint32_t ESP32OTACloudProcess::appSize() {
  return ESP.getSketchSize();
}

bool ESP32OTACloudProcess::appFlashOpen() {
  rom_partition = esp_ota_get_running_partition();

  if(rom_partition == nullptr) {
    return false;
  }

  return true;
}

void ESP32OTACloudProcess::calculateSHA256(SHA256& sha256_calc) {
  if(!appFlashOpen()) {
    return; // TODO error reporting
  }

  sha256_calc.begin();

  uint8_t b[SPI_FLASH_SEC_SIZE];
  if(b == nullptr) {
    DEBUG_VERBOSE("ESP32::SHA256 Not enough memory to allocate buffer");
    return; // TODO error reporting
  }

  uint32_t       read_bytes = 0;
  uint32_t const app_size   = ESP.getSketchSize();
  for(uint32_t a = rom_partition->address; read_bytes < app_size; ) {
    /* Check if we are reading last sector and compute used size */
    uint32_t const read_size = read_bytes + SPI_FLASH_SEC_SIZE < app_size ?
      SPI_FLASH_SEC_SIZE : app_size - read_bytes;

    /* Use always 4 bytes aligned reads */
    if (!ESP.flashRead(a, reinterpret_cast<uint32_t*>(b), (read_size + 3) & ~3)) {
      DEBUG_VERBOSE("ESP32::SHA256 Could not read data from flash");
      return;
    }
    sha256_calc.update(b, read_size);
    a += read_size;
    read_bytes += read_size;
  }

  appFlashClose();
}

#endif // defined(ARDUINO_ARCH_ESP32) && OTA_ENABLED