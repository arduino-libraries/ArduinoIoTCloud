/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2019 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef CLOUDBOOL_H_
#define CLOUDBOOL_H_

/******************************************************************************
  INCLUDE
 ******************************************************************************/

#include <Arduino.h>
#include "../Property.h"

/******************************************************************************
  CLASS DECLARATION
 ******************************************************************************/

class CloudBool : public Property {
  protected:
    bool  _value,
          _cloud_value;
  public:
    CloudBool() {
      CloudBool(false);
    }
    CloudBool(bool v) : _value(v), _cloud_value(v) {}
    operator bool() const {
      return _value;
    }
    virtual bool isDifferentFromCloud() {
      return _value != _cloud_value;
    }
    virtual void fromCloudToLocal() {
      _value = _cloud_value;
    }
    virtual void fromLocalToCloud() {
      _cloud_value = _value;
    }
    virtual CborError appendAttributesToCloud(CborEncoder *encoder) {
      return appendAttribute(_value, "", encoder);
    }
    virtual void setAttributesFromCloud() {
      setAttribute(_cloud_value, "");
    }
    //modifiers
    CloudBool& operator=(bool v) {
      _value = v;
      updateLocalTimestamp();
      return *this;
    }
    CloudBool& operator=(CloudBool v) {
      return operator=((bool)v);
    }
    //accessors
    CloudBool operator!() const {
      return CloudBool(!_value);
    }
    //friends
};

#endif /* CLOUDBOOL_H_ */
