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

#if defined __has_include
  #if __has_include (<Notecard.h>)
    #define HAS_NOTECARD
  #endif
#endif

#include <ArduinoECCX08Config.h>

/******************************************************************************
 * USER CONFIGURABLE DEFINES
 ******************************************************************************/

#ifndef NTP_USE_RANDOM_PORT
  #define NTP_USE_RANDOM_PORT     (1)
#endif

#ifndef DEBUG_ERROR
  #define DEBUG_ERROR(fmt, ...) Debug.print(DBG_ERROR, fmt, ## __VA_ARGS__)
#endif

#ifndef DEBUG_WARNING
  #define DEBUG_WARNING(fmt, ...) Debug.print(DBG_WARNING, fmt, ## __VA_ARGS__)
#endif

#ifndef DEBUG_INFO
  #define DEBUG_INFO(fmt, ...) Debug.print(DBG_INFO, fmt, ## __VA_ARGS__)
#endif

#ifndef DEBUG_DEBUG
  #define DEBUG_DEBUG(fmt, ...) Debug.print(DBG_DEBUG, fmt, ## __VA_ARGS__)
#endif

#ifndef DEBUG_VERBOSE
  #define DEBUG_VERBOSE(fmt, ...) //Debug.print(DBG_VERBOSE, fmt, ## __VA_ARGS__)
#endif

/******************************************************************************
 * AUTOMATICALLY CONFIGURED DEFINES
 ******************************************************************************/

#if !defined(HAS_NOTECARD)

#if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT)
  #define OTA_STORAGE_SNU         (1)
#else
  #define OTA_STORAGE_SNU         (0)
#endif

#if defined(ARDUINO_NANO_RP2040_CONNECT)
  #define OTA_STORAGE_SFU         (1)
#else
  #define OTA_STORAGE_SFU         (0)
#endif

#ifdef ARDUINO_SAMD_MKRGSM1400
  #define OTA_STORAGE_SSU         (1) // OTA_STORAGE_SSU is not implemented yet in OTASamd
#else
  #define OTA_STORAGE_SSU         (0)
#endif

#if defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_NICLA_VISION) || defined(ARDUINO_OPTA) || defined(ARDUINO_GIGA)
  #define OTA_STORAGE_PORTENTA_QSPI   (1)
#else
  #define OTA_STORAGE_PORTENTA_QSPI   (0)
#endif

#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_UNOR4_WIFI)
  #define OTA_STORAGE_ESP         (1)
#endif

#if (OTA_STORAGE_SFU || OTA_STORAGE_SNU || OTA_STORAGE_PORTENTA_QSPI || OTA_STORAGE_ESP)
  #define OTA_ENABLED             (1)
#else
  #define OTA_ENABLED             (0)
#endif

#if defined(ARDUINO_SAMD_MKRGSM1400) || defined(ARDUINO_SAMD_MKR1000) ||   \
  defined(ARDUINO_SAMD_MKRNB1500) || defined(ARDUINO_PORTENTA_H7_M7)      ||   \
  defined (ARDUINO_NANO_RP2040_CONNECT) || defined(ARDUINO_OPTA) || \
  defined(ARDUINO_GIGA)
  #define BOARD_HAS_ECCX08
  #define HAS_TCP
#endif

#if defined(ARDUINO_NICLA_VISION) || defined(ARDUINO_PORTENTA_C33)
  #define BOARD_HAS_SE050
  #define HAS_TCP
#endif

#if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT)
  #define BOARD_HAS_OFFLOADED_ECCX08
  #define HAS_TCP
#endif

#if defined(ARDUINO_SAMD_MKRWAN1300) || defined(ARDUINO_SAMD_MKRWAN1310)
  #define HAS_LORA
#endif

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
  #define BOARD_ESP
  #define BOARD_HAS_SECRET_KEY
  #define HAS_TCP
#endif

#if defined(ARDUINO_NANO_RP2040_CONNECT)
  #define BEAR_SSL_CLIENT_IBUF_SIZE (16384 + 325) // Allows download from storage API
#endif

#if defined(ARDUINO_EDGE_CONTROL)
  #define BOARD_HAS_SECRET_KEY
  #define HAS_TCP
#endif

#if defined(ARDUINO_UNOR4_WIFI)
  #define BOARD_HAS_SOFTSE
  #define BOARD_HAS_SECRET_KEY
  #define HAS_TCP
#endif

#if defined(BOARD_HAS_SOFTSE) || defined(BOARD_HAS_OFFLOADED_ECCX08) || defined(BOARD_HAS_ECCX08) || defined(BOARD_HAS_SE050)
  #define BOARD_HAS_SECURE_ELEMENT
#endif

#endif // HAS_NOTECARD

#if defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_NICLA_VISION) || defined(ARDUINO_OPTA) || defined(ARDUINO_GIGA)
  #define BEAR_SSL_CLIENT_IBUF_SIZE (16384 + 325) // Allows download from storage API
  #define BOARD_STM32H7
#endif

/******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#if defined(HAS_LORA)
  #define AIOT_CONFIG_LPWAN_UPDATE_RETRY_DELAY_ms                 (10000UL)
#endif

#if defined(HAS_NOTECARD) || defined(HAS_TCP)
  #define AIOT_CONFIG_RECONNECTION_RETRY_DELAY_ms                  (1000UL)
  #define AIOT_CONFIG_MAX_RECONNECTION_RETRY_DELAY_ms             (32000UL)

  #if defined(HAS_NOTECARD)
    // 10x the standard delays for Notecard
    #define AIOT_CONFIG_THING_ID_REQUEST_RETRY_DELAY_ms           (20000UL)
    #define AIOT_CONFIG_MAX_THING_ID_REQUEST_RETRY_DELAY_ms      (320000UL)
  #else
    #define AIOT_CONFIG_THING_ID_REQUEST_RETRY_DELAY_ms            (2000UL)
    #define AIOT_CONFIG_MAX_THING_ID_REQUEST_RETRY_DELAY_ms       (32000UL)
  #endif

  #define AIOT_CONFIG_THING_ID_REQUEST_MAX_RETRY_CNT                 (10UL)

  #define AIOT_CONFIG_DEVICE_REGISTERED_RETRY_DELAY_k                (10UL)
  #define AIOT_CONFIG_MAX_DEVICE_REGISTERED_RETRY_DELAY_k            (40UL)

  #define AIOT_CONFIG_TIMEOUT_FOR_LASTVALUES_SYNC_ms              (30000UL)
  #define AIOT_CONFIG_LASTVALUES_SYNC_MAX_RETRY_CNT                  (10UL)
#endif

#define AIOT_CONFIG_LIB_VERSION "2.1.2"

#endif /* ARDUINO_AIOTC_CONFIG_H_ */
