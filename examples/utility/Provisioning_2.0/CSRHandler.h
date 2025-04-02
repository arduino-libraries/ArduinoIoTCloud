/*
  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once
#include <Arduino.h>
#include <Arduino_ConnectionHandler.h>
#include <Arduino_SecureElement.h>
#include <tls/utility/TLSClientMqtt.h>
#include <ArduinoHttpClient.h>
#include "utility/LEDFeedback.h"
#define JITTER_BASE 0
#define JITTER_MAX 1000

class CSRHandlerClass {
public:
  CSRHandlerClass();
  ~CSRHandlerClass();
  enum class CSRHandlerStates {
    BUILD_CSR,
    REQUEST_SIGNATURE,
    WAITING_RESPONSE,
    PARSE_RESPONSE,
    BUILD_CERTIFICATE,
    CERT_CREATED,
    WAITING_COMPLETE_RES,
    COMPLETED,
    ERROR,
    END
  };
  bool begin(ConnectionHandler &connectionHandler, SecureElement &secureElement, String &uhwid);
  void end();
  CSRHandlerStates poll();
private:
  CSRHandlerStates _state;
  unsigned long _nextRequestAt;
  uint32_t _requestAttempt;
  uint32_t _startWaitingResponse;
  String *_uhwid;
  String _fw_version;

  int _issueYear;
  uint8_t _issueMonth;
  uint8_t _issueDay;
  uint8_t _issueHour;
  byte _serialNumber[16];
  byte _authorityKeyIdentifier[20];
  byte _signature[64];
  String _deviceId;

  ECP256Certificate *_certForCSR;
  ConnectionHandler *_connectionHandler;
  SecureElement *_secureElement;
  TLSClientMqtt *_tlsClient;
  HttpClient *_client;
  LEDFeedbackClass &_ledFeedback;
  void updateNextRequestAt();
  void nextNetworkRetry();
  uint32_t jitter(uint32_t base = JITTER_BASE, uint32_t max = JITTER_MAX);
  bool postRequest(const char *url, String &postData);
  uint32_t getTimestamp();
  CSRHandlerStates handleBuildCSR();
  CSRHandlerStates handleRequestSignature();
  CSRHandlerStates handleWaitingResponse();
  CSRHandlerStates handleParseResponse();
  CSRHandlerStates handleBuildCertificate();
  CSRHandlerStates handleCertCreated();
  CSRHandlerStates handleWaitingCompleteRes();
  void handleError();
};
