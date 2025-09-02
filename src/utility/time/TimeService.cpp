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

#include <time.h>

#include "AIoTC_Config.h"
#include "AIoTC_Const.h"
#include "NTPUtils.h"
#include "TimeService.h"

#if defined(BOARD_HAS_HW_RTC)
  #if defined(ARDUINO_ARCH_SAMD)
    #include <RTCZero.h>
  #endif
  #if defined(ARDUINO_ARCH_MBED)
    #include <mbed_rtc_time.h>
  #endif
  #if defined(ARDUINO_ARCH_RENESAS)
    #include <RTC.h>
  #endif
#else
  #include "RTCMillis.h"
#endif

/******************************************************************************
  GLOBAL VARIABLES
 ******************************************************************************/

#if !defined(BOARD_HAS_HW_RTC)
RTCMillis rtc;
#endif

#if defined(BOARD_HAS_HW_RTC) && defined(ARDUINO_ARCH_SAMD)
RTCZero rtc;
#endif

/******************************************************************************
  INTERNAL FUNCTION DECLARATION
 ******************************************************************************/

time_t cvt_time(char const * time);

/******************************************************************************
  RTC PRIVATE FUNCTION DEFINITION
 ******************************************************************************/

#if defined(BOARD_HAS_HW_RTC)
  #if defined(ARDUINO_ARCH_SAMD)
static inline void _initRTC() {
  rtc.begin();
}
static inline void _setRTC(unsigned long time) {
  rtc.setEpoch(time);
}
static inline unsigned long _getRTC() {
  return rtc.getEpoch();
}
  #endif
  #if defined(ARDUINO_ARCH_MBED)
static inline void _initRTC() {
  /* Nothing to do */
}
static inline void _setRTC(unsigned long time) {
  set_time(time);
}
static inline unsigned long _getRTC() {
  return time(NULL);
}
  #endif
  #if defined(ARDUINO_ARCH_RENESAS)
static inline void _initRTC() {
  RTC.begin();
}
static inline void _setRTC(unsigned long time) {
  RTCTime t(time);
  RTC.setTime(t);
}
static inline unsigned long _getRTC() {
  RTCTime t;
  RTC.getTime(t);
  return t.getUnixTime();
}
  #endif
  #if defined(ARDUINO_ARCH_ESP32)
static inline void _initRTC() {
  //configTime(0, 0, "time.arduino.cc", "pool.ntp.org", "time.nist.gov");
}
static inline void _setRTC(unsigned long time) {
  const timeval epoch = {(time_t)time, 0};
  settimeofday(&epoch, 0);
}
static inline unsigned long _getRTC() {
  return time(NULL);
}
  #endif
#else /* !BOARD_HAS_HW_RTC */
  #pragma message "No hardware RTC implementation found, using soft RTC"
static inline void _initRTC() {
  rtc.begin();
}
static inline void _setRTC(unsigned long time) {
  rtc.set(time);
}
static inline unsigned long _getRTC() {
  return rtc.get();
}
#endif

/******************************************************************************
  DEFINES
 ******************************************************************************/

#define EPOCH_AT_COMPILE_TIME cvt_time(__DATE__)

/******************************************************************************
  CONSTANTS
 ******************************************************************************/

/* Default NTP synch is scheduled each 24 hours from startup */
static time_t const TIMESERVICE_NTP_SYNC_TIMEOUT_ms = DAYS * 1000;
static time_t const EPOCH = 0;

/******************************************************************************
  CTOR/DTOR
 ******************************************************************************/

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

/******************************************************************************
  PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

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
    /* Try to sync time from NTP or connection handler */
    sync();
  }

  /* Use RTC time if has been configured at least once */
  if(_last_sync_tick) {
    return getRTC();
  }

  /* Return the epoch timestamp at compile time as a last line of defense
   * trying to connect to the server. Otherwise the certificate expiration
   * date is wrong and we'll be unable to establish a connection. Schedulers
   * won't work correctly using this value.
   */
  return EPOCH_AT_COMPILE_TIME;
}

void TimeServiceClass::setTime(unsigned long time)
{
  setRTC(time);
}

bool TimeServiceClass::sync()
{
  _is_rtc_configured = false;

  unsigned long utc = EPOCH;
  if(_sync_func) {
    utc = _sync_func();
  } else {
#if defined(HAS_NOTECARD) || defined(HAS_TCP)
    utc = getRemoteTime();
#elif defined(HAS_LORA)
    /* Just keep incrementing stored RTC value starting from EPOCH_AT_COMPILE_TIME */
    utc = getRTC();
#endif
  }

  if(isTimeValid(utc)) {
    DEBUG_DEBUG("TimeServiceClass::%s done. Drift: %d RTC value: %u", __FUNCTION__, getRTC() - utc, utc);
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
/******************************************************************************
  PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

#if defined(HAS_NOTECARD) || defined(HAS_TCP)
bool TimeServiceClass::connected()
{
  if(_con_hdl == nullptr) {
    return false;
  } else {
    return _con_hdl->check() == NetworkConnectionState::CONNECTED;
  }
}

unsigned long TimeServiceClass::getRemoteTime()
{
  if(connected()) {
#ifdef HAS_TCP
    /* At first try to obtain a valid time via NTP.
     * This is the most reliable time source and it will
     * ensure a correct behaviour of the library.
     */
    if(_con_hdl->getInterface() != NetworkAdapter::CELL) {
      unsigned long const ntp_time = NTPUtils::getTime(_con_hdl->getUDP());
      if(isTimeValid(ntp_time)) {
        return ntp_time;
      }
    }
    DEBUG_WARNING("TimeServiceClass::%s cannot get time from NTP, fallback on connection handler", __FUNCTION__);
#endif  /* HAS_TCP */

    /* As fallback if NTP request fails try to obtain the
     * network time using the connection handler.
     */
    unsigned long const connection_time = _con_hdl->getTime();
    if(isTimeValid(connection_time)) {
      return connection_time;
    }
    DEBUG_WARNING("TimeServiceClass::%s cannot get time from connection handler", __FUNCTION__);
  }

  /* Return known invalid value because we are not connected */
  return EPOCH;
}

#endif  /* HAS_NOTECARD || HAS_TCP */

bool TimeServiceClass::isTimeValid(unsigned long const time)
{
  /* EPOCH_AT_COMPILE_TIME is in local time, so we need to subtract the maximum
   * possible timezone offset UTC+14 to make sure we are less then UTC time
   */
  return (time > (EPOCH_AT_COMPILE_TIME - (14 * 60 * 60)));
}

bool TimeServiceClass::isTimeZoneOffsetValid(long const offset)
{
  /* UTC offset can go from +14 to -12 hours */
  return ((offset < (14 * 60 * 60)) && (offset > (-12 * 60 * 60)));
}

void TimeServiceClass::initRTC()
{
  _initRTC();
}

void TimeServiceClass::setRTC(unsigned long time)
{
  _setRTC(time);
}

unsigned long TimeServiceClass::getRTC()
{
  return _getRTC();
}

/******************************************************************************
  INTERNAL FUNCTION DEFINITION
 ******************************************************************************/

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

/******************************************************************************
  EXTERN DEFINITION
 ******************************************************************************/

TimeServiceClass TimeService;
