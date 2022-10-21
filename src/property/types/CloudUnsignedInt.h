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

#ifndef CLOUDUINT_H_
#define CLOUDUINT_H_

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include <Arduino.h>
#include "../Property.h"

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/



class CloudUnsignedInt : public Property {
  private:
    unsigned int _value,
        _cloud_value;
  public:
    CloudUnsignedInt()                                          {
      CloudUnsignedInt(0);
    }
    CloudUnsignedInt(unsigned int v) : _value(v), _cloud_value(v) {}
    operator unsigned int() const {
      return _value;
    }
    virtual bool isDifferentFromCloud() {
      return _value != _cloud_value && ((std::max(_value , _cloud_value) - std::min(_value , _cloud_value)) >= Property::_min_delta_property);
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
    CloudUnsignedInt& operator=(unsigned int v) {
      _value = v;
      updateLocalTimestamp();
      return *this;
    }
    CloudUnsignedInt& operator=(CloudUnsignedInt v) {
      return operator=((unsigned int)v);
    }
    CloudUnsignedInt& operator+=(unsigned int v) {
      return operator=(_value += v);
    }
    CloudUnsignedInt& operator-=(unsigned int v) {
      return operator=(_value -= v);
    }
    CloudUnsignedInt& operator*=(unsigned int v) {
      return operator=(_value *= v);
    }
    CloudUnsignedInt& operator/=(unsigned int v) {
      return operator=(_value /= v);
    }
    CloudUnsignedInt& operator%=(unsigned int v) {
      return operator=(_value %= v);
    }
    CloudUnsignedInt& operator++() {
      return operator=(++_value);
    }
    CloudUnsignedInt& operator--() {
      return operator=(--_value);
    }
    CloudUnsignedInt operator++(int) {
      operator=(_value + 1);
      return CloudUnsignedInt(_value);
    }
    CloudUnsignedInt operator--(int) {
      operator=(_value - 1);
      return CloudUnsignedInt(_value);
    }
    CloudUnsignedInt& operator&=(unsigned int v) {
      return operator=(_value &= v);
    }
    CloudUnsignedInt& operator|=(unsigned int v) {
      return operator=(_value |= v);
    }
    CloudUnsignedInt& operator^=(unsigned int v) {
      return operator=(_value ^= v);
    }
    CloudUnsignedInt& operator<<=(unsigned int v) {
      return operator=(_value <<= v);
    }
    CloudUnsignedInt& operator>>=(unsigned int v) {
      return operator=(_value >>= v);
    }
    //accessors
    CloudUnsignedInt operator+() const {
      return CloudUnsignedInt(+_value);
    }
    CloudUnsignedInt operator-() const {
      return CloudUnsignedInt(-_value);
    }
    CloudUnsignedInt operator!() const {
      return CloudUnsignedInt(!_value);
    }
    CloudUnsignedInt operator~() const {
      return CloudUnsignedInt(~_value);
    }
    //friends
    friend CloudUnsignedInt operator+(CloudUnsignedInt iw, CloudUnsignedInt v) {
      return iw += v;
    }
    friend CloudUnsignedInt operator+(CloudUnsignedInt iw, unsigned int v) {
      return iw += v;
    }
    friend CloudUnsignedInt operator+(unsigned int v, CloudUnsignedInt iw) {
      return CloudUnsignedInt(v) += iw;
    }
    friend CloudUnsignedInt operator-(CloudUnsignedInt iw, CloudUnsignedInt v) {
      return iw -= v;
    }
    friend CloudUnsignedInt operator-(CloudUnsignedInt iw, unsigned int v) {
      return iw -= v;
    }
    friend CloudUnsignedInt operator-(unsigned int v, CloudUnsignedInt iw) {
      return CloudUnsignedInt(v) -= iw;
    }
    friend CloudUnsignedInt operator*(CloudUnsignedInt iw, CloudUnsignedInt v) {
      return iw *= v;
    }
    friend CloudUnsignedInt operator*(CloudUnsignedInt iw, unsigned int v) {
      return iw *= v;
    }
    friend CloudUnsignedInt operator*(unsigned int v, CloudUnsignedInt iw) {
      return CloudUnsignedInt(v) *= iw;
    }
    friend CloudUnsignedInt operator/(CloudUnsignedInt iw, CloudUnsignedInt v) {
      return iw /= v;
    }
    friend CloudUnsignedInt operator/(CloudUnsignedInt iw, unsigned int v) {
      return iw /= v;
    }
    friend CloudUnsignedInt operator/(unsigned int v, CloudUnsignedInt iw) {
      return CloudUnsignedInt(v) /= iw;
    }
    friend CloudUnsignedInt operator%(CloudUnsignedInt iw, CloudUnsignedInt v) {
      return iw %= v;
    }
    friend CloudUnsignedInt operator%(CloudUnsignedInt iw, unsigned int v) {
      return iw %= v;
    }
    friend CloudUnsignedInt operator%(unsigned int v, CloudUnsignedInt iw) {
      return CloudUnsignedInt(v) %= iw;
    }
    friend CloudUnsignedInt operator&(CloudUnsignedInt iw, CloudUnsignedInt v) {
      return iw &= v;
    }
    friend CloudUnsignedInt operator&(CloudUnsignedInt iw, unsigned int v) {
      return iw &= v;
    }
    friend CloudUnsignedInt operator&(unsigned int v, CloudUnsignedInt iw) {
      return CloudUnsignedInt(v) &= iw;
    }
    friend CloudUnsignedInt operator|(CloudUnsignedInt iw, CloudUnsignedInt v) {
      return iw |= v;
    }
    friend CloudUnsignedInt operator|(CloudUnsignedInt iw, unsigned int v) {
      return iw |= v;
    }
    friend CloudUnsignedInt operator|(unsigned int v, CloudUnsignedInt iw) {
      return CloudUnsignedInt(v) |= iw;
    }
    friend CloudUnsignedInt operator^(CloudUnsignedInt iw, CloudUnsignedInt v) {
      return iw ^= v;
    }
    friend CloudUnsignedInt operator^(CloudUnsignedInt iw, unsigned int v) {
      return iw ^= v;
    }
    friend CloudUnsignedInt operator^(unsigned int v, CloudUnsignedInt iw) {
      return CloudUnsignedInt(v) ^= iw;
    }
    friend CloudUnsignedInt operator<<(CloudUnsignedInt iw, CloudUnsignedInt v) {
      return iw <<= v;
    }
    friend CloudUnsignedInt operator<<(CloudUnsignedInt iw, unsigned int v) {
      return iw <<= v;
    }
    friend CloudUnsignedInt operator<<(unsigned int v, CloudUnsignedInt iw) {
      return CloudUnsignedInt(v) <<= iw;
    }
    friend CloudUnsignedInt operator>>(CloudUnsignedInt iw, CloudUnsignedInt v) {
      return iw >>= v;
    }
    friend CloudUnsignedInt operator>>(CloudUnsignedInt iw, unsigned int v) {
      return iw >>= v;
    }
    friend CloudUnsignedInt operator>>(unsigned int v, CloudUnsignedInt iw) {
      return CloudUnsignedInt(v) >>= iw;
    }

};


#endif /* CLOUDUINT_H_ */
