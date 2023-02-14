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

#include <stm32h7xx_hal_rtc_ex.h>

#include "tls/utility/SHA256.h"

#include "../watchdog/Watchdog.h"

/******************************************************************************
 * EXTERN
 ******************************************************************************/

extern RTC_HandleTypeDef RTCHandle;

/******************************************************************************
 * FUNCTION DEFINITION
 ******************************************************************************/

int portenta_h7_onOTARequest(char const * ota_url, NetworkAdapter iface)
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
  if(iface == NetworkAdapter::ETHERNET) {
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

String portenta_h7_getOTAImageSHA256()
{
  /* The length of the application can be retrieved the same way it was
   * communicated to the bootloader, that is by writing to the non-volatile
   * storage registers of the RTC.
   */
  SHA256         sha256;
  uint32_t const app_start = 0x8040000;
  uint32_t const app_size  = HAL_RTCEx_BKUPRead(&RTCHandle, RTC_BKP_DR3);

  sha256.begin();
  uint32_t b = 0;
  uint32_t bytes_read = 0; for(uint32_t a = app_start;
                               bytes_read < app_size;
                               bytes_read += sizeof(b), a += sizeof(b))
  {
    /* Read the next chunk of memory. */
    memcpy(&b, reinterpret_cast<const void *>(a), sizeof(b));
    /* Feed it to SHA256. */
    sha256.update(reinterpret_cast<uint8_t *>(&b), sizeof(b));
  }
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
  DEBUG_VERBOSE("SHA256: %d bytes (of %d) read", bytes_read, app_size);
  return sha256_str;
}

bool portenta_h7_isOTACapable()
{
  return Arduino_Portenta_OTA::isOtaCapable();
}

#endif /* BOARD_STM32H7 */
