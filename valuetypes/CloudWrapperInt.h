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

#ifndef CLOUDWRAPPERINT_H_
#define CLOUDWRAPPERINT_H_

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <Arduino.h>
#include "../ArduinoCloudProperty.hpp"

/******************************************************************************
 * TYPEDEF
 ******************************************************************************/

/******************************************************************************
 * TYPEDEF
 ******************************************************************************/

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class CloudWrapperInt : public ArduinoCloudProperty {
private:
  int  &_primitive_value,
        _cloud_shadow_value,
        _local_shadow_value;  
public:
  CloudWrapperInt(int& v) : _primitive_value(v), _cloud_shadow_value(v), _local_shadow_value(v) {}
  virtual bool isDifferentFromCloudShadow() {
    return _primitive_value != _cloud_shadow_value && (abs(_primitive_value - _cloud_shadow_value) >= ArduinoCloudProperty::_min_delta_property);
  }
  virtual void toShadow() {
    _cloud_shadow_value = _local_shadow_value = _primitive_value;
  }
  virtual void fromCloudShadow() {
    _primitive_value = _cloud_shadow_value;
  }
  virtual void appendValue(CborEncoder * mapEncoder) const {
    cbor_encode_int  (mapEncoder, static_cast<int>(CborIntegerMapKey::Value));    
    cbor_encode_int  (mapEncoder, _primitive_value);
  }
  virtual void setValue(CborMapData const * const map_data) {
    _primitive_value = map_data->val.get();
  }
  virtual void setCloudShadowValue(CborMapData const * const map_data) {
    _cloud_shadow_value = map_data->val.get();
  }
  virtual bool isPrimitive() {
    return true;
  }
  virtual bool isChangedLocally() {
    return _primitive_value != _local_shadow_value;
  }
};


#endif /* CLOUDWRAPPERINT_H_ */
