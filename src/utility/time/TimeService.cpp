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

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include "TimeService.h"

#include <time.h>

#include "NTPUtils.h"

/**************************************************************************************
 * GLOBAL VARIABLES
 **************************************************************************************/

#ifdef ARDUINO_ARCH_SAMD
RTCZero rtc;
#endif

/**************************************************************************************
 * INTERNAL FUNCTION DECLARATION
 **************************************************************************************/

time_t cvt_time(char const * time);

/**************************************************************************************
 * CONSTANTS
 **************************************************************************************/

static time_t const EPOCH_AT_COMPILE_TIME = cvt_time(__DATE__);
static time_t const EPOCH = 0;

/**************************************************************************************
 * CTOR/DTOR
 **************************************************************************************/

TimeService::TimeService()
: _con_hdl(nullptr)
#if defined (ARDUINO_ARCH_SAMD) || defined (ARDUINO_ARCH_MBED)
, _is_rtc_configured(false)
#endif
, _is_tz_configured(false)
, _timezone_offset(0)
, _timezone_dst_until(0)
{

}

/**************************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 **************************************************************************************/

void TimeService::begin(ConnectionHandler * con_hdl)
{
  _con_hdl = con_hdl;
#ifdef ARDUINO_ARCH_SAMD
  rtc.begin();
#endif
}

unsigned long TimeService::getTime()
{
#ifdef ARDUINO_ARCH_SAMD
  if(!_is_rtc_configured)
  {
    unsigned long utc = getRemoteTime();
    if(EPOCH_AT_COMPILE_TIME != utc)
    {
      rtc.setEpoch(utc);
      _is_rtc_configured = true;
    }
    return utc;
  }
  return rtc.getEpoch();
#elif ARDUINO_ARCH_MBED
  if(!_is_rtc_configured)
  {
    unsigned long utc = getRemoteTime();
    if(EPOCH_AT_COMPILE_TIME != utc)
    {
      set_time(utc);
      _is_rtc_configured = true;
    }
    return utc;
  }
  return time(NULL);
#else
  return getRemoteTime();
#endif
}

void TimeService::setTimeZoneData(long offset, unsigned long dst_until)
{
  if(_timezone_offset != offset)
    DEBUG_DEBUG("ArduinoIoTCloudTCP::%s tz_offset: [%d]", __FUNCTION__, offset);
  _timezone_offset = offset;

  if(_timezone_dst_until != dst_until)
    DEBUG_DEBUG("ArduinoIoTCloudTCP::%s tz_dst_unitl: [%ul]", __FUNCTION__, dst_until);
  _timezone_dst_until = dst_until;

  _is_tz_configured = true;
}

unsigned long TimeService::getLocalTime()
{
  unsigned long utc = getTime();
  if(_is_tz_configured) {
    return utc + _timezone_offset;
  } else {
    return EPOCH;
  }
}

unsigned long TimeService::getTimeFromString(const String& input)
{
  struct tm t =
  {
    0 /* tm_sec   */,
    0 /* tm_min   */,
    0 /* tm_hour  */,
    0 /* tm_mday  */,
    0 /* tm_mon   */,
    0 /* tm_year  */,
    0 /* tm_wday  */,
    0 /* tm_yday  */,
    0 /* tm_isdst */
  };

  char s_month[16];
  int month, day, year, hour, min, sec;
  static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
  static const int expected_length = 20;
  static const int expected_parameters = 6;

  if(input == nullptr || input.length() != expected_length)
  {
    DEBUG_ERROR("ArduinoIoTCloudTCP::%s invalid input length", __FUNCTION__);
    return 0;
  }

  int scanned_parameters = sscanf(input.c_str(), "%d %s %d %d:%d:%d", &year, s_month, &day, &hour, &min, &sec);

  if(scanned_parameters != expected_parameters)
  {
    DEBUG_ERROR("ArduinoIoTCloudTCP::%s invalid input parameters number", __FUNCTION__);
    return 0;
  }

  char * s_month_position = strstr(month_names, s_month);

  if(s_month_position == nullptr || strlen(s_month) != 3) {
    DEBUG_ERROR("ArduinoIoTCloudTCP::%s invalid month name, use %s", __FUNCTION__, month_names);
    return 0;
  }

  month = (s_month_position - month_names) / 3;

  if(month <  0 || month > 11 || day <  1 || day > 31 || year < 1900 || hour < 0 ||
     hour  > 24 || min   <  0 || min > 60 || sec <  0 || sec  >  60) {
    DEBUG_ERROR("ArduinoIoTCloudTCP::%s invalid date values", __FUNCTION__);
    return 0;
  }

  t.tm_mon = month;
  t.tm_mday = day;
  t.tm_year = year - 1900;
  t.tm_hour = hour;
  t.tm_min = min;
  t.tm_sec = sec;
  t.tm_isdst = -1;

  return mktime(&t);
}
/**************************************************************************************
 * PRIVATE MEMBER FUNCTIONS
 **************************************************************************************/

bool TimeService::connected()
{
  if(_con_hdl == nullptr) {
    return false;
  } else {
    return _con_hdl->getStatus() == NetworkConnectionState::CONNECTED;
  }
}

unsigned long TimeService::getRemoteTime()
{
#include "../../AIoTC_Config.h"
#ifndef HAS_LORA

  if(connected()) {
    /* At first try to see if a valid time can be obtained
     * using the network time available via the connection
     * handler.
     */
    unsigned long const connection_time = _con_hdl->getTime();
    if(isTimeValid(connection_time)) {
      return connection_time;
    }

#ifndef __AVR__
    /* If no valid network time is available try to obtain the
     * time via NTP next.
     */
    unsigned long const ntp_time = NTPUtils::getTime(_con_hdl->getUDP());
    if(isTimeValid(ntp_time)) {
      return ntp_time;
    }
#endif
  }

#endif /* ifndef HAS_LORA */

  /* Return the epoch timestamp at compile time as a last
   * line of defense. Otherwise the certificate expiration
   * date is wrong and we'll be unable to establish a connection
   * to the server.
   */
  return EPOCH_AT_COMPILE_TIME;
}

bool TimeService::isTimeValid(unsigned long const time)
{
  return (time >= EPOCH_AT_COMPILE_TIME);
}

/**************************************************************************************
 * INTERNAL FUNCTION DEFINITION
 **************************************************************************************/

time_t cvt_time(char const * time)
{
  char s_month[5];
  int month, day, year;
  struct tm t =
  {
    0 /* tm_sec   */,
    0 /* tm_min   */,
    0 /* tm_hour  */,
    0 /* tm_mday  */,
    0 /* tm_mon   */,
    0 /* tm_year  */,
    0 /* tm_wday  */,
    0 /* tm_yday  */,
    0 /* tm_isdst */
  };
  static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";

  sscanf(time, "%s %d %d", s_month, &day, &year);

  month = (strstr(month_names, s_month) - month_names) / 3;

  t.tm_mon = month;
  t.tm_mday = day;
  t.tm_year = year - 1900;
  t.tm_isdst = -1;

  return mktime(&t);
}

TimeService & ArduinoIoTCloudTimeService() {
  static TimeService _timeService_instance;
  return _timeService_instance;
}
