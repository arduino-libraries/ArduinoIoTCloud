/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2019 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

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
