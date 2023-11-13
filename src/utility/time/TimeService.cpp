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

#include <AIoTC_Config.h>

#include <time.h>
#include "TimeService.h"
#include "NTPUtils.h"
#include "AIoTC_Const.h"

#ifdef ARDUINO_ARCH_SAMD
  #include <RTCZero.h>
#endif

#ifdef ARDUINO_ARCH_MBED
  #include <mbed_rtc_time.h>
#endif

#ifdef ARDUINO_ARCH_ESP8266
  #include "RTCMillis.h"
#endif

#ifdef ARDUINO_ARCH_RENESAS
  #include "RTC.h"
#endif

/**************************************************************************************
 * GLOBAL VARIABLES
 **************************************************************************************/

#ifdef ARDUINO_ARCH_SAMD
RTCZero rtc;
#endif

#ifdef ARDUINO_ARCH_ESP8266
RTCMillis rtc;
#endif

/**************************************************************************************
 * INTERNAL FUNCTION DECLARATION
 **************************************************************************************/

time_t cvt_time(char const * time);

#ifdef ARDUINO_ARCH_SAMD
void samd_initRTC();
void samd_setRTC(unsigned long time);
unsigned long samd_getRTC();
#endif

#ifdef ARDUINO_ARCH_MBED
void mbed_initRTC();
void mbed_setRTC(unsigned long time);
unsigned long mbed_getRTC();
#endif

#ifdef ARDUINO_ARCH_ESP32
void esp32_initRTC();
void esp32_setRTC(unsigned long time);
unsigned long esp32_getRTC();
#endif

#ifdef ARDUINO_ARCH_ESP8266
void esp8266_initRTC();
void esp8266_setRTC(unsigned long time);
unsigned long esp8266_getRTC();
#endif

#ifdef ARDUINO_ARCH_RENESAS
void renesas_initRTC();
void renesas_setRTC(unsigned long time);
unsigned long renesas_getRTC();
#endif

/**************************************************************************************
 * DEFINES
 **************************************************************************************/

#define EPOCH_AT_COMPILE_TIME cvt_time(__DATE__)

/**************************************************************************************
 * CONSTANTS
 **************************************************************************************/

/* Default NTP synch is scheduled each 24 hours from startup */
static time_t const TIMESERVICE_NTP_SYNC_TIMEOUT_ms = DAYS * 1000;
static time_t const EPOCH = 0;

/**************************************************************************************
 * CTOR/DTOR
 **************************************************************************************/

TimeServiceClass::TimeServiceClass()
: _con_hdl(nullptr)
, _is_rtc_configured(false)
, _is_tz_configured(false)
, _timezone_offset(24 * 60 * 60)
, _timezone_dst_until(0)
, _last_sync_tick(0)
, _sync_interval_ms(TIMESERVICE_NTP_SYNC_TIMEOUT_ms)
, _sync_func(nullptr)
{

}

/**************************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 **************************************************************************************/

void TimeServiceClass::begin(ConnectionHandler * con_hdl)
{
  _con_hdl = con_hdl;
  initRTC();
#ifdef HAS_LORA
  setRTC(EPOCH_AT_COMPILE_TIME);
#endif
}

unsigned long TimeServiceClass::getTime()
{
  /* Check if it's time to sync */
  unsigned long const current_tick = millis();
  bool const is_ntp_sync_timeout = (current_tick - _last_sync_tick) > _sync_interval_ms;
  if(!_is_rtc_configured || is_ntp_sync_timeout) {
    sync();
  }

  /* Read time from RTC */
  unsigned long utc = getRTC();
  return isTimeValid(utc) ? utc : EPOCH_AT_COMPILE_TIME;
}

void TimeServiceClass::setTime(unsigned long time)
{
  setRTC(time);
}

