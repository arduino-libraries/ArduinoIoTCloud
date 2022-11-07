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

#ifdef BOARD_STM32H7

#include "OTA.h"

#include <Arduino_DebugUtils.h>
#include <Arduino_Portenta_OTA.h>
#include <Arduino_ConnectionHandler.h>

#include "../watchdog/Watchdog.h"

/******************************************************************************
 * FUNCTION DEFINITION
 ******************************************************************************/

int portenta_h7_onOTARequest(char const * ota_url, const bool use_ethernet)
{
  watchdog_reset();

  Arduino_Portenta_OTA::Error ota_portenta_err = Arduino_Portenta_OTA::Error::None;
  /* Use 2nd partition of QSPI (1st partition contains WiFi firmware) */
  Arduino_Portenta_OTA_QSPI ota_portenta_qspi(QSPI_FLASH_FATFS_MBR, 2);

#if defined (ARDUINO_PORTENTA_OTA_HAS_WATCHDOG_FEED)
  ota_portenta_qspi.setFeedWatchdogFunc(watchdog_reset);
#endif

  watchdog_reset();

  /* Initialize the QSPI memory for OTA handling. */
  if((ota_portenta_err = ota_portenta_qspi.begin()) != Arduino_Portenta_OTA::Error::None) {
    DEBUG_ERROR("Arduino_Portenta_OTA_QSPI::begin() failed with %d", static_cast<int>(ota_portenta_err));
    return static_cast<int>(ota_portenta_err);
  }

  watchdog_reset();

  /* Just to be safe delete any remains from previous updates. */
  remove("/fs/UPDATE.BIN");
  remove("/fs/UPDATE.BIN.LZSS");

  watchdog_reset();

  /* Download the OTA file from the web storage location. */
  MbedSocketClass * download_socket = static_cast<MbedSocketClass*>(&WiFi);
#if defined (BOARD_HAS_ETHERNET)
  if(use_ethernet) {
    download_socket = static_cast<MbedSocketClass*>(&Ethernet);
  }
#endif
  int const ota_portenta_qspi_download_ret_code = ota_portenta_qspi.download(ota_url, true /* is_https */, download_socket);
  DEBUG_VERBOSE("Arduino_Portenta_OTA_QSPI::download(%s) returns %d", ota_url, ota_portenta_qspi_download_ret_code);

  watchdog_reset();

  /* Decompress the LZSS compressed OTA file. */
  int const ota_portenta_qspi_decompress_ret_code = ota_portenta_qspi.decompress();
  DEBUG_VERBOSE("Arduino_Portenta_OTA_QSPI::decompress() returns %d", ota_portenta_qspi_decompress_ret_code);
  if (ota_portenta_qspi_decompress_ret_code < 0)
  {
    DEBUG_ERROR("Arduino_Portenta_OTA_QSPI::decompress() failed with %d", ota_portenta_qspi_decompress_ret_code);
    return ota_portenta_qspi_decompress_ret_code;
  }

  watchdog_reset();

  /* Schedule the firmware update. */
  if((ota_portenta_err = ota_portenta_qspi.update()) != Arduino_Portenta_OTA::Error::None) {
    DEBUG_ERROR("Arduino_Portenta_OTA_QSPI::update() failed with %d", static_cast<int>(ota_portenta_err));
    return static_cast<int>(ota_portenta_err);
  }

  /* Perform the reset to reboot - then the bootloader performs the actual application update. */
  NVIC_SystemReset();
}

#endif /* BOARD_STM32H7 */
