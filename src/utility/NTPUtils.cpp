#include "NTPUtils.h"
#include "Arduino.h"
/*
  This Utility Class is derived from the example code found here https://www.arduino.cc/en/Tutorial/UdpNTPClient
  For more information on NTP (Network Time Protocol) you can refer to this Wikipedia article https://en.wikipedia.org/wiki/Network_Time_Protocol
*/


// could be a constexpr in C++14
static time_t cvt_TIME(char const *time) {
  char s_month[5];
  int month, day, year;
  struct tm t = {0 /* tm_sec   */,
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


NTPUtils::NTPUtils(UDP& Udp) : Udp(Udp) {}

bool NTPUtils::isTimeValid(unsigned long time) {
  return (time > static_cast<unsigned long>(cvt_TIME(__DATE__)));
}

void NTPUtils::sendNTPpacket(uint8_t* packetBuffer) {
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011;
  packetBuffer[1] = 0;
  packetBuffer[2] = 6;
  packetBuffer[3] = 0xEC;
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  Udp.beginPacket(timeServer, 123);
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

unsigned long NTPUtils::getTime() {

  unsigned int localPort = 8888;
  uint8_t packetBuffer[NTP_PACKET_SIZE];

  Udp.begin(localPort);
  sendNTPpacket(packetBuffer);
  long start = millis();
  while (!Udp.parsePacket() && (millis() - start < 10000)) {}
  if (millis() - start >= 1000) {
    //timeout reached
    Udp.stop();
    return 0;
  }
  Udp.read(packetBuffer, NTP_PACKET_SIZE);

  unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
  unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
  unsigned long secsSince1900 = highWord << 16 | lowWord;
  const unsigned long seventyYears = 2208988800UL;
  unsigned long epoch = secsSince1900 - seventyYears;

  Udp.stop();
  return epoch;
}
