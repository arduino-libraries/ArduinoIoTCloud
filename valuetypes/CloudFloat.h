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

#ifndef CLOUDFLOAT_H_
#define CLOUDFLOAT_H_

#include <math.h>

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

class CloudFloat : public ArduinoCloudProperty {
private:
  float _value,
        _cloud_shadow_value;
public:
  CloudFloat()                                            { CloudFloat(0.0f); }
  CloudFloat(float v) : _value(v), _cloud_shadow_value(v) {}
  operator float() const {return _value;}
  virtual bool isDifferentFromCloudShadow() {
    return _value != _cloud_shadow_value && (abs(_value - _cloud_shadow_value) >= ArduinoCloudProperty::_min_delta_property);
  }
  virtual void toShadow() {
    _cloud_shadow_value = _value;
  }
  virtual void fromCloudShadow() {
    _value = _cloud_shadow_value;
  }
  virtual void appendValue(CborEncoder * mapEncoder) const {
    cbor_encode_int  (mapEncoder, static_cast<int>(CborIntegerMapKey::Value));    
    cbor_encode_float(mapEncoder, _value);
  }
  virtual void setValue(CborMapData const * const map_data) {
    _value = map_data->val.get();
  }
  virtual void setCloudShadowValue(CborMapData const * const map_data) {
    _cloud_shadow_value = map_data->val.get();
  }
  //modifiers
  CloudFloat& operator=(float v) {
    _value = v;
    updateLocalTimestamp();
    return *this;
  }
  CloudFloat& operator=(CloudFloat v) {
    return operator=((float)v);
  }
  CloudFloat& operator+=(float v) {return operator=(_value+=v);}
  CloudFloat& operator-=(float v) {return operator=(_value-=v);}
  CloudFloat& operator*=(float v) {return operator=(_value*=v);}
  CloudFloat& operator/=(float v) {return operator=(_value/=v);}
  CloudFloat& operator++() {return operator=(_value++);}
  CloudFloat& operator--() {return operator=(_value--);}
  //friends
  friend CloudFloat operator+(CloudFloat iw, CloudFloat v) {return iw+=v;}
  friend CloudFloat operator+(CloudFloat iw, float v) {return iw+=v;}
  friend CloudFloat operator+(float v, CloudFloat iw) {return CloudFloat(v)+=iw;}
  friend CloudFloat operator-(CloudFloat iw, CloudFloat v) {return iw-=v;}
  friend CloudFloat operator-(CloudFloat iw, float v) {return iw-=v;}
  friend CloudFloat operator-(float v, CloudFloat iw) {return CloudFloat(v)-=iw;}
  friend CloudFloat operator*(CloudFloat iw, CloudFloat v) {return iw*=v;}
  friend CloudFloat operator*(CloudFloat iw, float v) {return iw*=v;}
  friend CloudFloat operator*(float v, CloudFloat iw) {return CloudFloat(v)*=iw;}
  friend CloudFloat operator/(CloudFloat iw, CloudFloat v) {return iw/=v;}
  friend CloudFloat operator/(CloudFloat iw, float v) {return iw/=v;}
  friend CloudFloat operator/(float v, CloudFloat iw) {return CloudFloat(v)/=iw;}
};


#endif /* CLOUDFLOAT_H_ */
