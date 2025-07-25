/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2020 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef ARDUINO_IOT_CLOUD_TIME_SERVICE_H_
#define ARDUINO_IOT_CLOUD_TIME_SERVICE_H_

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <AIoTC_Config.h>
#include <Arduino_ConnectionHandler.h>

/******************************************************************************
 * TYPEDEF
 ******************************************************************************/

typedef unsigned long(*syncTimeFunctionPtr)(void);

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class TimeServiceClass
{

public:

  TimeServiceClass();

  void          begin  (ConnectionHandler * con_hdl);
  unsigned long getTime();
  void          setTime(unsigned long time);
  unsigned long getLocalTime();
  void          setTimeZoneData(long offset, unsigned long valid_until);
  bool          sync();
  void          setSyncInterval(unsigned long seconds);
  void          setSyncFunction(syncTimeFunctionPtr sync_func);

  /* Helper function to convert an input String into a UNIX timestamp.
   * The input String format must be as follow "2021 Nov 01 17:00:00"
   */
  static unsigned long getTimeFromString(const String& input);

  static bool isTimeValid(unsigned long const time);

private:

  ConnectionHandler * _con_hdl;
  bool _is_rtc_configured;
  bool _is_tz_configured;
  long _timezone_offset;
  unsigned long _timezone_dst_until;
  unsigned long _last_sync_tick;
  unsigned long _sync_interval_ms;
  syncTimeFunctionPtr _sync_func;

#if defined(HAS_NOTECARD) || defined(HAS_TCP)
  unsigned long getRemoteTime();
  bool connected();
#endif
  void initRTC();
  void setRTC(unsigned long time);
  unsigned long getRTC();
  static bool isTimeZoneOffsetValid(long const offset);

};

/******************************************************************************
 * EXTERN DECLARATION
 ******************************************************************************/

extern TimeServiceClass TimeService;

#endif /* ARDUINO_IOT_CLOUD_TIME_SERVICE_H_ */
