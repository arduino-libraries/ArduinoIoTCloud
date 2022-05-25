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

#ifndef ARDUINO_IOT_CLOUD_TIME_SERVICE_H_
#define ARDUINO_IOT_CLOUD_TIME_SERVICE_H_

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <Arduino_ConnectionHandler.h>

#include "../../AIoTC_Config.h"

#ifdef HAS_RTC
#ifdef ARDUINO_ARCH_SAMD
  #include <RTCZero.h>
#endif

#ifdef ARDUINO_ARCH_MBED
  #include <mbed_rtc_time.h>
#endif
#endif /* HAS_RTC */

/**************************************************************************************
 * CLASS DECLARATION
 **************************************************************************************/

class TimeService
{

public:

  TimeService();


  void          begin  (ConnectionHandler * con_hdl);
  unsigned long getTime();
  unsigned long getLocalTime();
  void          setTimeZoneData(long offset, unsigned long valid_until);
  /* Helper function to convert an input String into a UNIX timestamp.
   * The input String format must be as follow "2021 Nov 01 17:00:00"
   */
  static unsigned long getTimeFromString(const String& input);

private:

  ConnectionHandler * _con_hdl;
#ifdef HAS_RTC
  bool _is_rtc_configured;
#endif

#ifdef HAS_TCP
  bool _is_tz_configured;
  long _timezone_offset;
  unsigned long _timezone_dst_until;

  unsigned long getRemoteTime();
  bool connected();
#endif

#ifdef HAS_RTC
  void configureRTC();
  unsigned long getRTC();
#endif

  static bool isTimeValid(unsigned long const time);

};

TimeService & ArduinoIoTCloudTimeService();

#endif /* ARDUINO_IOT_CLOUD_TIME_SERVICE_H_ */
