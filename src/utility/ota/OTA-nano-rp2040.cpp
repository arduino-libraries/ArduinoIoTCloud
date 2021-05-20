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

#if defined(ARDUINO_NANO_RP2040_CONNECT)

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include "OTA.h"

#include <Arduino_DebugUtils.h>

/******************************************************************************
 * FUNCTION DEFINITION
 ******************************************************************************/

int rp2040_connect_onOTARequest(char const * ota_url)
{
  SFU::begin();

  /* Just to be safe delete any remains from previous updates. */
  WiFiStorage.remove("/fs/UPDATE.BIN");
  struct stat st;
  int err = stat("/ota/UPDATE.BIN", &st);
  if (err == 0) {
    remove("/ota/UPDATE.BIN");
  }

  /* TODO: FIXME: Download into NINA then transfer to /fs/UPDATE.BIN */
  uint8_t nina_ota_err_code = 0;
  if (!WiFiStorage.downloadOTA(ota_url, &nina_ota_err_code))
  {
    DEBUG_ERROR("ArduinoIoTCloudTCP::%s error download to nina: %d", __FUNCTION__, nina_ota_err_code);
    return static_cast<int>(OTAError::DownloadFailed);
  }

  FILE* update = fopen("/ota/UPDATE.BIN", "w");

  uint8_t tempbuf[128];

  WiFiStorageFile nina_update = WiFiStorage.open("/fs/UPDATE.BIN");

  while (true) {
    int ret = nina_update.read(tempbuf, sizeof(tempbuf));
    if (ret == 0) {
      break;
    }
    fwrite(tempbuf, ret, 1, update);
  }

  fclose(update);

  /* Perform the reset to reboot to SxU. */
  NVIC_SystemReset();
}

#endif /* ARDUINO_NANO_RP2040_CONNECT */
