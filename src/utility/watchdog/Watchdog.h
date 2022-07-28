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

#ifndef ARDUINO_AIOTC_UTILITY_WATCHDOG_H_
#define ARDUINO_AIOTC_UTILITY_WATCHDOG_H_

/******************************************************************************
 * FUNCTION DECLARATION
 ******************************************************************************/

#if defined (ARDUINO_ARCH_SAMD) || defined (ARDUINO_ARCH_MBED)
void watchdog_enable();
void watchdog_reset();
void watchdog_enable_network_feed(const bool use_ethernet);
#endif /* (ARDUINO_ARCH_SAMD) || (ARDUINO_ARCH_MBED) */

#ifdef ARDUINO_ARCH_MBED
void mbed_watchdog_trigger_reset();
#endif /* ARDUINO_ARCH_MBED */

#endif /* ARDUINO_AIOTC_UTILITY_WATCHDOG_H_ */
