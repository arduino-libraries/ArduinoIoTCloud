/*
  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "CSRHandler.h"
#include <utility/SElementArduinoCloudCertificate.h>
#include <utility/SElementCSR.h>
#include <utility/SElementArduinoCloudDeviceId.h>
#include <utility/SElementArduinoCloud.h>
#include <utility/SElementArduinoCloudJWT.h>
#include <utility/time/TimeService.h>
#include <stdlib.h>
#include "Arduino_DebugUtils.h"
#include <Arduino_HEX.h>

#define RESPONSE_TIMEOUT 5000
#define CONNECTION_RETRY_TIMEOUT_MS 2000
#define BACKEND_INTERVAL_s 12
#define MAX_CSR_REQUEST_INTERVAL 180000
#define MAX_CSR_REQUEST_INTERVAL_ATTEMPTS 15

#ifdef COMPILE_TEST
constexpr char *server = "boards-v2.oniudra.cc";
#else
constexpr char *server = "boards-v2.arduino.cc";
#endif

CSRHandlerClass::CSRHandlerClass() :
  _ledFeedback{LEDFeedbackClass::getInstance()},
  _state{CSRHandlerStates::END},
  _nextRequestAt{0},
  _requestAttempt{0},
  _startWaitingResponse{0},
  _uhwid{nullptr},
  _certForCSR{nullptr},
  _connectionHandler{nullptr},
  _secureElement{nullptr},
  _tlsClient{nullptr},
  _client{nullptr},
  _fw_version{""},
  _deviceId{""},
  _issueYear{0},
  _issueMonth{0},
  _issueDay{0},
  _issueHour{0} {
  memset(_serialNumber, 0, sizeof(_serialNumber));
  memset(_authorityKeyIdentifier, 0, sizeof(_authorityKeyIdentifier));
  memset(_signature, 0, sizeof(_signature));
}

CSRHandlerClass::~CSRHandlerClass() {
  if (_certForCSR) {
    delete _certForCSR;
    _certForCSR = nullptr;
  }
  if (_client) {
    delete _client;
    _client = nullptr;
  }

  if(_tlsClient){
    delete _tlsClient;
    _tlsClient = nullptr;
  }
}

bool CSRHandlerClass::begin(ConnectionHandler &connectionHandler, SecureElement &secureElement, String &uhwid) {
  if(_state != CSRHandlerStates::END) {
    return true;
  }

  if(uhwid == "") {
    return false;
  }

  _connectionHandler = &connectionHandler;
  _secureElement = &secureElement;
  _uhwid = &uhwid;

#ifdef BOARD_HAS_WIFI
  _fw_version = WiFi.firmwareVersion();
#endif
  if(!_tlsClient){
    _tlsClient = new TLSClientMqtt();
  }
  _tlsClient->begin(*_connectionHandler);
  _tlsClient->setTimeout(RESPONSE_TIMEOUT);
  _client = new HttpClient(*_tlsClient, server, 443);
  TimeService.begin(_connectionHandler);
  _requestAttempt = 0;
  _nextRequestAt = 0;
  _startWaitingResponse = 0;
  _state = CSRHandlerStates::BUILD_CSR;
}

void CSRHandlerClass::end() {
  if (_client) {
    _client->stop();
    delete _client;
    _client = nullptr;
  }

  if (_certForCSR) {
    delete _certForCSR;
    _certForCSR = nullptr;
  }

  if(_tlsClient){
    delete _tlsClient;
    _tlsClient = nullptr;
  }
  _fw_version = "";
  _deviceId = "";
  _state = CSRHandlerStates::END;
}

CSRHandlerClass::CSRHandlerStates CSRHandlerClass::poll() {
  switch (_state) {
    case CSRHandlerStates::BUILD_CSR:            _state = handleBuildCSR          (); break;
    case CSRHandlerStates::REQUEST_SIGNATURE:    _state = handleRequestSignature  (); break;
    case CSRHandlerStates::WAITING_RESPONSE:     _state = handleWaitingResponse   (); break;
    case CSRHandlerStates::PARSE_RESPONSE:       _state = handleParseResponse     (); break;
    case CSRHandlerStates::BUILD_CERTIFICATE:    _state = handleBuildCertificate  (); break;
    case CSRHandlerStates::CERT_CREATED:         _state = handleCertCreated       (); break;
    case CSRHandlerStates::WAITING_COMPLETE_RES: _state = handleWaitingCompleteRes(); break;
    case CSRHandlerStates::COMPLETED:                                                 break;
    case CSRHandlerStates::ERROR:                         handleError             (); break;
    case CSRHandlerStates::END:                                                       break;
  }

  return _state;
}

void CSRHandlerClass::updateNextRequestAt() {
  uint32_t delay;
  if(_requestAttempt <= MAX_CSR_REQUEST_INTERVAL_ATTEMPTS) {
    delay = BACKEND_INTERVAL_s * _requestAttempt * 1000; // use linear backoff since backend has a rate limit
  }else {
    delay = MAX_CSR_REQUEST_INTERVAL;
  }

  _nextRequestAt = millis() + delay + jitter();
}

uint32_t CSRHandlerClass::jitter(uint32_t base, uint32_t max) {
  srand(millis());
  return base + rand() % (max - base);
}

bool CSRHandlerClass::postRequest(const char *url, String &postData) {
  if(!_client){
    _client = new HttpClient(*_tlsClient, server, 443);
  }

  uint32_t ts = getTimestamp();
  if(ts == 0){
    DEBUG_WARNING("CSRH::%s Failed getting timestamp", __FUNCTION__);
    return false;
  }

  String token = getAIoTCloudJWT(*_secureElement, *_uhwid, ts, 1);

  _requestAttempt++;
  _client->beginRequest();

  if(_client->post(url) == 0){
    _client->sendHeader("Host", server);
    _client->sendHeader("Connection", "close");
    _client->sendHeader("Content-Type", "application/json;charset=UTF-8");
    _client->sendHeader("Authorization", "Bearer " + token);
    _client->sendHeader("Content-Length", postData.length());
    _client->beginBody();
    _client->print(postData);
    _startWaitingResponse = millis();
    return true;
  }
  return false;
}

uint32_t CSRHandlerClass::getTimestamp() {
  uint8_t getTsAttempt = 0;
  uint32_t ts = 0;
  do{
    TimeService.sync();
    ts = TimeService.getTime();
    getTsAttempt++;
  }while(ts == 0 && getTsAttempt < 3);

  return ts;
}

CSRHandlerClass::CSRHandlerStates CSRHandlerClass::handleBuildCSR() {
  if (!_certForCSR) {
    _certForCSR = new ECP256Certificate();
  }
  _certForCSR->begin();

  _certForCSR->setSubjectCommonName(*_uhwid);

  if (!SElementCSR::build(*_secureElement, *_certForCSR, static_cast<int>(SElementArduinoCloudSlot::Key), true)) {
    DEBUG_ERROR("CSRH::%s Error generating CSR!", __FUNCTION__);
    _ledFeedback.setMode(LEDFeedbackClass::LEDFeedbackMode::ERROR);
    return CSRHandlerStates::ERROR;
  }
  return CSRHandlerStates::REQUEST_SIGNATURE;
}

CSRHandlerClass::CSRHandlerStates CSRHandlerClass::handleRequestSignature() {
  CSRHandlerStates nextState = _state;

  if(millis() < _nextRequestAt) {
    return nextState;
  }

  NetworkConnectionState connectionRes = _connectionHandler->check();
  if (connectionRes != NetworkConnectionState::CONNECTED) {
    nextNetworkRetry();
    return nextState;
  }

  if(!_certForCSR){
    return CSRHandlerStates::BUILD_CSR;
  }

  String csr = _certForCSR->getCSRPEM();
  csr.replace("\n", "\\n");

  String PostData = "{\"csr\":\"";
  PostData += csr;
  PostData += "\"}";
  DEBUG_INFO("CSRH Downloading certificate...");

  if(postRequest("/provisioning/v1/onboarding/provision/csr", PostData)){
    nextState = CSRHandlerStates::WAITING_RESPONSE;
  } else {
    updateNextRequestAt();
    DEBUG_WARNING("CSRH::%s Failed sending request, retrying in %d ms", __FUNCTION__, _nextRequestAt - millis());
  }

  return nextState;
}

CSRHandlerClass::CSRHandlerStates CSRHandlerClass::handleWaitingResponse() {
  CSRHandlerStates nextState = _state;
  NetworkConnectionState connectionRes = _connectionHandler->check();
  if (connectionRes != NetworkConnectionState::CONNECTED) {
    nextNetworkRetry();
    _client->stop();
    return CSRHandlerStates::REQUEST_SIGNATURE;
  }

  if (millis() - _startWaitingResponse > RESPONSE_TIMEOUT) {
    _client->stop();
    updateNextRequestAt();
    DEBUG_WARNING("CSRH::%s CSR request timeout, retrying in %d ms", __FUNCTION__, _nextRequestAt - millis());
    nextState = CSRHandlerStates::REQUEST_SIGNATURE;
  }


  int statusCode = _client->responseStatusCode();
  if(statusCode == 200){
    nextState = CSRHandlerStates::PARSE_RESPONSE;
  } else {
    _client->stop();
    updateNextRequestAt();
    DEBUG_WARNING("CSRH::%s CSR request error code %d, retrying in %d ms", __FUNCTION__, statusCode ,_nextRequestAt - millis());
    nextState = CSRHandlerStates::REQUEST_SIGNATURE;
  }

  return nextState;
}

CSRHandlerClass::CSRHandlerStates CSRHandlerClass::handleParseResponse() {
  String certResponse = _client->responseBody();
  _client->stop();

  /* Parse the response in format:
   * device_id|authority_key_identifier|not_before|serial|signature_asn1_x|signature_asn1_y
   */
  char *response = (char *)certResponse.c_str();
  char *token[6];
  int i = 1;
  token[0] = strtok(response, "|");
  for (; i < 6; i++) {
    char *tok = strtok(NULL, "|");
    if(tok == NULL){
      break;
    }
    token[i] = tok;
  }

  if(i < 6 || strlen(token[0]) != 36 || strlen(token[1]) != 40
   || strlen(token[2]) < 10 || strlen(token[3]) != 32
   || strlen(token[4]) != 64 || strlen(token[5]) != 64
   || sscanf(token[2], "%4d-%2d-%2dT%2d", &_issueYear, &_issueMonth, &_issueDay, &_issueHour) != 4){
    updateNextRequestAt();
    DEBUG_ERROR("CSRH::%s Error parsing response, retrying in %d ms", __FUNCTION__, _nextRequestAt - millis());
    return CSRHandlerStates::REQUEST_SIGNATURE;
  }

  _deviceId = token[0];
  hex::decode(token[1], _authorityKeyIdentifier, sizeof(_authorityKeyIdentifier));
  hex::decode(token[3], _serialNumber, sizeof(_serialNumber));
  hex::decode(token[4], _signature, sizeof(_signature));
  hex::decode(token[5], &_signature[32], sizeof(_signature) - 32);

  return CSRHandlerStates::BUILD_CERTIFICATE;
}

