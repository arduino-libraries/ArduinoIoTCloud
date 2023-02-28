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

#ifndef ARDUINO_IOT_CLOUD_RTC_MILLIS_H_
#define ARDUINO_IOT_CLOUD_RTC_MILLIS_H_

#ifdef ARDUINO_ARCH_ESP8266

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

/**************************************************************************************
 * CLASS DECLARATION
 **************************************************************************************/

class RTCMillis
{

public:

  RTCMillis();

  void          begin();
  void          set(unsigned long time);
  unsigned long get();

private:
  unsigned long _last_rtc_update_tick;
  unsigned long _last_rtc_update_value;

};

#endif /* ARDUINO_ARCH_ESP8266 */

#endif /* ARDUINO_IOT_CLOUD_RTC_MILLIS_H_ */