bool TimeServiceClass::sync()
{
  _is_rtc_configured = false;

  unsigned long utc = EPOCH_AT_COMPILE_TIME;
  if(_sync_func) {
    utc = _sync_func();
  } else {
#ifdef HAS_TCP
    utc = getRemoteTime();
#endif
#ifdef HAS_LORA
    /* Just keep incrementing stored RTC value*/
    utc = getRTC();
#endif
  }

  if(isTimeValid(utc)) {
    DEBUG_DEBUG("TimeServiceClass::%s  Drift: %d RTC value: %u", __FUNCTION__, getRTC() - utc, utc);
    setRTC(utc);
    _last_sync_tick = millis();
    _is_rtc_configured = true;
  }
  return _is_rtc_configured;
}

void TimeServiceClass::setSyncInterval(unsigned long seconds)
{
  _sync_interval_ms = seconds * 1000;
}

void TimeServiceClass::setSyncFunction(syncTimeFunctionPtr sync_func)
{
  if(sync_func) {
    _sync_func = sync_func;
  }
}

void TimeServiceClass::setTimeZoneData(long offset, unsigned long dst_until)
{
  if(isTimeZoneOffsetValid(offset) && isTimeValid(dst_until)) {
    if(_timezone_offset != offset || _timezone_dst_until != dst_until) {
      DEBUG_DEBUG("TimeServiceClass::%s offset: %d dst_unitl %u", __FUNCTION__, offset, dst_until);
      _timezone_offset = offset;
      _timezone_dst_until = dst_until;
      _is_tz_configured = true;
    }
  }
}

unsigned long TimeServiceClass::getLocalTime()
{
  unsigned long utc = getTime();
  if(_is_tz_configured) {
    return utc + _timezone_offset;
  } else {
    return EPOCH;
  }
}

