/*
  This file is part of the Arduino_SecureElement library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef ARDUINO_IOT_CLOUD_DEVICE_H
#define ARDUINO_IOT_CLOUD_DEVICE_H

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include "interfaces/CloudProcess.h"
#include "utility/time/TimedAttempt.h"
#include "property/PropertyContainer.h"

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class ArduinoCloudDevice : public CloudProcess {
public:

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
  unsigned int _propertyContainerIndex;
  bool _attached;
  bool _registered;

  State handleInit();
  State handleSendCapabilities();
  State handleConnected();
  State handleDisconnected();
};

#endif /* ARDUINO_IOT_CLOUD_DEVICE_H */