CSRHandlerClass::CSRHandlerStates CSRHandlerClass::handleBuildCertificate() {
  int expireYears = 31;

  if (!SElementArduinoCloudDeviceId::write(*_secureElement, _deviceId, SElementArduinoCloudSlot::DeviceId)) {
    DEBUG_ERROR("CSRH::%s Error storing device id!", __FUNCTION__);
    _ledFeedback.setMode(LEDFeedbackClass::LEDFeedbackMode::ERROR);
    return CSRHandlerStates::ERROR;
  }

  ECP256Certificate cert;
  cert.begin();

  cert.setSubjectCommonName(_deviceId);
  cert.setIssuerCountryName("US");
  cert.setIssuerOrganizationName("Arduino LLC US");
  cert.setIssuerOrganizationalUnitName("IT");
  cert.setIssuerCommonName("Arduino");
  cert.setSignature(_signature, sizeof(_signature));
  cert.setAuthorityKeyId(_authorityKeyIdentifier, sizeof(_authorityKeyIdentifier));
  cert.setSerialNumber(_serialNumber, sizeof(_serialNumber));
  cert.setIssueYear(_issueYear);
  cert.setIssueMonth(_issueMonth);
  cert.setIssueDay(_issueDay);
  cert.setIssueHour(_issueHour);
  cert.setExpireYears(expireYears);

  if (!SElementArduinoCloudCertificate::build(*_secureElement, cert, static_cast<int>(SElementArduinoCloudSlot::Key))) {
    DEBUG_ERROR("CSRH::%s Error building secureElement compressed cert!", __FUNCTION__);
    _ledFeedback.setMode(LEDFeedbackClass::LEDFeedbackMode::ERROR);
    return CSRHandlerStates::ERROR;
  }

  if (!SElementArduinoCloudCertificate::write(*_secureElement, cert, SElementArduinoCloudSlot::CompressedCertificate)) {
    DEBUG_ERROR("CSRH::%s Error storing cert!" , __FUNCTION__);
    _ledFeedback.setMode(LEDFeedbackClass::LEDFeedbackMode::ERROR);
    return CSRHandlerStates::ERROR;
  }

  DEBUG_INFO("CSRH Certificate created!");
  _nextRequestAt = 0;
  _requestAttempt = 0;
  return CSRHandlerStates::CERT_CREATED;
}

