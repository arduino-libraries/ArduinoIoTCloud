/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2019 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef CLOUDINT_H_
#define CLOUDINT_H_

/******************************************************************************
  INCLUDE
 ******************************************************************************/

#include <Arduino.h>
#include "../Property.h"

/******************************************************************************
  CLASS DECLARATION
 ******************************************************************************/

class CloudInt : public Property {
  private:
    int _value,
        _cloud_value;
  public:
    CloudInt() {
      CloudInt(0);
    }
    CloudInt(int v) : _value(v), _cloud_value(v) {}
    operator int() const {
      return _value;
    }
    virtual bool isDifferentFromCloud() {
      return _value != _cloud_value && (abs(_value - _cloud_value) >= Property::_min_delta_property);
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
    CloudInt& operator=(int v) {
      _value = v;
      updateLocalTimestamp();
      return *this;
    }
    CloudInt& operator=(CloudInt v) {
      return operator=((int)v);
    }
    CloudInt& operator+=(int v) {
      return operator=(_value += v);
    }
    CloudInt& operator-=(int v) {
      return operator=(_value -= v);
    }
    CloudInt& operator*=(int v) {
      return operator=(_value *= v);
    }
    CloudInt& operator/=(int v) {
      return operator=(_value /= v);
    }
    CloudInt& operator%=(int v) {
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
    CloudInt& operator&=(int v) {
      return operator=(_value &= v);
    }
    CloudInt& operator|=(int v) {
      return operator=(_value |= v);
    }
    CloudInt& operator^=(int v) {
      return operator=(_value ^= v);
    }
    CloudInt& operator<<=(int v) {
      return operator=(_value <<= v);
    }
    CloudInt& operator>>=(int v) {
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
    friend CloudInt operator+(CloudInt iw, int v) {
      return iw += v;
    }
    friend CloudInt operator+(int v, CloudInt iw) {
      return CloudInt(v) += iw;
    }
    friend CloudInt operator-(CloudInt iw, CloudInt v) {
      return iw -= v;
    }
    friend CloudInt operator-(CloudInt iw, int v) {
      return iw -= v;
    }
    friend CloudInt operator-(int v, CloudInt iw) {
      return CloudInt(v) -= iw;
    }
    friend CloudInt operator*(CloudInt iw, CloudInt v) {
      return iw *= v;
    }
    friend CloudInt operator*(CloudInt iw, int v) {
      return iw *= v;
    }
    friend CloudInt operator*(int v, CloudInt iw) {
      return CloudInt(v) *= iw;
    }
    friend CloudInt operator/(CloudInt iw, CloudInt v) {
      return iw /= v;
    }
    friend CloudInt operator/(CloudInt iw, int v) {
      return iw /= v;
    }
    friend CloudInt operator/(int v, CloudInt iw) {
      return CloudInt(v) /= iw;
    }
    friend CloudInt operator%(CloudInt iw, CloudInt v) {
      return iw %= v;
    }
    friend CloudInt operator%(CloudInt iw, int v) {
      return iw %= v;
    }
    friend CloudInt operator%(int v, CloudInt iw) {
      return CloudInt(v) %= iw;
    }
    friend CloudInt operator&(CloudInt iw, CloudInt v) {
      return iw &= v;
    }
    friend CloudInt operator&(CloudInt iw, int v) {
      return iw &= v;
    }
    friend CloudInt operator&(int v, CloudInt iw) {
      return CloudInt(v) &= iw;
    }
    friend CloudInt operator|(CloudInt iw, CloudInt v) {
      return iw |= v;
    }
    friend CloudInt operator|(CloudInt iw, int v) {
      return iw |= v;
    }
    friend CloudInt operator|(int v, CloudInt iw) {
      return CloudInt(v) |= iw;
    }
    friend CloudInt operator^(CloudInt iw, CloudInt v) {
      return iw ^= v;
    }
    friend CloudInt operator^(CloudInt iw, int v) {
      return iw ^= v;
    }
    friend CloudInt operator^(int v, CloudInt iw) {
      return CloudInt(v) ^= iw;
    }
    friend CloudInt operator<<(CloudInt iw, CloudInt v) {
      return iw <<= v;
    }
    friend CloudInt operator<<(CloudInt iw, int v) {
      return iw <<= v;
    }
    friend CloudInt operator<<(int v, CloudInt iw) {
      return CloudInt(v) <<= iw;
    }
    friend CloudInt operator>>(CloudInt iw, CloudInt v) {
      return iw >>= v;
    }
    friend CloudInt operator>>(CloudInt iw, int v) {
      return iw >>= v;
    }
    friend CloudInt operator>>(int v, CloudInt iw) {
      return CloudInt(v) >>= iw;
    }
};

#endif /* CLOUDINT_H_ */
