/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2020 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef ARDUINO_IOT_CLOUD_RTC_MILLIS_H_
#define ARDUINO_IOT_CLOUD_RTC_MILLIS_H_

#if !defined(BOARD_HAS_HW_RTC)

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

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

#endif /* BOARD_HAS_HW_RTC */

#endif /* ARDUINO_IOT_CLOUD_RTC_MILLIS_H_ */
