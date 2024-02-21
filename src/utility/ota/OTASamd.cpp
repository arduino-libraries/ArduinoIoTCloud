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

#if defined (ARDUINO_ARCH_SAMD) && OTA_ENABLED

#include "OTA.h"
#include <Arduino_DebugUtils.h>
#include "../watchdog/Watchdog.h"
#include "utility/ota/FlashSHA256.h"

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

  return static_cast<int>(OTAError::None);
#endif /* OTA_STORAGE_SNU */

  (void)ota_url;
  return static_cast<int>(OTAError::DownloadFailed);
}

String samd_getOTAImageSHA256()
{
  /* Calculate the SHA256 checksum over the firmware stored in the flash of the
   * MCU. Note: As we don't know the length per-se we read chunks of the flash
   * until we detect one containing only 0xFF (= flash erased). This only works
   * for firmware updated via OTA and second stage bootloaders (SxU family)
   * because only those erase the complete flash before performing an update.
   * Since the SHA256 firmware image is only required for the cloud servers to
   * perform a version check after the OTA update this is a acceptable trade off.
   * The bootloader is excluded from the calculation and occupies flash address
   * range 0 to 0x2000, total flash size of 0x40000 bytes (256 kByte).
   */
  return FlashSHA256::calc(0x2000, 0x40000 - 0x2000);
}

bool samd_isOTACapable()
{
#if OTA_STORAGE_SNU
  if (String(WiFi.firmwareVersion()) < String("1.4.1")) {
    DEBUG_WARNING("ArduinoIoTCloudTCP::%s In order to be ready for cloud OTA, NINA firmware needs to be >= 1.4.1, current %s", __FUNCTION__, WiFi.firmwareVersion());
    return false;
  } else {
    return true;
  }
#endif
  return false;
}

#endif /* ARDUINO_ARCH_SAMD */
