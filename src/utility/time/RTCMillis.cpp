/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2020 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include "AIoTC_Config.h"

#if !defined(BOARD_HAS_HW_RTC)

#include <Arduino.h>
#include "RTCMillis.h"

/******************************************************************************
 * CTOR/DTOR
 ******************************************************************************/

RTCMillis::RTCMillis()
: _last_rtc_update_tick(0)
, _last_rtc_update_value(0)
{

}

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

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
  _last_rtc_update_value += elapsed_s;
  _last_rtc_update_tick += elapsed_s * 1000;
  return _last_rtc_update_value;
}

#endif /* BOARD_HAS_HW_RTC */
