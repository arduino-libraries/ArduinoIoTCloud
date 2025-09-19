/*
  Copyright (c) 2025 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once
#include <Arduino.h>
#include <ANetworkConfigurator_Config.h>

#ifdef BOARD_USE_NINA
#include "WiFiNINA.h"
#include "utility/wifi_drv.h"
#endif

inline void LedFactoryTest() {
#if defined(BOARD_HAS_RGB)
#if defined(BOARD_USE_NINA)
  // MKR WiFi 1010, RP2040 Connect
  WiFiDrv::pinMode(GREEN_LED, OUTPUT);
  WiFiDrv::digitalWrite(GREEN_LED, LED_OFF);
  WiFiDrv::pinMode(BLUE_LED, OUTPUT);
  WiFiDrv::digitalWrite(BLUE_LED, LED_OFF);
  WiFiDrv::pinMode(RED_LED, OUTPUT);
  WiFiDrv::digitalWrite(RED_LED, LED_OFF);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LED_OFF);

  for(uint8_t i = 0; i<2; i++){
    digitalWrite(LED_BUILTIN, LED_ON);
    WiFiDrv::digitalWrite(RED_LED, LED_ON);
    delay(300);
    digitalWrite(LED_BUILTIN, LED_OFF);
    WiFiDrv::digitalWrite(RED_LED, LED_OFF);
    WiFiDrv::digitalWrite(GREEN_LED, LED_ON);
    delay(300);
    digitalWrite(LED_BUILTIN, LED_ON);
    WiFiDrv::digitalWrite(GREEN_LED, LED_OFF);
    WiFiDrv::digitalWrite(BLUE_LED, LED_ON);
    delay(300);
    digitalWrite(LED_BUILTIN, LED_OFF);
    WiFiDrv::digitalWrite(BLUE_LED, LED_OFF);
    delay(200);
  }
#else
  // Portenta H7, Giga, Nicla Vision, Portenta C33
  pinMode(GREEN_LED, OUTPUT);
  digitalWrite(GREEN_LED, LED_OFF);
  pinMode(RED_LED, OUTPUT);
  digitalWrite(RED_LED, LED_OFF);
  pinMode(BLUE_LED, OUTPUT);
  digitalWrite(BLUE_LED, LED_OFF);
  for(uint8_t i = 0; i<2; i++){
    digitalWrite(RED_LED, LED_ON);
    delay(300);
    digitalWrite(RED_LED, LED_OFF);
    digitalWrite(GREEN_LED, LED_ON);
    delay(300);
    digitalWrite(GREEN_LED, LED_OFF);
    digitalWrite(BLUE_LED, LED_ON);
    delay(300);
    digitalWrite(BLUE_LED, LED_OFF);
    delay(200);
  }
#endif
#else // Nano 33 IoT
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LED_OFF);
  uint32_t start = millis();
  while(millis() - start < 2000) {
    digitalWrite(LED_BUILTIN, LED_ON);
    delay(300);
    digitalWrite(LED_BUILTIN, LED_OFF);
    delay(300);
  }
#endif

}
