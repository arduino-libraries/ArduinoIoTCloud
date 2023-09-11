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

#if OTA_ENABLED

#include "OTA.h"
#include <Arduino_DebugUtils.h>

/******************************************************************************
 * FUNCTION DECLARATION
 ******************************************************************************/

#ifdef ARDUINO_ARCH_SAMD
int samd_onOTARequest(char const * url);
String samd_getOTAImageSHA256();
bool samd_isOTACapable();
#endif

#ifdef ARDUINO_NANO_RP2040_CONNECT
int rp2040_connect_onOTARequest(char const * url);
String rp2040_connect_getOTAImageSHA256();
bool rp2040_connect_isOTACapable();
#endif

#ifdef BOARD_STM32H7
int portenta_h7_onOTARequest(char const * url, NetworkAdapter iface);
String portenta_h7_getOTAImageSHA256();
void portenta_h7_setNetworkAdapter(NetworkAdapter iface);
bool portenta_h7_isOTACapable();
#endif

#ifdef ARDUINO_ARCH_ESP32
int esp32_onOTARequest(char const * url);
String esp32_getOTAImageSHA256();
bool esp32_isOTACapable();
#endif

#ifdef ARDUINO_UNOR4_WIFI
int unor4_onOTARequest(char const * url);
String unor4_getOTAImageSHA256();
bool unor4_isOTACapable();
#endif

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

int OTA::onRequest(String url, NetworkAdapter iface)
{
  DEBUG_INFO("ArduinoIoTCloudTCP::%s _ota_url = %s", __FUNCTION__, url.c_str());

#if defined (ARDUINO_ARCH_SAMD)
  (void)iface;
  return samd_onOTARequest(url.c_str());
#elif defined (ARDUINO_NANO_RP2040_CONNECT)
  (void)iface;
  return rp2040_connect_onOTARequest(url.c_str());
#elif defined (BOARD_STM32H7)
  return portenta_h7_onOTARequest(url.c_str(), iface);
#elif defined (ARDUINO_ARCH_ESP32)
  (void)iface;
  return esp32_onOTARequest(url.c_str());
#elif defined (ARDUINO_UNOR4_WIFI)
  (void)iface;
  return unor4_onOTARequest(url.c_str());
#else
  #error "OTA not supported for this architecture"
#endif
}

String OTA::getImageSHA256()
{
#if defined (ARDUINO_ARCH_SAMD)
  return samd_getOTAImageSHA256();
#elif defined (ARDUINO_NANO_RP2040_CONNECT)
  return rp2040_connect_getOTAImageSHA256();
#elif defined (BOARD_STM32H7)
  return portenta_h7_getOTAImageSHA256();
#elif defined (ARDUINO_ARCH_ESP32)
  return esp32_getOTAImageSHA256();
#elif defined (ARDUINO_UNOR4_WIFI)
  return unor4_getOTAImageSHA256();
#else
  #error "No method for SHA256 checksum calculation over application image defined for this architecture."
#endif
}

bool OTA::isCapable()
{
#if defined (ARDUINO_ARCH_SAMD)
  return samd_isOTACapable();
#elif defined (ARDUINO_NANO_RP2040_CONNECT)
  return rp2040_connect_isOTACapable();
#elif defined (BOARD_STM32H7)
  return portenta_h7_isOTACapable();
#elif defined (ARDUINO_ARCH_ESP32)
  return esp32_isOTACapable();
#elif defined (ARDUINO_UNOR4_WIFI)
  return unor4_isOTACapable();
#else
  #error "OTA not supported for this architecture"
#endif
}

#endif /* OTA_ENABLED */
