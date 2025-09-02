/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef ARDUINO_IOT_CLOUD_DEVICE_H
#define ARDUINO_IOT_CLOUD_DEVICE_H

/******************************************************************************
  INCLUDE
 ******************************************************************************/

#include <Arduino_TimedAttempt.h>
#include "interfaces/CloudProcess.h"
#include "property/PropertyContainer.h"
#include <Arduino_ConnectionHandler.h>
#include <connectionHandlerModels/settings.h>

/******************************************************************************
  CLASS DECLARATION
 ******************************************************************************/

class ArduinoCloudDevice : public CloudProcess {
public:

  typedef std::function<void(models::NetworkSetting&)> GetNetworkSettingCbk;

  ArduinoCloudDevice(MessageStream* stream);
  virtual void update() override;
  virtual void handleMessage(Message* m) override;

  virtual void begin();
  virtual int connected();

  inline PropertyContainer &getPropertyContainer() {
    return _propertyContainer;
  };
  inline unsigned int &getPropertyContainerIndex() {
    return _propertyContainerIndex;
  }
  inline bool isAttached() {
    return _attached;
  };

  void setGetNetworkSettingCbk(GetNetworkSettingCbk cbk) {
    _getNetConfigCallback = cbk;
  }


private:

  enum class State {
    Disconnected,
    Init,
    SendCapabilities,
    Connected,
  };

  State _state;
  CommandId _command;
  TimedAttempt _attachAttempt;
  PropertyContainer _propertyContainer;
  GetNetworkSettingCbk _getNetConfigCallback;
  unsigned int _propertyContainerIndex;
  bool _attached;
  bool _registered;

  State handleInit();
  State handleSendCapabilities();
  State handleConnected();
  State handleDisconnected();
};

#endif /* ARDUINO_IOT_CLOUD_DEVICE_H */
