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

/******************************************************************************
 * FUNCTION DEFINITION
 ******************************************************************************/

#ifdef ARDUINO_ARCH_SAMD
/* This function is called within the WiFiNINA library when invoking
 * the method 'connectBearSSL' in order to prevent a premature bite
 * of the watchdog (max timeout on SAMD is 16 s). wifi_nina_feed...
 * is defined a weak function there and overwritten by this "strong"
 * function here.
 */
void wifi_nina_feed_watchdog()
{
    Watchdog.reset();
}
#endif /* ARDUINO_ARCH_SAMD */
