/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2019 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

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
    CloudUnsignedInt() {
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
