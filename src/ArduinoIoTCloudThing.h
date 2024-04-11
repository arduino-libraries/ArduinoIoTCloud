/*
  This file is part of the Arduino_SecureElement library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/


#ifndef ARDUINO_IOT_CLOUD_THING_H
#define ARDUINO_IOT_CLOUD_THING_H

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include "interfaces/CloudProcess.h"
#include "utility/time/TimedAttempt.h"
#include "property/PropertyContainer.h"

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class ArduinoCloudThing : public CloudProcess {
public:

  ArduinoCloudThing(MessageStream *stream);
  virtual void update() override;
  virtual void handleMessage(Message *m) override;

  virtual void begin();
  virtual int connected();

  inline PropertyContainer &getPropertyContainer() {
    return _propertyContainer;
  };
  inline unsigned int &getPropertyContainerIndex() {
    return _propertyContainerIndex;
  }

private:

  enum class State {
    Disconnect,
    Init,
    RequestLastValues,
    Connected,
  };

  State _state;
  CommandId _command;
  TimedAttempt _syncAttempt;
  PropertyContainer _propertyContainer;
  unsigned int _propertyContainerIndex;
  int _utcOffset;
  Property *_utcOffsetProperty;
  unsigned int _utcOffsetExpireTime;
  Property *_utcOffsetExpireTimeProperty;

  State handleInit();
  State handleRequestLastValues();
  State handleConnected();
  State handleDisconnect();
};

#endif /* ARDUINO_IOT_CLOUD_THING_H */
