#ifndef __NTP_UTILS__
#define __NTP_UTILS__

#include "Udp.h"
#include <time.h>

class NTPUtils {
	public:
		NTPUtils(UDP& Udp);
		void sendNTPpacket(uint8_t* packetBuffer);
		unsigned long getTime();
		static bool isTimeValid(unsigned long time);
	private:
		const char* timeServer = "time.apple.com";
  		const int NTP_PACKET_SIZE = 48;
  		UDP& Udp;
};

#endif