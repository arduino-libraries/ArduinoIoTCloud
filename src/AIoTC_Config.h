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

#ifndef ARDUINO_AIOTC_CONFIG_H_
#define ARDUINO_AIOTC_CONFIG_H_

/******************************************************************************
 * USER CONFIGURABLE DEFINES
 ******************************************************************************/

#ifndef OTA_STORAGE_SFU
  #define OTA_STORAGE_SFU         (0)
#endif

#ifndef NTP_USE_RANDOM_PORT
  #define NTP_USE_RANDOM_PORT     (1)
#endif

#ifndef DEBUG_ERROR
# if defined(ARDUINO_AVR_UNO_WIFI_REV2)
#   define DEBUG_ERROR(fmt, ...) Debug.print(DBG_ERROR, fmt, ## __VA_ARGS__)
# else
#   define DEBUG_ERROR(fmt, ...) Debug.print(DBG_ERROR, fmt, ## __VA_ARGS__)
# endif
#endif

#ifndef DEBUG_WARNING
# if defined(ARDUINO_AVR_UNO_WIFI_REV2)
#   define DEBUG_WARNING(fmt, ...)
# else
#   define DEBUG_WARNING(fmt, ...) Debug.print(DBG_WARNING, fmt, ## __VA_ARGS__)
# endif
#endif

#ifndef DEBUG_INFO
# if defined(ARDUINO_AVR_UNO_WIFI_REV2)
#   define DEBUG_INFO(fmt, ...)
# else
#   define DEBUG_INFO(fmt, ...) Debug.print(DBG_INFO, fmt, ## __VA_ARGS__)
# endif
#endif

#ifndef DEBUG_DEBUG
# if defined(ARDUINO_AVR_UNO_WIFI_REV2)
#   define DEBUG_DEBUG(fmt, ...)
# else
#   define DEBUG_DEBUG(fmt, ...) Debug.print(DBG_DEBUG, fmt, ## __VA_ARGS__)
# endif
#endif

#ifndef DEBUG_VERBOSE
# if defined(ARDUINO_AVR_UNO_WIFI_REV2)
#   define DEBUG_VERBOSE(fmt, ...)
# else
#   define DEBUG_VERBOSE(fmt, ...) //Debug.print(DBG_VERBOSE, fmt, ## __VA_ARGS__)
# endif
#endif

#if defined(ARDUINO_AVR_UNO_WIFI_REV2) && !(defined(DEBUG_ERROR) || defined(DEBUG_WARNING) || defined(DEBUG_INFO) || defined(DEBUG_DEBUG) || defined(DEBUG_VERBOSE))
/* Provide defines for constants provided within Arduino_DebugUtils
 * in order to allow older sketches using those constants to still
 * compile.
 */
#  define DBG_NONE    -1
#  define DBG_ERROR    0
#  define DBG_WARNING  1
#  define DBG_INFO     2
#  define DBG_DEBUG    3
#  define DBG_VERBOSE  4
#endif

/******************************************************************************
 * AUTOMATICALLY CONFIGURED DEFINES
 ******************************************************************************/

#if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT) || \
  defined(ARDUINO_AVR_UNO_WIFI_REV2)
  #define OTA_STORAGE_SNU         (1)
#else
  #define OTA_STORAGE_SNU         (0)
#endif

#ifdef ARDUINO_SAMD_MKRGSM1400
  #define OTA_STORAGE_SSU         (1)
#else
  #define OTA_STORAGE_SSU         (0)
#endif

#if defined(ARDUINO_PORTENTA_H7_M7)
  #define OTA_STORAGE_PORTENTA_QSPI   (1)
#else
  #define OTA_STORAGE_PORTENTA_QSPI   (0)
#endif

#if (OTA_STORAGE_SFU || OTA_STORAGE_SSU || OTA_STORAGE_SNU || OTA_STORAGE_PORTENTA_QSPI) && !defined(ARDUINO_AVR_UNO_WIFI_REV2)
  #define OTA_ENABLED             (1)
#else
  #define OTA_ENABLED             (0)
#endif

#if defined(ARDUINO_SAMD_MKRGSM1400) || defined(ARDUINO_SAMD_MKR1000) ||   \
  defined(ARDUINO_SAMD_MKRNB1500) || defined(ARDUINO_PORTENTA_H7_M7)      ||   \
  defined(ARDUINO_PORTENTA_H7_M4) || defined (ARDUINO_NANO_RP2040_CONNECT)
  #define BOARD_HAS_ECCX08
  #define HAS_TCP
#endif

#if defined(ARDUINO_AVR_UNO_WIFI_REV2) || \
  defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT)
  #define BOARD_HAS_OFFLOADED_ECCX08
  #define HAS_TCP
#endif

#if defined(ARDUINO_SAMD_MKRWAN1300) || defined(ARDUINO_SAMD_MKRWAN1310)
  #define HAS_LORA
#endif

#if defined(ARDUINO_ESP8266_ESP12) || defined(ARDUINO_ARCH_ESP32) || defined(ESP8266)
  #define BOARD_ESP
  #define HAS_TCP
#endif

/******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define AIOT_CONFIG_RECONNECTION_RETRY_DELAY_ms      (1000UL)
#define AIOT_CONFIG_MAX_RECONNECTION_RETRY_DELAY_ms (32000UL)
#define AIOT_CONFIG_TIMEOUT_FOR_LASTVALUES_SYNC_ms  (30000UL)
#define AIOT_CONFIG_LASTVALUES_SYNC_MAX_RETRY_CNT      (10UL)

#endif /* ARDUINO_AIOTC_CONFIG_H_ */
