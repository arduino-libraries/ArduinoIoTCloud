#ifndef __NTP_UTILS__
#define __NTP_UTILS__
/*
	This Utility Class is derived from the example code found here https://www.arduino.cc/en/Tutorial/UdpNTPClient
	For more information on NTP (Network Time Protocol) you can refer to this Wikipedia article https://en.wikipedia.org/wiki/Network_Time_Protocol
*/

#include "Udp.h"
#include <time.h>

class NTPUtils {
  public:
    NTPUtils(UDP& Udp);
    void sendNTPpacket(uint8_t* packetBuffer);
    unsigned long getTime();
    static bool isTimeValid(unsigned long time);
  private:
    const char* timeServer = "time.arduino.cc";
    const int NTP_PACKET_SIZE = 48;
    UDP& Udp;
};

#endif
