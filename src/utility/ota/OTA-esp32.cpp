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
#include <Arduino_ESP_OTA.h>

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

#endif /* ARDUINO_ARCH_ESP32 */
