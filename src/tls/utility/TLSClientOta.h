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
  class TLSClientOta : public WiFiBearSSLClient {
#elif defined(BOARD_HAS_ECCX08)
  /*
   * Arduino MKR GSM 1400
   * Arduino MKR NB 1500
   * Arduino Portenta H7
   * Arduino Giga R1
   * OPTA
   */
  #include <tls/BearSSLClient.h>
  class TLSClientOta : public BearSSLClient {
#elif defined(ARDUINO_PORTENTA_C33)
  /*
   * Arduino Portenta C33
   */
  #include <SSLClient.h>
  class TLSClientOta : public SSLClient {
#elif defined(ARDUINO_NICLA_VISION)
  /*
   * Arduino Nicla Vision
   */
  #include <WiFiSSLSE050Client.h>
  class TLSClientOta : public WiFiSSLSE050Client {
#elif defined(ARDUINO_EDGE_CONTROL)
  /*
   * Arduino Edge Control
   */
  #include <GSMSSLClient.h>
  class TLSClientOta : public GSMSSLClient {
#elif defined(ARDUINO_UNOR4_WIFI)
  /*
   * Arduino UNO R4 WiFi
   */
  #include <WiFiSSLClient.h>
  class TLSClientOta : public WiFiSSLClient {
#elif defined(BOARD_ESP)
  /*
   * ESP32*
   * ESP82*
   */
  #include <WiFiClientSecure.h>
  class TLSClientOta : public WiFiClientSecure {
#endif

public:
  void begin(ConnectionHandler & connection);

private:
  inline Client* getNewClient(NetworkAdapter net) {
    switch(net) {
#ifdef BOARD_HAS_WIFI
    case NetworkAdapter::WIFI:
      return new WiFiClient();
#endif // BOARD_HAS_WIFI
#ifdef BOARD_HAS_ETHERNET
    case NetworkAdapter::ETHERNET:
      return new EthernetClient();
#endif // BOARD_HAS_ETHERNET
#ifdef BOARD_HAS_NB
    case NetworkAdapter::NB:
      return new NBClient();
#endif // BOARD_HAS_NB
#ifdef BOARD_HAS_GSM
    case NetworkAdapter::GSM:
      return new GSMClient();
#endif // BOARD_HAS_GSM
#ifdef BOARD_HAS_CATM1_NBIOT
    case NetworkAdapter::CATM1:
      return new GSMClient();
#endif // BOARD_HAS_CATM1_NBIOT
    default:
      return nullptr;
    }
  }
};
