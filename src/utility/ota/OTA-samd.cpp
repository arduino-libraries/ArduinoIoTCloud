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

#ifdef ARDUINO_ARCH_SAMD

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include "OTA.h"

#include <Arduino_DebugUtils.h>

#include "../watchdog/Watchdog.h"

#if OTA_STORAGE_SNU
#  include <SNU.h>
#  include <WiFiNINA.h> /* WiFiStorage */
#endif

/******************************************************************************
 * FUNCTION DEFINITION
 ******************************************************************************/

int samd_onOTARequest(char const * ota_url)
{
  watchdog_reset();

#if OTA_STORAGE_SNU
  /* Just to be safe delete any remains from previous updates. */
  WiFiStorage.remove("/fs/UPDATE.BIN.LZSS");
  WiFiStorage.remove("/fs/UPDATE.BIN.LZSS.TMP");

  watchdog_reset();

  /* Trigger direct download to NINA module. */
  uint8_t nina_ota_err_code = 0;
  if (!WiFiStorage.downloadOTA(ota_url, &nina_ota_err_code))
  {
    DEBUG_ERROR("ArduinoIoTCloudTCP::%s error download to nina: %d", __FUNCTION__, nina_ota_err_code);
    return static_cast<int>(OTAError::DownloadFailed);
  }

  /* Perform the reset to reboot to SxU. */
  NVIC_SystemReset();
#endif /* OTA_STORAGE_SNU */
}

#endif /* ARDUINO_ARCH_SAMD */
