/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2020 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef __NTP_UTILS__
#define __NTP_UTILS__

#include "../../AIoTC_Config.h"
#ifndef HAS_LORA

/*
	This Utility Class is derived from the example code found here
  https://www.arduino.cc/en/Tutorial/UdpNTPClient
	For more information on NTP (Network Time Protocol) you can refer to this
  Wikipedia article https://en.wikipedia.org/wiki/Network_Time_Protocol
*/

/******************************************************************************
  INCLUDE
 ******************************************************************************/

#include <Arduino.h>
#include <Udp.h>

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class NTPUtils
{
public:

  static unsigned long getTime(UDP & udp);
  static int getRandomPort(int const min_port, int const max_port);

private:

  static size_t        const NTP_PACKET_SIZE      = 48;
  static int           const NTP_TIME_SERVER_PORT = 123;
  static int           const NTP_LOCAL_PORT       = 8888;
#if NTP_USE_RANDOM_PORT
  static int           const MIN_NTP_PORT         = 49152;
  static int           const MAX_NTP_PORT         = 65535;
#endif
  static unsigned long const NTP_TIMEOUT_MS       = 1000;
  static constexpr const char * NTP_TIME_SERVER   = "time.arduino.cc";

  static void sendNTPpacket(UDP & udp);
};

#endif /* #ifndef HAS_LORA */

#endif
