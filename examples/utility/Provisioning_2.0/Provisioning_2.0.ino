/*
  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "thingProperties.h"
#include "CSRHandler.h"
#include "ClaimingHandler.h"
#include "SecretsHelper.h"
#include <Arduino_FlashFormatter.h>
#include <Arduino_SecureElement.h>
#include <utility/SElementArduinoCloudDeviceId.h>
#include <utility/SElementArduinoCloudCertificate.h>
#include "utility/LEDFeedback.h"
#include "FactoryTester.h"
#if defined(ARDUINO_OPTA)
#include "OptaFactoryTest.h"
#endif

const char *SKETCH_VERSION = "0.6.2";

enum class DeviceState {
  #if defined(ARDUINO_OPTA)
    OPTA_TEST,
  #endif
    HARDWARE_CHECK,
    BEGIN,
    NETWORK_CONFIG,
    CSR,
    BEGIN_CLOUD,
    RUN,
    ERROR
  };

DeviceState _state = DeviceState::HARDWARE_CHECK;
SecureElement secureElement;

String uhwid = "";
bool resetEvent = false;

CSRHandlerClass CSRHandler;
ClaimingHandlerClass ClaimingHandler;

bool clearStoredCredentials() {
  const uint8_t empty[4] = {0x00,0x00,0x00,0x00};
  if(!NetworkConfigurator.resetStoredConfiguration() || \
    !secureElement.writeSlot(static_cast<int>(SElementArduinoCloudSlot::DeviceId), (byte*)empty, sizeof(empty)) || \
    !secureElement.writeSlot(static_cast<int>(SElementArduinoCloudSlot::CompressedCertificate), (byte*)empty, sizeof(empty))) {
    return false;
  }

  ArduinoCloud.disconnect();
  resetEvent = true;
  return true;
  }

void setup() {
  Serial.begin(9600);

  delay(1500);

  setDebugMessageLevel(4);

  initProperties();

  #if !defined(ARDUINO_OPTA) && !defined(ARDUINO_UNOR4_WIFI)
  LedFactoryTest();
  #endif

  #if defined(ARDUINO_OPTA)
  OptaFactoryTest.begin();
  OptaFactoryTest.optaIDTest();
  uint32_t start = millis();
  while(millis() - start < 500){
    if(OptaFactoryTest.poll() == true){
      pinMode(LEDG, OUTPUT);
      ResetInput::getInstance().begin();
       _state = DeviceState::OPTA_TEST;
      return;
    }
  }
  if(_getPid_() != OPTA_WIFI_PID) {
    //Force blinking green LED if not OPTA WiFi
    LEDFeedbackClass::getInstance().setMode(LEDFeedbackClass::LEDFeedbackMode::BLE_AVAILABLE);
  }
  #endif

  AgentsManagerClass::getInstance().begin();
  LEDFeedbackClass::getInstance().begin();
  DEBUG_INFO("Starting Provisioning version %s", SKETCH_VERSION);
}

void sendStatus(StatusMessage msg) {
  ProvisioningOutputMessage outMsg = { MessageOutputType::STATUS, { msg } };
  AgentsManagerClass::getInstance().sendMsg(outMsg);
}

#if defined(ARDUINO_OPTA)
uint32_t lastLedStatusChanged = 0;
bool ledactive = false;
DeviceState handleOptaTest() {
  bool running = OptaFactoryTest.poll();
  if(!running){
    digitalWrite(LEDG, LOW);
    digitalWrite(LEDR, HIGH);
  } else {
    if(millis() - lastLedStatusChanged > 250){
      lastLedStatusChanged = millis();
      ledactive = !ledactive;
      digitalWrite(LEDG, ledactive);
    }
  }

  return DeviceState::OPTA_TEST;
}
#endif

DeviceState handleHardwareCheck() {
  // Init the secure element
  if(!secureElement.begin()) {
    DEBUG_ERROR("Sketch: Error during secureElement begin!");
    LEDFeedbackClass::getInstance().setMode(LEDFeedbackClass::LEDFeedbackMode::ERROR);
    sendStatus(StatusMessage::HW_ERROR_SE_BEGIN);
    return DeviceState::ERROR;
  }

  if (!secureElement.locked()) {
    if (!secureElement.writeConfiguration()) {
      DEBUG_ERROR("Sketch: Writing secureElement configuration failed!");
      LEDFeedbackClass::getInstance().setMode(LEDFeedbackClass::LEDFeedbackMode::ERROR);
      sendStatus(StatusMessage::HW_ERROR_SE_CONFIG);
      return DeviceState::ERROR;
    }

    if (!secureElement.lock()) {
      DEBUG_ERROR("Sketch: Locking secureElement configuration failed!");
      LEDFeedbackClass::getInstance().setMode(LEDFeedbackClass::LEDFeedbackMode::ERROR);
      sendStatus(StatusMessage::HW_ERROR_SE_LOCK);
      return DeviceState::ERROR;
    }
    DEBUG_INFO("secureElement locked successfully");
  }

  FlashFormatter flashFormatter;
  // Check if the board storage is properly formatted
  if(!flashFormatter.checkAndFormatPartition()) {
    DEBUG_ERROR("Sketch: Error partitioning storage");
    LEDFeedbackClass::getInstance().setMode(LEDFeedbackClass::LEDFeedbackMode::ERROR);
    sendStatus(StatusMessage::FAIL_TO_PARTITION_STORAGE);
    return DeviceState::ERROR;
  }

  return DeviceState::BEGIN;
}

DeviceState handleBegin() {
  uhwid = GetUHWID();
  if(uhwid == ""){
    DEBUG_ERROR("Sketch: Error getting UHWID");
    LEDFeedbackClass::getInstance().setMode(LEDFeedbackClass::LEDFeedbackMode::ERROR);
    sendStatus(StatusMessage::ERROR_GENERATING_UHWID);
    return DeviceState::ERROR;
  }
  // Scan the network options
  NetworkConfigurator.scanNetworkOptions();
  NetworkConfigurator.begin();
  ClaimingHandler.begin(secureElement, uhwid, clearStoredCredentials);
  DEBUG_INFO("BLE Available");
  return DeviceState::NETWORK_CONFIG;
}

DeviceState handleNetworkConfig() {
  ClaimingHandler.poll();
  if(resetEvent){
    resetEvent = false;
  }
  NetworkConfiguratorStates s = NetworkConfigurator.update();

  DeviceState nextState = _state;
  if (s == NetworkConfiguratorStates::CONFIGURED) {
    String deviceId = "";
    SElementArduinoCloudDeviceId::read(secureElement, deviceId, SElementArduinoCloudSlot::DeviceId);

    if (deviceId == "") {
      CSRHandler.begin(ArduinoIoTPreferredConnection, secureElement, uhwid);
      nextState = DeviceState::CSR;
    } else {
      nextState = DeviceState::BEGIN_CLOUD;
    }
  }
  return nextState;
}

DeviceState handleCSR() {
  NetworkConfigurator.update();
  ClaimingHandler.poll();
  if(resetEvent) {
    resetEvent = false;
    CSRHandler.end();
    return DeviceState::NETWORK_CONFIG;
  }

  DeviceState nextState = _state;

  CSRHandlerClass::CSRHandlerStates res = CSRHandler.poll();
  if (res == CSRHandlerClass::CSRHandlerStates::COMPLETED) {
    CSRHandler.end();
    nextState = DeviceState::BEGIN_CLOUD;
  }

  return nextState;
}

DeviceState handleBeginCloud() {
  // Close the connection to the peer (App mobile, FE, etc)
  NetworkConfigurator.disconnectAgent();
  // Close the BLE connectivity
  if (NetworkConfigurator.isAgentEnabled(ConfiguratorAgent::AgentTypes::BLE)) {
    NetworkConfigurator.enableAgent(ConfiguratorAgent::AgentTypes::BLE, false);
  }
  // Connect to Arduino IoT Cloud
#ifdef COMPILE_TEST
  ArduinoCloud.begin(ArduinoIoTPreferredConnection, false, "mqtts-sa.iot.oniudra.cc");
#else
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
#endif
  ArduinoCloud.printDebugInfo();

  return DeviceState::RUN;
}

void cloudConnectedHandler(bool connected) {
  static bool _status = false;
  if(connected != _status){
    _status = connected;
    if(connected){
      LEDFeedbackClass::getInstance().setMode(LEDFeedbackClass::LEDFeedbackMode::CONNECTED_TO_CLOUD);
    } else {
      LEDFeedbackClass::getInstance().setMode(LEDFeedbackClass::LEDFeedbackMode::NONE);
    }
  }
}

DeviceState handleRun() {
  ClaimingHandler.poll();
  if(resetEvent) {
    resetEvent = false;
    return DeviceState::NETWORK_CONFIG;
  }

  DeviceState nextState = _state;
  ArduinoCloud.update();

  cloudConnectedHandler(ArduinoCloud.connected());

  return nextState;
}

DeviceState handleError() {
  LEDFeedbackClass::getInstance().update();
  AgentsManagerClass::getInstance().update();
  return DeviceState::ERROR;
}

void loop() {
  switch (_state) {
    #if defined(ARDUINO_OPTA)
    case DeviceState::OPTA_TEST:        _state = handleOptaTest     (); break;
    #endif
    case DeviceState::HARDWARE_CHECK:  _state = handleHardwareCheck(); break;
    case DeviceState::BEGIN:           _state = handleBegin        (); break;
    case DeviceState::NETWORK_CONFIG : _state = handleNetworkConfig(); break;
    case DeviceState::CSR:             _state = handleCSR          (); break;
    case DeviceState::BEGIN_CLOUD:     _state = handleBeginCloud   (); break;
    case DeviceState::RUN:             _state = handleRun          (); break;
    case DeviceState::ERROR:           _state = handleError        (); break;
    default:                                                           break;
  }

  #if defined(ARDUINO_OPTA)
  if(_state == DeviceState::NETWORK_CONFIG || _state == DeviceState::RUN){
    OptaFactoryTest.poll();
  }
  #endif

}
