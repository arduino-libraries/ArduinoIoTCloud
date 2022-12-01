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

#ifndef CLOUDWRAPPERUINT_H_
#define CLOUDWRAPPERUINT_H_

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include <Arduino.h>
#include "CloudWrapperBase.h"

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class CloudWrapperUnsignedInt : public CloudWrapperBase {
  private:
    unsigned int  &_primitive_value,
         _cloud_value,
         _local_value;
  public:
    CloudWrapperUnsignedInt(unsigned int& v) : _primitive_value(v), _cloud_value(v), _local_value(v) {}
    virtual bool isDifferentFromCloud() {
      return _primitive_value != _cloud_value && ((std::max(_primitive_value , _cloud_value) - std::min(_primitive_value , _cloud_value)) >= Property::_min_delta_property);
    }
    virtual void fromCloudToLocal() {
      _primitive_value = _cloud_value;
    }
    virtual void fromLocalToCloud() {
      _cloud_value = _primitive_value;
    }
    virtual CborError appendAttributesToCloud(CborEncoder *encoder) {
      return appendAttribute(_primitive_value, "", encoder);
    }
    virtual void setAttributesFromCloud() {
      setAttribute(_cloud_value, "");
    }
    virtual bool isPrimitive() {
      return true;
    }
    virtual bool isChangedLocally() {
      return _primitive_value != _local_value;
    }
};


#endif /* CLOUDWRAPPERINT_H_ */
