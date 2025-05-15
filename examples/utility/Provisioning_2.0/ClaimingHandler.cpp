/*
  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "ClaimingHandler.h"
#include <utility/SElementArduinoCloudJWT.h>
#include "Arduino_DebugUtils.h"
#include <ArduinoBLE.h>
#include "utility/HCI.h"
#include <Arduino_HEX.h>

extern const char *SKETCH_VERSION;

ClaimingHandlerClass::ClaimingHandlerClass():
  _uhwid {nullptr},
  _state {ClaimingHandlerStates::END},
  _secureElement {nullptr},
  _clearStoredCredentials {nullptr},
  _agentManager { AgentsManagerClass::getInstance()},
  _ledFeedback {LEDFeedbackClass::getInstance()} {
  _receivedEvent = ClaimingReqEvents::NONE;
  _ts = 0;
}

bool ClaimingHandlerClass::begin(SecureElement &secureElement, String &uhwid, ClearStoredCredentialsHandler clearStoredCredentials) {
  if(_state != ClaimingHandlerStates::END) {
    return true;
  }

  if(uhwid == "" || clearStoredCredentials == nullptr) {
    return false;
  }

  if (!_agentManager.addRequestHandler(RequestType::GET_ID, getIdRequestCb)) {
    return false;
  }

  if (!_agentManager.addRequestHandler(RequestType::RESET, resetStoredCredRequestCb)) {
    return false;
  }

  if(!_agentManager.addRequestHandler(RequestType::GET_BLE_MAC_ADDRESS, getBLEMacAddressRequestCb)) {
    return false;
  }

  if(!_agentManager.addRequestHandler(RequestType::GET_PROVISIONING_SKETCH_VERSION, getProvSketchVersionRequestCb)) {
    return false;
  }

  if (!_agentManager.addReturnTimestampCallback(setTimestamp)) {
    return false;
  }

  _agentManager.begin();
  _uhwid = &uhwid;
  _secureElement = &secureElement;
  _clearStoredCredentials = clearStoredCredentials;
  _state = ClaimingHandlerStates::INIT;
}

void ClaimingHandlerClass::end() {
  if(_state == ClaimingHandlerStates::END) {
    return;
  }

  _agentManager.removeReturnTimestampCallback();
  _agentManager.removeRequestHandler(RequestType::GET_ID);
  _agentManager.removeRequestHandler(RequestType::RESET);
  _agentManager.end();
  _state = ClaimingHandlerStates::END;
}

void ClaimingHandlerClass::poll() {
  if(_state == ClaimingHandlerStates::END) {
    return;
  }
  _ledFeedback.update();
  _agentManager.update();

  switch (_receivedEvent) {
    case ClaimingReqEvents::GET_ID:                  getIdReqHandler               (); break;
    case ClaimingReqEvents::RESET:                   resetStoredCredReqHandler     (); break;
    case ClaimingReqEvents::GET_BLE_MAC_ADDRESS:     getBLEMacAddressReqHandler    (); break;
    case ClaimingReqEvents::GET_PROV_SKETCH_VERSION: getProvSketchVersionReqHandler(); break;
  }
  _receivedEvent = ClaimingReqEvents::NONE;
  return;
}

void ClaimingHandlerClass::getIdReqHandler() {
  if (_ts != 0) {
    byte _uhwidBytes[32];
    hex::decode(_uhwid->c_str(), _uhwidBytes, _uhwid->length());
    //Send UHWID
    ProvisioningOutputMessage idMsg = {MessageOutputType::UHWID};
    idMsg.m.uhwid = _uhwidBytes;
    _agentManager.sendMsg(idMsg);

    String token = getAIoTCloudJWT(*_secureElement, *_uhwid, _ts, 1);
    if (token == "") {
      DEBUG_ERROR("CH::%s Error: token not created", __FUNCTION__);
      sendStatus(StatusMessage::ERROR);
      return;
    }

    //Send JWT
    ProvisioningOutputMessage jwtMsg = {MessageOutputType::JWT};
    jwtMsg.m.jwt = token.c_str();
    _agentManager.sendMsg(jwtMsg);
    _ts = 0;
  } else {
    DEBUG_ERROR("CH::%s Error: timestamp not provided" , __FUNCTION__);
    sendStatus(StatusMessage::PARAMS_NOT_FOUND);
  }
}

void ClaimingHandlerClass::resetStoredCredReqHandler() {
  if( !_clearStoredCredentials()){
    DEBUG_ERROR("CH::%s Error: reset stored credentials failed", __FUNCTION__);
    sendStatus(StatusMessage::ERROR);
  } else {
    sendStatus(StatusMessage::RESET_COMPLETED);
  }

}

void ClaimingHandlerClass::getBLEMacAddressReqHandler() {
  uint8_t mac[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

  bool activated = false;
  ConfiguratorAgent * connectedAgent = _agentManager.getConnectedAgent();
  if(!_agentManager.isAgentEnabled(ConfiguratorAgent::AgentTypes::BLE) || (connectedAgent != nullptr &&
    connectedAgent->getAgentType() != ConfiguratorAgent::AgentTypes::BLE)) {
      activated = true;
      BLE.begin();
  }

  HCI.readBdAddr(mac);

  for(int i = 0; i < 3; i++){
    uint8_t byte = mac[i];
    mac[i] = mac[5-i];
    mac[5-i] = byte;
  }
  if (activated) {
    BLE.end();
  }

  ProvisioningOutputMessage outputMsg;
  outputMsg.type = MessageOutputType::BLE_MAC_ADDRESS;
  outputMsg.m.BLEMacAddress = mac;
  _agentManager.sendMsg(outputMsg);
}

void ClaimingHandlerClass::getProvSketchVersionReqHandler() {
  ProvisioningOutputMessage outputMsg;
  outputMsg.type = MessageOutputType::PROV_SKETCH_VERSION;
  outputMsg.m.provSketchVersion = SKETCH_VERSION;
  _agentManager.sendMsg(outputMsg);
}

void ClaimingHandlerClass::getIdRequestCb() {
  DEBUG_VERBOSE("CH Get ID request received");
  _receivedEvent = ClaimingReqEvents::GET_ID;
}
void ClaimingHandlerClass::setTimestamp(uint64_t ts) {
  _ts = ts;
}

void ClaimingHandlerClass::resetStoredCredRequestCb() {
  DEBUG_VERBOSE("CH Reset stored credentials request received");
  _receivedEvent = ClaimingReqEvents::RESET;
}

void ClaimingHandlerClass::getBLEMacAddressRequestCb() {
  DEBUG_VERBOSE("CH Get BLE MAC address request received");
  _receivedEvent = ClaimingReqEvents::GET_BLE_MAC_ADDRESS;
}

void ClaimingHandlerClass::getProvSketchVersionRequestCb() {
  DEBUG_VERBOSE("CH Get provisioning sketch version request received");
  _receivedEvent = ClaimingReqEvents::GET_PROV_SKETCH_VERSION;
}

bool ClaimingHandlerClass::sendStatus(StatusMessage msg) {
  ProvisioningOutputMessage statusMsg = { MessageOutputType::STATUS, { msg } };
  return _agentManager.sendMsg(statusMsg);
}
