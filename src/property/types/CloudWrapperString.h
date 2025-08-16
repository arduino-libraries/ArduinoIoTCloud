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

#ifndef CLOUDWRAPPERSTRING_H_
#define CLOUDWRAPPERSTRING_H_

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include <Arduino.h>
#include "CloudWrapperBase.h"

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class CloudWrapperString : public CloudWrapperBase {
  private:
    String  &_primitive_value,
            _cloud_value,
            _local_value;
  public:
    CloudWrapperString(String& v) :
      _primitive_value(v),
      _cloud_value(v),
      _local_value(v) {
    }
    bool isDifferentFromCloud() override {
      return _primitive_value != _cloud_value;
    }
    void fromCloudToLocal() override {
      _primitive_value = _cloud_value;
    }
    void fromLocalToCloud() override {
      _cloud_value = _primitive_value;
    }
    CborError appendAttributesToCloud(CborEncoder *encoder) override {
      return appendAttribute(_primitive_value, "", encoder);
    }
    void setAttributesFromCloud() override {
      setAttribute(_cloud_value, "");
    }
    bool isPrimitive() override {
      return true;
    }
    bool isChangedLocally() override {
      return _primitive_value != _local_value;
    }
};


#endif /* CLOUDWRAPPERSTRING_H_ */