CSRHandlerClass::CSRHandlerStates CSRHandlerClass::handleCertCreated() {
  CSRHandlerStates nextState = _state;

  if(millis() < _nextRequestAt) {
    return nextState;
  }

  NetworkConnectionState connectionRes = _connectionHandler->check();
  if (connectionRes != NetworkConnectionState::CONNECTED) {
    nextNetworkRetry();
    return nextState;
  }

  String PostData = "{\"wifi_fw_version\":\"";
  PostData += _fw_version;
  PostData += "\"}";
  if(postRequest("/provisioning/v1/onboarding/provision/complete", PostData)){
    nextState = CSRHandlerStates::WAITING_COMPLETE_RES;
  } else {
    updateNextRequestAt();
    DEBUG_WARNING("CSRH::%s Error sending complete request, retrying in %d ms", __FUNCTION__, _nextRequestAt - millis());
  }

  return nextState;
}

CSRHandlerClass::CSRHandlerStates CSRHandlerClass::handleWaitingCompleteRes() {
  CSRHandlerStates nextState = _state;
  NetworkConnectionState connectionRes = _connectionHandler->check();
  if (connectionRes != NetworkConnectionState::CONNECTED) {
    nextNetworkRetry();
    _client->stop();
    return CSRHandlerStates::CERT_CREATED;
  }

  if (millis() - _startWaitingResponse > RESPONSE_TIMEOUT) {
    _client->stop();
    updateNextRequestAt();
    DEBUG_WARNING("CSRH::%s Complete request timeout, retrying in %d ms", __FUNCTION__, _nextRequestAt - millis());
    nextState = CSRHandlerStates::CERT_CREATED;
  }

  int statusCode = _client->responseStatusCode();
  if(statusCode == 200){
    if (_certForCSR) {
      delete _certForCSR;
      _certForCSR = nullptr;
    }
    DEBUG_INFO("CSRH Provisioning completed!");
    nextState = CSRHandlerStates::COMPLETED;
  } else if (statusCode == 429 || statusCode == 503) {
    updateNextRequestAt();
    nextState = CSRHandlerStates::CERT_CREATED;
  } else {
    DEBUG_WARNING("CSRH::%s Complete request error code %d, retrying in %d ms", __FUNCTION__, statusCode ,_nextRequestAt - millis());
    _requestAttempt = 0;
    _nextRequestAt = 0;
    nextState = CSRHandlerStates::REQUEST_SIGNATURE;
  }
  _client->stop();

  return nextState;
}

void CSRHandlerClass::nextNetworkRetry() {
  _nextRequestAt = millis() + CONNECTION_RETRY_TIMEOUT_MS;
}

void CSRHandlerClass::handleError() {
  _ledFeedback.update();
}