unsigned long TimeServiceClass::getTimeFromString(const String& input)
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

  if(input == nullptr || input.length() != expected_length) {
    DEBUG_ERROR("TimeServiceClass::%s invalid input length", __FUNCTION__);
    return 0;
  }

  int scanned_parameters = sscanf(input.c_str(), "%d %s %d %d:%d:%d", &year, s_month, &day, &hour, &min, &sec);

  if(scanned_parameters != expected_parameters) {
    DEBUG_ERROR("TimeServiceClass::%s invalid input parameters number", __FUNCTION__);
    return 0;
  }

  char * s_month_position = strstr(month_names, s_month);

  if(s_month_position == nullptr || strlen(s_month) != 3) {
    DEBUG_ERROR("TimeServiceClass::%s invalid month name, use %s", __FUNCTION__, month_names);
    return 0;
  }

  month = (s_month_position - month_names) / 3;

  if(month <  0 || month > 11 || day <  1 || day > 31 || year < 1900 || hour < 0 ||
     hour  > 24 || min   <  0 || min > 60 || sec <  0 || sec  >  60) {
    DEBUG_ERROR("TimeServiceClass::%s invalid date values", __FUNCTION__);
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

#ifdef HAS_TCP
bool TimeServiceClass::connected()
{
  if(_con_hdl == nullptr) {
    return false;
  } else {
    return _con_hdl->getStatus() == NetworkConnectionState::CONNECTED;
  }
}

unsigned long TimeServiceClass::getRemoteTime()
{
  if(connected()) {
    /* At first try to obtain a valid time via NTP.
     * This is the most reliable time source and it will
     * ensure a correct behaviour of the library.
     */
    unsigned long const ntp_time = NTPUtils::getTime(_con_hdl->getUDP());
    if(isTimeValid(ntp_time)) {
      return ntp_time;
    }

    /* As fallback if NTP request fails try to obtain the
     * network time using the connection handler.
     */
    unsigned long const connection_time = _con_hdl->getTime();
    if(isTimeValid(connection_time)) {
      return connection_time;
    }
  }

  /* Return the epoch timestamp at compile time as a last
   * line of defense. Otherwise the certificate expiration
   * date is wrong and we'll be unable to establish a connection
   * to the server.
   */
  return EPOCH_AT_COMPILE_TIME;
}

#endif  /* HAS_TCP */

bool TimeServiceClass::isTimeValid(unsigned long const time)
{
  return (time > EPOCH_AT_COMPILE_TIME);
}

bool TimeServiceClass::isTimeZoneOffsetValid(long const offset)
{
  /* UTC offset can go from +14 to -12 hours */
  return ((offset < (14 * 60 * 60)) && (offset > (-12 * 60 * 60)));
}

void TimeServiceClass::initRTC()
{
#if defined (ARDUINO_ARCH_SAMD)
  samd_initRTC();
#elif defined (ARDUINO_ARCH_MBED)
  mbed_initRTC();
#elif defined (ARDUINO_ARCH_ESP32)
  esp32_initRTC();
#elif defined (ARDUINO_ARCH_ESP8266)
  esp8266_initRTC();
#elif defined (ARDUINO_ARCH_RENESAS)
  renesas_initRTC();
#else
  #error "RTC not available for this architecture"
#endif
}

void TimeServiceClass::setRTC(unsigned long time)
{
#if defined (ARDUINO_ARCH_SAMD)
  samd_setRTC(time);
#elif defined (ARDUINO_ARCH_MBED)
  mbed_setRTC(time);
#elif defined (ARDUINO_ARCH_ESP32)
  esp32_setRTC(time);
#elif defined (ARDUINO_ARCH_ESP8266)
  esp8266_setRTC(time);
#elif defined (ARDUINO_ARCH_RENESAS)
  renesas_setRTC(time);
#else
  #error "RTC not available for this architecture"
#endif
}

unsigned long TimeServiceClass::getRTC()
{
#if defined (ARDUINO_ARCH_SAMD)
  return samd_getRTC();
#elif defined (ARDUINO_ARCH_MBED)
  return mbed_getRTC();
#elif defined (ARDUINO_ARCH_ESP32)
  return esp32_getRTC();
#elif defined (ARDUINO_ARCH_ESP8266)
  return esp8266_getRTC();
#elif defined (ARDUINO_ARCH_RENESAS)
  return renesas_getRTC();
#else
  #error "RTC not available for this architecture"
#endif
}

/**************************************************************************************
 * INTERNAL FUNCTION DEFINITION
 **************************************************************************************/

time_t cvt_time(char const * time)
{
  static time_t build_time = 0;

  if (!build_time) {
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

    build_time = mktime(&t);
  }

  return build_time;
}

#ifdef ARDUINO_ARCH_SAMD
void samd_initRTC()
{
  rtc.begin();
}

void samd_setRTC(unsigned long time)
{
  rtc.setEpoch(time);
}

unsigned long samd_getRTC()
{
  return rtc.getEpoch();
}
#endif

#ifdef ARDUINO_ARCH_MBED
void mbed_initRTC()
{
  /* Nothing to do */
}

void mbed_setRTC(unsigned long time)
{
  set_time(time);
}

unsigned long mbed_getRTC()
{
  return time(NULL);
}
#endif

#ifdef ARDUINO_ARCH_ESP32
void esp32_initRTC()
{
  //configTime(0, 0, "time.arduino.cc", "pool.ntp.org", "time.nist.gov");
}

void esp32_setRTC(unsigned long time)
{
  const timeval epoch = {(time_t)time, 0};
  settimeofday(&epoch, 0);
}

unsigned long esp32_getRTC()
{
  return time(NULL);
}
#endif

#ifdef ARDUINO_ARCH_ESP8266
void esp8266_initRTC()
{
  rtc.begin();
}

void esp8266_setRTC(unsigned long time)
{
  rtc.set(time);
}

unsigned long esp8266_getRTC()
{
  return rtc.get();
}
#endif

#ifdef ARDUINO_ARCH_RENESAS
void renesas_initRTC()
{
  RTC.begin();
}

void renesas_setRTC(unsigned long time)
{
  RTCTime t(time);
  RTC.setTime(t);
}

unsigned long renesas_getRTC()
{
  RTCTime t;
  RTC.getTime(t);
  return t.getUnixTime();
}
#endif

/******************************************************************************
 * EXTERN DEFINITION
 ******************************************************************************/

TimeServiceClass TimeService;
