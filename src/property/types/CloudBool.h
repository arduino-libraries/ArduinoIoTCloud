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
    CloudBool()                                           {
      CloudBool(false);
    }
    CloudBool(bool v) : _value(v), _cloud_value(v) {}
    operator bool() const                             {
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
