/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include <Arduino_ConnectionHandler.h>
#include <AIoTC_Config.h>

enum class ArduinoIoTAuthenticationMode
{
  PASSWORD,
  CERTIFICATE
};

#if defined(BOARD_HAS_OFFLOADED_ECCX08)
  /*
   * Arduino MKR WiFi1010 - WiFi
   * Arduino NANO 33 IoT  - WiFi
   */
  #include "WiFiSSLClient.h"
  class TLSClientBroker : public WiFiBearSSLClient {
#elif defined(BOARD_HAS_ECCX08)
  /*
   * Arduino MKR GSM 1400
   * Arduino MKR NB 1500
   * Arduino NANO RP 2040
   * Arduino Portenta H7
   * Arduino Giga R1
   * OPTA
   */
  #include <ArduinoBearSSLConfig.h>
  #include <ArduinoBearSSL.h>
  class TLSClientBroker : public BearSSLClient {
#elif defined(ARDUINO_PORTENTA_C33)
  /*
   * Arduino Portenta C33
   */
  #include <SSLClient.h>
  class TLSClientBroker : public SSLClient {
#elif defined(ARDUINO_NICLA_VISION)
  /*
   * Arduino Nicla Vision
   */
  #include <WiFiSSLSE050Client.h>
  class TLSClientBroker : public WiFiSSLSE050Client {
#elif defined(ARDUINO_EDGE_CONTROL)
  /*
   * Arduino Edge Control
   */
  #include <GSMSSLClient.h>
  class TLSClientBroker : public GSMSSLClient {
#elif defined(ARDUINO_UNOR4_WIFI)
  /*
   * Arduino UNO R4 WiFi
   */
  #include <WiFiSSLClient.h>
  class TLSClientBroker : public WiFiSSLClient {
#elif defined(BOARD_ESP) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
  /*
   * ESP32*
   * ESP82*
   * PICOW
   */
  #include <WiFiClientSecure.h>
  class TLSClientBroker : public WiFiClientSecure {
#endif

public:
  void begin(Client* client, ArduinoIoTAuthenticationMode authMode = ArduinoIoTAuthenticationMode::CERTIFICATE);

};
