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

#ifndef CLOUDINT_H_
#define CLOUDINT_H_

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

class CloudInt : public ArduinoCloudProperty {
private:
  int _value,
      _cloud_shadow_value;  
public:
  CloudInt()                                          { CloudInt(0); }
  CloudInt(int v) : _value(v), _cloud_shadow_value(v) {}
  operator int() const {return _value;}
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
    cbor_encode_int  (mapEncoder, _value);
  }
  virtual void setValue(CborMapData const * const map_data) {
    _value = map_data->val.get();
  }
  virtual void setCloudShadowValue(CborMapData const * const map_data) {
    _cloud_shadow_value = map_data->val.get();
  }
  //modifiers
  CloudInt& operator=(int v) {
    _value = v;
    updateLocalTimestamp();
    return *this;
  }
  CloudInt& operator=(CloudInt v) {
    return operator=((int)v);
  }
  CloudInt& operator+=(int v) {return operator=(_value+=v);}
  CloudInt& operator-=(int v) {return operator=(_value-=v);}
  CloudInt& operator*=(int v) {return operator=(_value*=v);}
  CloudInt& operator/=(int v) {return operator=(_value/=v);}
  CloudInt& operator%=(int v) {return operator=(_value%=v);}
  CloudInt& operator++() {return operator=(++_value);}
  CloudInt& operator--() {return operator=(--_value);}
  CloudInt operator++(int) {return CloudInt(_value++);}
  CloudInt operator--(int) {return CloudInt(_value--);}
  CloudInt& operator&=(int v) {return operator=(_value&=v);}
  CloudInt& operator|=(int v) {return operator=(_value|=v);}
  CloudInt& operator^=(int v) {return operator=(_value^=v);}
  CloudInt& operator<<=(int v) {return operator=(_value<<=v);}
  CloudInt& operator>>=(int v) {return operator=(_value>>=v);}
  //accessors
  CloudInt operator+() const {return CloudInt(+_value);}
  CloudInt operator-() const {return CloudInt(-_value);}
  CloudInt operator!() const {return CloudInt(!_value);}
  CloudInt operator~() const {return CloudInt(~_value);}
  //friends
  friend CloudInt operator+(CloudInt iw, CloudInt v) {return iw+=v;}
  friend CloudInt operator+(CloudInt iw, int v) {return iw+=v;}
  friend CloudInt operator+(int v, CloudInt iw) {return CloudInt(v)+=iw;}
  friend CloudInt operator-(CloudInt iw, CloudInt v) {return iw-=v;}
  friend CloudInt operator-(CloudInt iw, int v) {return iw-=v;}
  friend CloudInt operator-(int v, CloudInt iw) {return CloudInt(v)-=iw;}
  friend CloudInt operator*(CloudInt iw, CloudInt v) {return iw*=v;}
  friend CloudInt operator*(CloudInt iw, int v) {return iw*=v;}
  friend CloudInt operator*(int v, CloudInt iw) {return CloudInt(v)*=iw;}
  friend CloudInt operator/(CloudInt iw, CloudInt v) {return iw/=v;}
  friend CloudInt operator/(CloudInt iw, int v) {return iw/=v;}
  friend CloudInt operator/(int v, CloudInt iw) {return CloudInt(v)/=iw;}
  friend CloudInt operator%(CloudInt iw, CloudInt v) {return iw%=v;}
  friend CloudInt operator%(CloudInt iw, int v) {return iw%=v;}
  friend CloudInt operator%(int v, CloudInt iw) {return CloudInt(v)%=iw;}
  friend CloudInt operator&(CloudInt iw, CloudInt v) {return iw&=v;}
  friend CloudInt operator&(CloudInt iw, int v) {return iw&=v;}
  friend CloudInt operator&(int v, CloudInt iw) {return CloudInt(v)&=iw;}
  friend CloudInt operator|(CloudInt iw, CloudInt v) {return iw|=v;}
  friend CloudInt operator|(CloudInt iw, int v) {return iw|=v;}
  friend CloudInt operator|(int v, CloudInt iw) {return CloudInt(v)|=iw;}
  friend CloudInt operator^(CloudInt iw, CloudInt v) {return iw^=v;}
  friend CloudInt operator^(CloudInt iw, int v) {return iw^=v;}
  friend CloudInt operator^(int v, CloudInt iw) {return CloudInt(v)^=iw;}
  friend CloudInt operator<<(CloudInt iw, CloudInt v) {return iw<<=v;}
  friend CloudInt operator<<(CloudInt iw, int v) {return iw<<=v;}
  friend CloudInt operator<<(int v, CloudInt iw) {return CloudInt(v)<<=iw;}
  friend CloudInt operator>>(CloudInt iw, CloudInt v) {return iw>>=v;}
  friend CloudInt operator>>(CloudInt iw, int v) {return iw>>=v;}
  friend CloudInt operator>>(int v, CloudInt iw) {return CloudInt(v)>>=iw;}

};


#endif /* CLOUDINT_H_ */
