/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <AIoTC_Config.h>

#ifdef HAS_TCP

#include "ArduinoIoTCloudDevice.h"
#include "interfaces/CloudProcess.h"

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/
ArduinoCloudDevice::ArduinoCloudDevice(MessageStream *ms)
: CloudProcess(ms),
_state{State::Init},
_attachAttempt(0, 0),
_attached(false),
_registered(false) {
}

void ArduinoCloudDevice::begin() {
  _attachAttempt.begin(AIOT_CONFIG_DEVICE_TOPIC_SUBSCRIBE_RETRY_DELAY_ms,
                       AIOT_CONFIG_MAX_DEVICE_TOPIC_SUBSCRIBE_RETRY_DELAY_ms);
}

void ArduinoCloudDevice::update() {
  /* Run through the state machine. */
  State nextState = _state;
  switch (_state) {
    case State::Init:             nextState = handleInit();             break;
    case State::SendCapabilities: nextState = handleSendCapabilities(); break;
    case State::Connected:        nextState = handleConnected();        break;
    case State::Disconnected:     nextState = handleDisconnected();     break;
  }

  /* Handle external events */
  switch (_command) {
    case DeviceAttachedCmdId:
      _attached = true;
      _registered = true;
      DEBUG_VERBOSE("CloudDevice::%s Device is attached", __FUNCTION__);
      nextState = State::Connected;
      break;

    case DeviceDetachedCmdId:
      _attached = false;
      _registered = false;
      nextState = State::Init;
      break;

    case DeviceRegisteredCmdId:
      _registered = true;
      nextState = State::Connected;
      break;

    /* We have received a reset command */
    case ResetCmdId:
      nextState = State::Init;
      break;

    default:
      break;
  }

  _command = UnknownCmdId;
  _state = nextState;
}

int ArduinoCloudDevice::connected() {
  return _state != State::Disconnected ? 1 : 0;
}

void ArduinoCloudDevice::handleMessage(Message *m) {
  _command = UnknownCmdId;
  if (m != nullptr) {
    _command = m->id;
  }
}

ArduinoCloudDevice::State ArduinoCloudDevice::handleInit() {
  /* Reset attempt struct for the nex retry after disconnection */
  _attachAttempt.begin(AIOT_CONFIG_DEVICE_TOPIC_SUBSCRIBE_RETRY_DELAY_ms,
                       AIOT_CONFIG_MAX_DEVICE_TOPIC_SUBSCRIBE_RETRY_DELAY_ms);

  _attached = false;
  _registered = false;

  return State::SendCapabilities;
}

ArduinoCloudDevice::State ArduinoCloudDevice::handleSendCapabilities() {
  /* Sends device capabilities message */
  DeviceBeginCmd deviceBegin = { DeviceBeginCmdId, AIOT_CONFIG_LIB_VERSION };
  deliver(reinterpret_cast<Message*>(&deviceBegin));

  /* Subscribe to device topic to request */
  ThingBeginCmd thingBegin = { ThingBeginCmdId };
  deliver(reinterpret_cast<Message*>(&thingBegin));

  /* No device configuration received. Wait: 4s -> 8s -> 16s -> 32s -> 32s ...*/
  _attachAttempt.retry();
  DEBUG_VERBOSE("CloudDevice::%s not attached. %d next configuration request in %d ms",
                __FUNCTION__, _attachAttempt.getRetryCount(), _attachAttempt.getWaitTime());
  return State::Connected;
}

ArduinoCloudDevice::State ArduinoCloudDevice::handleConnected() {
  /* Max retry than disconnect */
  if (_attachAttempt.getRetryCount() > AIOT_CONFIG_DEVICE_TOPIC_MAX_RETRY_CNT) {
    return State::Disconnected;
  }

  if (!_attached && _attachAttempt.isExpired()) {
    if (_registered) {
      /* Device configuration received, but invalid thing_id. Do not increase
       * counter, but recompute delay.
       * Wait: 4s -> 80s -> 160s -> 320s -> 640s -> 1280s -> 1280s ...
       */
      _attachAttempt.reconfigure(AIOT_CONFIG_DEVICE_TOPIC_ATTACH_RETRY_DELAY_ms,
                                 AIOT_CONFIG_MAX_DEVICE_TOPIC_ATTACH_RETRY_DELAY_ms);
    }
    return State::SendCapabilities;
  }

  return State::Connected;
}

ArduinoCloudDevice::State ArduinoCloudDevice::handleDisconnected() {
  return State::Disconnected;
}

#endif /* HAS_TCP */
