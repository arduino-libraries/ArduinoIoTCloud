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

#include "../../AIoTC_Config.h"
#ifndef HAS_LORA

#include "NTPUtils.h"

#include <Arduino.h>
#ifdef BOARD_HAS_ECCX08
  #include <ArduinoECCX08.h>
#endif

/**************************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 **************************************************************************************/

unsigned long NTPUtils::getTime(UDP & udp)
{
#ifdef NTP_USE_RANDOM_PORT
  udp.begin(NTPUtils::getRandomPort(MIN_NTP_PORT, MAX_NTP_PORT));
#else
  udp.begin(NTP_LOCAL_PORT);
#endif

  sendNTPpacket(udp);

  bool is_timeout = false;
  unsigned long const start = millis();
  do
  {
    is_timeout = (millis() - start) >= NTP_TIMEOUT_MS;
  } while(!is_timeout && !udp.parsePacket());

  if(is_timeout) {
    udp.stop();
    return 0;
  }

  uint8_t ntp_packet_buf[NTP_PACKET_SIZE];
  udp.read(ntp_packet_buf, NTP_PACKET_SIZE);
  udp.stop();

  unsigned long const highWord      = word(ntp_packet_buf[40], ntp_packet_buf[41]);
  unsigned long const lowWord       = word(ntp_packet_buf[42], ntp_packet_buf[43]);
  unsigned long const secsSince1900 = highWord << 16 | lowWord;
  unsigned long const seventyYears  = 2208988800UL;
  unsigned long const epoch         = secsSince1900 - seventyYears;

  return epoch;
}

/**************************************************************************************
 * PRIVATE MEMBER FUNCTIONS
 **************************************************************************************/

void NTPUtils::sendNTPpacket(UDP & udp)
{
  uint8_t ntp_packet_buf[NTP_PACKET_SIZE] = {0};

  ntp_packet_buf[0]  = 0b11100011;
  ntp_packet_buf[1]  = 0;
  ntp_packet_buf[2]  = 6;
  ntp_packet_buf[3]  = 0xEC;
  ntp_packet_buf[12] = 49;
  ntp_packet_buf[13] = 0x4E;
  ntp_packet_buf[14] = 49;
  ntp_packet_buf[15] = 52;

  udp.beginPacket(NTP_TIME_SERVER, NTP_TIME_SERVER_PORT);
  udp.write(ntp_packet_buf, NTP_PACKET_SIZE);
  udp.endPacket();
}

#ifdef NTP_USE_RANDOM_PORT
int NTPUtils::getRandomPort(int const min_port, int const max_port)
{
#if defined (ARDUINO_ARCH_ESP8266) || (ARDUINO_ARCH_ESP32) || \
            (ARDUINO_ARCH_RENESAS) || (ARDUINO_ARCH_MBED)
  /* Uses HW Random Number Generator */
#elif defined (ARDUINO_ARCH_SAMD)
  /* Use ADC to generate a seed */
  randomSeed(adcSeed());
#else
  randomSeed(analogRead(0));
#endif
  return random(min_port, max_port);
}

#if defined (ARDUINO_ARCH_SAMD)
unsigned long NTPUtils::adcSeed()
{
  uint32_t seed = 0;
  uint32_t bitCount = 0;
  uint16_t sampctlr = ADC->SAMPCTRL.reg;

  // Use lowest sampling time
  ADC->SAMPCTRL.reg  = 0;
  // Enable ADC
  ADC->CTRLA.bit.ENABLE = 1;
  while (ADC->STATUS.bit.SYNCBUSY == 1);
  do {
    uint16_t adcReading;
    // Start ADC conversion
    ADC->SWTRIG.bit.START = 1;
    // Wait until ADC conversion is done
    while (!(ADC->INTFLAG.bit.RESRDY));
    while (ADC->STATUS.bit.SYNCBUSY == 1);
    // Get result
    adcReading = ADC->RESULT.reg;
    // Clear result ready flag
    ADC->INTFLAG.reg = ADC_INTFLAG_RESRDY;
    while (ADC->STATUS.bit.SYNCBUSY == 1);
    // Take least significant bit
    uint8_t b0 = adcReading & 0x0001;
    ADC->SWTRIG.bit.START = 1;
    // Wait until ADC conversion is done
    while (!(ADC->INTFLAG.bit.RESRDY));
    while (ADC->STATUS.bit.SYNCBUSY == 1);
    // Get result
    adcReading = ADC->RESULT.reg;
    // Clear result ready flag
    ADC->INTFLAG.reg = ADC_INTFLAG_RESRDY;
    while (ADC->STATUS.bit.SYNCBUSY == 1);
    // Take least significant nibble
    uint8_t b1 = adcReading & 0x0001;
    if (b0 == b1) {
     continue;
    }
    seed |= b0 << bitCount;
    bitCount++;
  } while(bitCount < 32);
  // Disable ADC
  ADC->CTRLA.bit.ENABLE = 0;
  // restore original sampling time
  ADC->SAMPCTRL.reg = sampctlr;
  return seed;
}
#endif /* ARDUINO_ARCH_SAMD */

#endif /* NTP_USE_RANDOM_PORT */

#endif /* !HAS_LORA */
