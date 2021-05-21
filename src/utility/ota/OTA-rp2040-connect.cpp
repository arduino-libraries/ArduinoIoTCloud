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

#ifdef ARDUINO_NANO_RP2040_CONNECT

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include "OTA.h"

#include <Arduino_DebugUtils.h>

/* This library resides within the Nano RP2040 Connect Arduino Core
 * and provides a 2nd stage bootloader which upon booting checks
 * for the existence of a firmware update image and then performs
 * a firmware update.
 */
#include <SFU.h>

/* The Arduino Nano RP2040 Connect has a NINA-W102 WiFi module
 * mounted for connectivity. Therefore its library needs to be
 * included here in order to access the WiFi module for downloading
 * the firmware update image.
 */
#include <WiFiNINA.h>

#include "../watchdog/Watchdog.h"


#include "mbed.h"
#include "FlashIAPBlockDevice.h"
#include "FATFileSystem.h"

/******************************************************************************
 * FUNCTION DEFINITION
 ******************************************************************************/

int nano_rp2040_connect_onOTARequest(char const * ota_url)
{
  int err = 0;
  
  mbed_watchdog_reset();

  /* As a first step prepare everything for the pending
   * firmware image download. Initialize the flash,
   * mount a filesystem and clean-up any left-overs from
   * previous (and possibly failed) update attempts.
   */

  FlashIAPBlockDevice sd(XIP_BASE + 0x100000, 0x100000);
  if ((err = sd.init()) < 0)
  {
    DEBUG_ERROR("%s: sd.init() failed with %d", __FUNCTION__, err);
    return err;
  }

  mbed_watchdog_reset();

  mbed::FATFileSystem fs("ota");
  if ((err = fs.mount(&sd)) != 0)
  {
     DEBUG_ERROR("%s: fs.mount() failed with %d", __FUNCTION__, err);
     return err;
  }

  mbed_watchdog_reset();

  remove("/ota/UPDATE.BIN");
  remove("/ota/UPDATE.BIN.LZSS");
  remove("/ota/UPDATE.BIN.LZSS.TMP");

  mbed_watchdog_reset();

  FILE * file = fopen("/ota/UPDATE.BIN.LZSS.TMP", "wb");
  if (!file)
  {
    DEBUG_ERROR("%s: fopen() failed", __FUNCTION__);
    fclose(file);
    return errno;
  }

  /* Now its time to actually download the firmware
   * image from the server and store it on the filesystem.
   */
  //WiFiSSLClient client;
  WiFiClient client;

//  if (!client.connect("api2.arduino.cc", 443))
  if (!client.connect("107-systems.org", 80))
  {
    DEBUG_ERROR("%s: Connection failure with OTA storage server", __FUNCTION__, err);
    return -1; /* TODO: Implement better error codes. */
  }

  mbed_watchdog_reset();

/*
  client.println("GET iot/ota/8ea3d719-0df0-4a7f-b469-896d61fe42db HTTP/1.1");
  client.println("Host: api2.arduino.cc");
  client.println("Connection: close");
  client.println();
*/

  client.println("GET ota/rp2040-led-red.bin HTTP/1.1");
  client.println("Host: 107-systems.org");
  client.println("Connection: close");
  client.println();

  String http_header;
  bool is_header_complete = false;

  while (client.available())
  {
    mbed_watchdog_reset();

    char const c = client.read();

    if(!is_header_complete)
    {
      http_header += c;
      if (http_header.endsWith("\r\n\r\n"))
        is_header_complete = true;
    }
    else
    {
      if (fwrite(&c, 1, sizeof(c), file) != sizeof(c))
      {
        DEBUG_ERROR("%s: Writing of firmware image to flash failed", __FUNCTION__);
        return -2; /* TODO: Find better error codes. */
      }
    }
  }

  int const file_len = ftell(file);
  DEBUG_DEBUG("%s: %d bytes received", __FUNCTION__, file_len);

  fclose(file);

  /* TODO: Check CRC and header data and decompress. */

  return 0;
}

#endif /* ARDUINO_NANO_RP2040_CONNECT */
