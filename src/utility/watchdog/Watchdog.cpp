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

#include "Watchdog.h"

#include <AIoTC_Config.h>

#if defined(DEBUG_ERROR) || defined(DEBUG_WARNING) || defined(DEBUG_INFO) || defined(DEBUG_DEBUG) || defined(DEBUG_VERBOSE)
#  include <Arduino_DebugUtils.h>
#endif

#ifdef ARDUINO_ARCH_SAMD
#  include <Adafruit_SleepyDog.h>
#  define SAMD_WATCHDOG_MAX_TIME_ms (16 * 1000)
#endif /* ARDUINO_ARCH_SAMD */

#ifdef ARDUINO_ARCH_MBED
#  include <watchdog_api.h>
#  define PORTENTA_H7_WATCHDOG_MAX_TIMEOUT_ms (32760)
#  define NANO_RP2040_WATCHDOG_MAX_TIMEOUT_ms (8389)
#endif /* ARDUINO_ARCH_MBED */

#include <Arduino_ConnectionHandler.h>

/******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/
#if defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_MBED)
static bool is_watchdog_enabled = false;
#endif

/******************************************************************************
 * FUNCTION DEFINITION
 ******************************************************************************/

#ifdef ARDUINO_ARCH_SAMD
static void samd_watchdog_enable()
{
  is_watchdog_enabled = true;
  Watchdog.enable(SAMD_WATCHDOG_MAX_TIME_ms);
}

static void samd_watchdog_reset()
{
  if (is_watchdog_enabled) {
    Watchdog.reset();
  }
}

/* This function is called within the WiFiNINA library when invoking
 * the method 'connectBearSSL' in order to prevent a premature bite
 * of the watchdog (max timeout on SAMD is 16 s). wifi_nina_feed...
 * is defined a weak function there and overwritten by this "strong"
 * function here.
 */
#ifndef WIFI_HAS_FEED_WATCHDOG_FUNC
void wifi_nina_feed_watchdog()
{
  samd_watchdog_reset();
}
#endif

void mkr_gsm_feed_watchdog()
{
  samd_watchdog_reset();
}

void mkr_nb_feed_watchdog()
{
  samd_watchdog_reset();
}
#endif /* ARDUINO_ARCH_SAMD */

#ifdef ARDUINO_ARCH_MBED
static void mbed_watchdog_enable()
{
  watchdog_config_t cfg;
#if defined(BOARD_STM32H7)
  cfg.timeout_ms = PORTENTA_H7_WATCHDOG_MAX_TIMEOUT_ms;
#elif defined(ARDUINO_NANO_RP2040_CONNECT)
  cfg.timeout_ms = NANO_RP2040_WATCHDOG_MAX_TIMEOUT_ms;
#else
# error "You need to define the maximum possible timeout for this architecture."
#endif

  if (hal_watchdog_init(&cfg) == WATCHDOG_STATUS_OK) {
    is_watchdog_enabled = true;
  }
  else {
    DEBUG_WARNING("%s: watchdog could not be enabled", __FUNCTION__);
  }
}

static void mbed_watchdog_reset()
{
  if (is_watchdog_enabled) {
    hal_watchdog_kick();
  }
}

#if defined (ARDUINO_PORTENTA_H7_WIFI_HAS_FEED_WATCHDOG_FUNC)
static void mbed_watchdog_enable_network_feed(const bool use_ethernet)
{
#if defined(BOARD_HAS_ETHERNET)
  if(use_ethernet) {
    Ethernet.setFeedWatchdogFunc(watchdog_reset);
  } else
#endif
    WiFi.setFeedWatchdogFunc(watchdog_reset);

}
#endif

void mbed_watchdog_trigger_reset()
{
  watchdog_config_t cfg;
#if defined(BOARD_STM32H7)
  cfg.timeout_ms = 1;
#elif defined(ARDUINO_NANO_RP2040_CONNECT)
  cfg.timeout_ms = 1;
#else
# error "You need to define the maximum possible timeout for this architecture."
#endif

  if (hal_watchdog_init(&cfg) == WATCHDOG_STATUS_OK) {
    is_watchdog_enabled = true;
    while(1){}
  }
  else {
    DEBUG_WARNING("%s: watchdog could not be reconfigured", __FUNCTION__);
  }

}
#endif /* ARDUINO_ARCH_MBED */

#if defined (ARDUINO_ARCH_SAMD) || defined (ARDUINO_ARCH_MBED)
void watchdog_enable()
{
#ifdef ARDUINO_ARCH_SAMD
  samd_watchdog_enable();
#else
  mbed_watchdog_enable();
#endif
}

void watchdog_reset()
{
#ifdef ARDUINO_ARCH_SAMD
  samd_watchdog_reset();
#else
  mbed_watchdog_reset();
#endif
}

void watchdog_enable_network_feed(const bool use_ethernet)
{
#ifdef WIFI_HAS_FEED_WATCHDOG_FUNC
  WiFi.setFeedWatchdogFunc(watchdog_reset);
#endif

#ifdef ARDUINO_PORTENTA_H7_WIFI_HAS_FEED_WATCHDOG_FUNC
  mbed_watchdog_enable_network_feed(use_ethernet);
#endif
}
#endif /* (ARDUINO_ARCH_SAMD) || (ARDUINO_ARCH_MBED) */
