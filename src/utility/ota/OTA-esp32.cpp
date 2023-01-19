/*
   This file is part of ArduinoIoTCloud.

   Copyright 2020 ARDUINO SA (http://www.arduino.cc/)

   This software is released under the GNU General Public License version 3,
   which covers the main part of arduino-cli.
   The terms of this license can be found at:
   https://www.gnu.org/licenses/gpl-3.0.en.html

   You can be released from the requirements of the above licenses by purchasing
   a commercial license. Buying such a license is mandatory if you want to modify or
   otherwise use the software for commercial activities involving the Arduino
   software without disclosing the source code of your own applications. To purchase
   a commercial license, send an email to license@arduino.cc.
*/

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <AIoTC_Config.h>

#if defined ARDUINO_ARCH_ESP32 && OTA_ENABLED

#include "OTA.h"
#include <Arduino_DebugUtils.h>
#include <Arduino_ESP32_OTA.h>
#include "tls/utility/SHA256.h"

#include <esp_ota_ops.h>

/******************************************************************************
 * FUNCTION DEFINITION
 ******************************************************************************/

int esp32_onOTARequest(char const * ota_url)
{
  Arduino_ESP32_OTA::Error ota_err = Arduino_ESP32_OTA::Error::None;
  Arduino_ESP32_OTA ota;

  /* Initialize the board for OTA handling. */
  if ((ota_err = ota.begin()) != Arduino_ESP32_OTA::Error::None)
  {
    DEBUG_ERROR("Arduino_ESP32_OTA::begin() failed with %d", static_cast<int>(ota_err));
    return static_cast<int>(ota_err);
  }

  /* Download the OTA file from the web storage location. */
  int const ota_download = ota.download(ota_url);
  if (ota_download <= 0)
  {
    DEBUG_ERROR("Arduino_ESP_OTA::download() failed with %d", ota_download);
    return ota_download;
  }
  DEBUG_VERBOSE("Arduino_ESP_OTA::download() %d bytes downloaded", static_cast<int>(ota_download));

  /* Verify update integrity and apply */
  if ((ota_err = ota.update()) != Arduino_ESP32_OTA::Error::None)
  {
    DEBUG_ERROR("Arduino_ESP_OTA::update() failed with %d", static_cast<int>(ota_err));
    return static_cast<int>(ota_err);
  }

  /* Perform the reset to reboot */
  ota.reset();

  return static_cast<int>(OTAError::None);
}

String esp32_getOTAImageSHA256()
{
  const esp_partition_t *running = esp_ota_get_running_partition();
  if (!running) {
    DEBUG_ERROR("ESP32::SHA256 Running partition could not be found");
    return String();
  }

  uint8_t *b = (uint8_t*)malloc(SPI_FLASH_SEC_SIZE);
  if(b == nullptr) {
    DEBUG_ERROR("ESP32::SHA256 Not enough memory to allocate buffer");
    return String();
  }

  SHA256         sha256;
  uint32_t const app_start = running->address;
  uint32_t const app_size  = ESP.getSketchSize();
  uint32_t       read_bytes = 0;

  sha256.begin();
  for(uint32_t a = app_start; read_bytes < app_size; )
  {
    /* Check if we are reading last sector and compute used size */
    uint32_t const read_size = read_bytes + SPI_FLASH_SEC_SIZE < app_size ? SPI_FLASH_SEC_SIZE : app_size - read_bytes;

    /* Use always 4 bytes aligned reads */
    if (!ESP.flashRead(a, reinterpret_cast<uint32_t*>(b), (read_size + 3) & ~3)) {
      DEBUG_ERROR("ESP32::SHA256 Could not read data from flash");
      return String();
    }
    sha256.update(b, read_size);
    a += read_size;
    read_bytes += read_size;
  }
  free(b);

  /* Retrieve the final hash string. */
  uint8_t sha256_hash[SHA256::HASH_SIZE] = {0};
  sha256.finalize(sha256_hash);
  String sha256_str;
  std::for_each(sha256_hash,
                sha256_hash + SHA256::HASH_SIZE,
                [&sha256_str](uint8_t const elem)
                {
                  char buf[4];
                  snprintf(buf, 4, "%02X", elem);
                  sha256_str += buf;
                });
  DEBUG_VERBOSE("SHA256: %d bytes (of %d) read", read_bytes, app_size);
  return sha256_str;
}

bool esp32_isOTACapable()
{
  /* NOTE: here is possible to check if current partition scheme is OTA compatible */
  return true;
}

#endif /* ARDUINO_ARCH_ESP32 */
