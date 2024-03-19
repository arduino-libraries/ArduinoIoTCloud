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

#if defined(BOARD_HAS_OFFLOADED_ECCX08)
  /*
   * Arduino MKR WiFi1010 - WiFi
   * Arduino NANO 33 IoT  - WiFi
   */
  #include "WiFiSSLClient.h"
  class TLSClientMqtt : public WiFiBearSSLClient {
#elif defined(BOARD_HAS_ECCX08)
  /*
   * Arduino MKR GSM 1400
   * Arduino MKR NB 1500
   * Arduino Portenta H7
   * Arduino Giga R1
   * OPTA
   */
  #include <tls/BearSSLClient.h>
  class TLSClientMqtt : public BearSSLClient {
#elif defined(ARDUINO_PORTENTA_C33)
  /*
   * Arduino Portenta C33
   */
  #include <SSLClient.h>
  class TLSClientMqtt : public SSLClient {
#elif defined(ARDUINO_NICLA_VISION)
  /*
   * Arduino Nicla Vision
   */
  #include <WiFiSSLSE050Client.h>
  class TLSClientMqtt : public WiFiSSLSE050Client {
#elif defined(ARDUINO_EDGE_CONTROL)
  /*
   * Arduino Edge Control
   */
  #include <GSMSSLClient.h>
  class TLSClientMqtt : public GSMSSLClient {
#elif defined(ARDUINO_UNOR4_WIFI)
  /*
   * Arduino UNO R4 WiFi
   */
  #include <WiFiSSLClient.h>
  class TLSClientMqtt : public WiFiSSLClient {
#elif defined(BOARD_ESP)
  /*
   * ESP32*
   * ESP82*
   */
  #include <WiFiClientSecure.h>
  class TLSClientMqtt : public WiFiClientSecure {
#endif

public:
  void begin(ConnectionHandler & connection);

};
