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
   INCLUDE
 ******************************************************************************/

#include <stdint.h>

#include <Arduino.h>
#include "../Property.h"

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

template <typename T>
class CloudInt : public Property {
  private:
    T _value,
      _cloud_value;
  public:
    CloudInt() {
      CloudInt(0);
    }
    CloudInt(T v) : _value(v), _cloud_value(v) {}
    CloudInt(const CloudInt& r) : _value(r._value), _cloud_value(r._cloud_value) {}
    operator T() const {
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
    CloudInt& operator=(T v) {
      _value = v;
      updateLocalTimestamp();
      return *this;
    }
    CloudInt& operator=(CloudInt v) {
      return operator=(static_cast<T>(v));
    }
    CloudInt& operator+=(T v) {
      return operator=(_value += v);
    }
    CloudInt& operator-=(T v) {
      return operator=(_value -= v);
    }
    CloudInt& operator*=(T v) {
      return operator=(_value *= v);
    }
    CloudInt& operator/=(T v) {
      return operator=(_value /= v);
    }
    CloudInt& operator%=(T v) {
      return operator=(_value %= v);
    }
    CloudInt& operator++() {
      return operator=(++_value);
    }
    CloudInt& operator--() {
      return operator=(--_value);
    }
    CloudInt operator++(int) {
      operator=(_value + 1);
      return CloudInt(_value);
    }
    CloudInt operator--(int) {
      operator=(_value - 1);
      return CloudInt(_value);
    }
    CloudInt& operator&=(T v) {
      return operator=(_value &= v);
    }
    CloudInt& operator|=(T v) {
      return operator=(_value |= v);
    }
    CloudInt& operator^=(T v) {
      return operator=(_value ^= v);
    }
    CloudInt& operator<<=(T v) {
      return operator=(_value <<= v);
    }
    CloudInt& operator>>=(T v) {
      return operator=(_value >>= v);
    }
    //accessors
    CloudInt operator+() const {
      return CloudInt(+_value);
    }
    CloudInt operator-() const {
      return CloudInt(-_value);
    }
    CloudInt operator!() const {
      return CloudInt(!_value);
    }
    CloudInt operator~() const {
      return CloudInt(~_value);
    }
    //friends
    friend CloudInt operator+(CloudInt iw, CloudInt v) {
      return iw += v;
    }
    friend CloudInt operator+(CloudInt iw, T v) {
      return iw += v;
    }
    friend CloudInt operator+(T v, CloudInt iw) {
      return CloudInt(v) += iw;
    }
    friend CloudInt operator-(CloudInt iw, CloudInt v) {
      return iw -= v;
    }
    friend CloudInt operator-(CloudInt iw, T v) {
      return iw -= v;
    }
    friend CloudInt operator-(T v, CloudInt iw) {
      return CloudInt(v) -= iw;
    }
    friend CloudInt operator*(CloudInt iw, CloudInt v) {
      return iw *= v;
    }
    friend CloudInt operator*(CloudInt iw, T v) {
      return iw *= v;
    }
    friend CloudInt operator*(T v, CloudInt iw) {
      return CloudInt(v) *= iw;
    }
    friend CloudInt operator/(CloudInt iw, CloudInt v) {
      return iw /= v;
    }
    friend CloudInt operator/(CloudInt iw, T v) {
      return iw /= v;
    }
    friend CloudInt operator/(T v, CloudInt iw) {
      return CloudInt(v) /= iw;
    }
    friend CloudInt operator%(CloudInt iw, CloudInt v) {
      return iw %= v;
    }
    friend CloudInt operator%(CloudInt iw, T v) {
      return iw %= v;
    }
    friend CloudInt operator%(T v, CloudInt iw) {
      return CloudInt(v) %= iw;
    }
    friend CloudInt operator&(CloudInt iw, CloudInt v) {
      return iw &= v;
    }
    friend CloudInt operator&(CloudInt iw, T v) {
      return iw &= v;
    }
    friend CloudInt operator&(T v, CloudInt iw) {
      return CloudInt(v) &= iw;
    }
    friend CloudInt operator|(CloudInt iw, CloudInt v) {
      return iw |= v;
    }
    friend CloudInt operator|(CloudInt iw, T v) {
      return iw |= v;
    }
    friend CloudInt operator|(T v, CloudInt iw) {
      return CloudInt(v) |= iw;
    }
    friend CloudInt operator^(CloudInt iw, CloudInt v) {
      return iw ^= v;
    }
    friend CloudInt operator^(CloudInt iw, T v) {
      return iw ^= v;
    }
    friend CloudInt operator^(T v, CloudInt iw) {
      return CloudInt(v) ^= iw;
    }
    friend CloudInt operator<<(CloudInt iw, CloudInt v) {
      return iw <<= v;
    }
    friend CloudInt operator<<(CloudInt iw, T v) {
      return iw <<= v;
    }
    friend CloudInt operator<<(T v, CloudInt iw) {
      return CloudInt(v) <<= iw;
    }
    friend CloudInt operator>>(CloudInt iw, CloudInt v) {
      return iw >>= v;
    }
    friend CloudInt operator>>(CloudInt iw, T v) {
      return iw >>= v;
    }
    friend CloudInt operator>>(T v, CloudInt iw) {
      return CloudInt(v) >>= iw;
    }
};

#endif /* CLOUDINT_H_ */
