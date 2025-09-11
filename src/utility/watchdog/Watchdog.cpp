/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2020 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/******************************************************************************
  INCLUDE
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
#  include <drivers/Watchdog.h>
#  define PORTENTA_H7_WATCHDOG_MAX_TIMEOUT_ms  (32760)
#  define NANO_RP2040_WATCHDOG_MAX_TIMEOUT_ms  (8389)
#  define EDGE_CONTROL_WATCHDOG_MAX_TIMEOUT_ms (65536)
#endif /* ARDUINO_ARCH_MBED */

/******************************************************************************
  GLOBAL VARIABLES
 ******************************************************************************/
#if defined(ARDUINO_ARCH_SAMD)
static bool is_watchdog_enabled = false;
#endif

/******************************************************************************
  FUNCTION DEFINITION
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

/* This function is called within the GSMConnectionHandler. mkr_gsm_feed...
 * is defined a weak function there and overwritten by this "strong"
 * function here.
 */
void mkr_gsm_feed_watchdog()
{
  samd_watchdog_reset();
}
/* This function is called within the GSMConnectionHandler. mkr_nb_feed...
 * is defined a weak function there and overwritten by this "strong"
 * function here.
 */
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
#elif defined(ARDUINO_EDGE_CONTROL)
  cfg.timeout_ms = EDGE_CONTROL_WATCHDOG_MAX_TIMEOUT_ms;
#else
# error "You need to define the maximum possible timeout for this architecture."
#endif

  if (!mbed::Watchdog::get_instance().start(cfg.timeout_ms)) {
    DEBUG_WARNING("%s: watchdog could not be enabled", __FUNCTION__);
  }
}

static void mbed_watchdog_reset()
{
  if (mbed::Watchdog::get_instance().is_running()) {
    mbed::Watchdog::get_instance().kick();
  }
}

static void mbed_watchdog_enable_network_feed(NetworkAdapter ni)
{
  if (ni == NetworkAdapter::ETHERNET) {
#if defined(BOARD_HAS_ETHERNET)
    Ethernet.setFeedWatchdogFunc(watchdog_reset);
#endif
  }

  if (ni == NetworkAdapter::WIFI) {
#if defined(ARDUINO_PORTENTA_H7_WIFI_HAS_FEED_WATCHDOG_FUNC) && defined(BOARD_HAS_WIFI)
    WiFi.setFeedWatchdogFunc(watchdog_reset);
#endif
  }

  if (ni == NetworkAdapter::CATM1) {
#if defined(BOARD_HAS_CATM1_NBIOT)
    GSM.setFeedWatchdogFunc(watchdog_reset);
#endif
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

void watchdog_enable_network_feed(NetworkAdapter ni)
{
  /* Setup WiFi NINA watchdog feed callback function */
#if defined(ARDUINO_ARCH_SAMD) && defined(WIFI_HAS_FEED_WATCHDOG_FUNC)
  (void)ni;
  WiFi.setFeedWatchdogFunc(watchdog_reset);
#endif

  /* Setup mbed sockets watchdog feed callback function */
#if defined(ARDUINO_ARCH_MBED)
  mbed_watchdog_enable_network_feed(ni);
#endif
}
#endif /* (ARDUINO_ARCH_SAMD) || (ARDUINO_ARCH_MBED) */
