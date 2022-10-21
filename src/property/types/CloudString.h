//
// This file is part of ArduinoCloudThing
//
// Copyright 2019 ARDUINO SA (http://www.arduino.cc/)
//
// This software is released under the GNU General Public License version 3,
// which covers the main part of ArduinoCloudThing.
// The terms of this license can be found at:
// https://www.gnu.org/licenses/gpl-3.0.en.html
//
// You can be released from the requirements of the above licenses by purchasing
// a commercial license. Buying such a license is mandatory if you want to modify or
// otherwise use the software for commercial activities involving the Arduino
// software without disclosing the source code of your own applications. To purchase
// a commercial license, send an email to license@arduino.cc.
//

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
    CloudString()                                {
      CloudString("");
    }
    CloudString(const char *v)                   {
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
