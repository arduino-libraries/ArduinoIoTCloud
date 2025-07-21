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

#ifndef CLOUDWRAPPERBASE_H_
#define CLOUDWRAPPERBASE_H_

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include <Arduino.h>
#include "PropertyPrimitive.h"

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

// class CloudWrapperBaseInterface {
//   public:
//     virtual bool isChangedLocally() = 0;
// };

class CloudWrapperBase : public Property {
  public:
    virtual bool isChangedLocally() = 0;
};

template<typename T>
class CloudWrapperProperty : public CloudWrapperBase {
public:
    CloudWrapperProperty(T& value)
    : PropertyPrimitive<T>(value), _primitive_value(value) { }

    bool isDifferentFromCloud() override {
      return _primitive_value != PropertyPrimitive<T>::_cloud_value;
    }

    void fromCloudToLocal() override {
      _primitive_value = PropertyPrimitive<T>::_cloud_value;
    }
    void fromLocalToCloud() override {
      PropertyPrimitive<T>::_cloud_value = _primitive_value;
    }

    CborError appendAttributesToCloud(CborEncoder *encoder) override {
      return PropertyPrimitive<T>::appendAttribute(_primitive_value, "", encoder);
    }
    void setAttributesFromCloud() override {
      PropertyPrimitive<T>::setAttribute(PropertyPrimitive<T>::_cloud_value, "");
    }

    bool isChangedLocally() override {
      return _primitive_value != PropertyPrimitive<T>::_value;
    }
protected:
    T &_primitive_value;
};


#endif /* CLOUDWRAPPERBASE_H_ */
