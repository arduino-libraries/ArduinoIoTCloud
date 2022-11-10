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

#ifndef __NTP_UTILS__
#define __NTP_UTILS__

#include "../../AIoTC_Config.h"
#ifndef HAS_LORA

/*
	This Utility Class is derived from the example code found here https://www.arduino.cc/en/Tutorial/UdpNTPClient
	For more information on NTP (Network Time Protocol) you can refer to this Wikipedia article https://en.wikipedia.org/wiki/Network_Time_Protocol
*/

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <Arduino.h>
#include <Udp.h>

/**************************************************************************************
 * CLASS DECLARATION
 **************************************************************************************/

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
