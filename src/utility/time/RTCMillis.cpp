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

#ifdef ARDUINO_ARCH_ESP8266

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <Arduino.h>
#include "RTCMillis.h"

/**************************************************************************************
 * CTOR/DTOR
 **************************************************************************************/

RTCMillis::RTCMillis()
: _last_rtc_update_tick(0)
, _last_rtc_update_value(0)
{

}

/**************************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 **************************************************************************************/

void RTCMillis::begin()
{

}

void RTCMillis::set(unsigned long time)
{
  _last_rtc_update_tick = millis();
  _last_rtc_update_value = time;
}

unsigned long RTCMillis::get()
{
  unsigned long current_tick = millis();
  unsigned long const elapsed_s = (current_tick - _last_rtc_update_tick) / 1000;
  if(elapsed_s) {
    set(_last_rtc_update_value + elapsed_s);
  }
  return _last_rtc_update_value;
}

#endif /* ARDUINO_ARCH_ESP8266 */
