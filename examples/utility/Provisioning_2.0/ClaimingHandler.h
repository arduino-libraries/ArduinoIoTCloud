/*
  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once
#include "Arduino.h"
#include "configuratorAgents/AgentsManager.h"
#include <Arduino_SecureElement.h>
#include "utility/LEDFeedback.h"

typedef bool (*ClearStoredCredentialsHandler)();
class ClaimingHandlerClass {
public:
  ClaimingHandlerClass();
  bool begin(SecureElement &secureElement, String &uhwid, ClearStoredCredentialsHandler clearStoredCredentials);
  void end();
  void poll();
private:
  String *_uhwid;
  enum class ClaimingHandlerStates {
    INIT,
    END
  };
  enum class ClaimingReqEvents { NONE,
                                 GET_ID,
                                 RESET,
                                 GET_BLE_MAC_ADDRESS,
                                 GET_PROV_SKETCH_VERSION};
  static inline ClaimingReqEvents _receivedEvent;
  ClaimingHandlerStates _state;
  AgentsManagerClass &_agentManager;
  LEDFeedbackClass &_ledFeedback;
  static inline uint64_t _ts;
  SecureElement *_secureElement;
  String generateToken();

  bool sendStatus(StatusMessage msg);
  /* Commands handlers */
  void getIdReqHandler();
  void resetStoredCredReqHandler();
  void getBLEMacAddressReqHandler();
  void getProvSketchVersionReqHandler();
  ClearStoredCredentialsHandler _clearStoredCredentials;
  /* Callbacks for receiving commands */
  static void getIdRequestCb();
  static void setTimestamp(uint64_t ts);
  static void resetStoredCredRequestCb();
  static void getBLEMacAddressRequestCb();
  static void getProvSketchVersionRequestCb();
};
