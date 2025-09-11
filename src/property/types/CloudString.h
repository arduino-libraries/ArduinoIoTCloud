/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2019 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef CLOUDSTRING_H_
#define CLOUDSTRING_H_

/******************************************************************************
  INCLUDE
 ******************************************************************************/

#include <Arduino.h>
#include "../Property.h"

/******************************************************************************
  CLASS DECLARATION
 ******************************************************************************/

class CloudString : public Property {
  private:
    String  _value,
            _cloud_value;
  public:
    CloudString() {
      CloudString("");
    }
    CloudString(const char *v) {
      CloudString(String(v));
    }
    CloudString(String v) : _value(v), _cloud_value(v) {}
    operator String() const {
      return _value;
    }
    void clear() {
      _value = PropertyActions::CLEAR;
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
    CloudString& operator=(String v) {
      _value = v;
      updateLocalTimestamp();
      return *this;
    }
    CloudString& operator=(const char *v) {
      return operator=(String(v));
    }
    CloudString& operator+=(String v) {
      return operator=(_value += v);
    }
    bool operator==(const char *c) const {
      return operator==(String(c));
    }
    bool operator==(String c) const {
      return _value == c;
    }
    //friends
    friend CloudString operator+(CloudString cs, String v) {
      return cs += v;
    }
};

#endif /* CLOUDSTRING_H_ */
