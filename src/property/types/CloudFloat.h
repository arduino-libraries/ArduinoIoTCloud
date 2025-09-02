/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2019 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef CLOUDFLOAT_H_
#define CLOUDFLOAT_H_

#include <math.h>
#include "../math_utils.h"

/******************************************************************************
  INCLUDE
 ******************************************************************************/

#include <Arduino.h>
#include "../Property.h"

/******************************************************************************
  CLASS DECLARATION
 ******************************************************************************/

class CloudFloat : public Property {
  protected:
    float _value,
          _cloud_value;
  public:
    CloudFloat() {
      CloudFloat(0.0f);
    }
    CloudFloat(float v) : _value(v), _cloud_value(v) {}
    operator float() const {
      return _value;
    }
    virtual bool isDifferentFromCloud() {
      return arduino::math::ieee754_different(_value, _cloud_value, Property::_min_delta_property);
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
    CloudFloat& operator=(float v) {
      _value = v;
      updateLocalTimestamp();
      return *this;
    }
    CloudFloat& operator=(CloudFloat v) {
      return operator=((float)v);
    }
    CloudFloat& operator+=(float v) {
      return operator=(_value += v);
    }
    CloudFloat& operator-=(float v) {
      return operator=(_value -= v);
    }
    CloudFloat& operator*=(float v) {
      return operator=(_value *= v);
    }
    CloudFloat& operator/=(float v) {
      return operator=(_value /= v);
    }
    CloudFloat& operator++() {
      return operator=(_value + 1.0f);
    }
    CloudFloat& operator--() {
      return operator=(_value - 1.0f);
    }
    CloudFloat operator++(int) {
      operator=(_value + 1.0f);
      return CloudFloat(_value);
    }
    CloudFloat operator--(int) {
      operator=(_value - 1.0f);
      return CloudFloat(_value);
    }

    //friends
    friend CloudFloat operator+(CloudFloat iw, CloudFloat v) {
      return iw += v;
    }
    friend CloudFloat operator+(CloudFloat iw, float v) {
      return iw += v;
    }
    friend CloudFloat operator+(CloudFloat iw, int v) {
      return iw += (float)v;
    }
    friend CloudFloat operator+(CloudFloat iw, double v) {
      return iw += (float)v;
    }
    friend CloudFloat operator+(float v, CloudFloat iw) {
      return CloudFloat(v) += iw;
    }
    friend CloudFloat operator+(int v, CloudFloat iw) {
      return CloudFloat(v) += iw;
    }
    friend CloudFloat operator+(double v, CloudFloat iw) {
      return CloudFloat(v) += iw;
    }
    friend CloudFloat operator-(CloudFloat iw, CloudFloat v) {
      return iw -= v;
    }
    friend CloudFloat operator-(CloudFloat iw, float v) {
      return iw -= v;
    }
    friend CloudFloat operator-(CloudFloat iw, int v) {
      return iw -= (float)v;
    }
    friend CloudFloat operator-(CloudFloat iw, double v) {
      return iw -= (float)v;
    }
    friend CloudFloat operator-(float v, CloudFloat iw) {
      return CloudFloat(v) -= iw;
    }
    friend CloudFloat operator-(int v, CloudFloat iw) {
      return CloudFloat(v) -= iw;
    }
    friend CloudFloat operator-(double v, CloudFloat iw) {
      return CloudFloat(v) -= iw;
    }
    friend CloudFloat operator*(CloudFloat iw, CloudFloat v) {
      return iw *= v;
    }
    friend CloudFloat operator*(CloudFloat iw, float v) {
      return iw *= v;
    }
    friend CloudFloat operator*(CloudFloat iw, int v) {
      return iw *= (float)v;
    }
    friend CloudFloat operator*(CloudFloat iw, double v) {
      return iw *= (float)v;
    }
    friend CloudFloat operator*(float v, CloudFloat iw) {
      return CloudFloat(v) *= iw;
    }
    friend CloudFloat operator*(int v, CloudFloat iw) {
      return CloudFloat(v) *= iw;
    }
    friend CloudFloat operator*(double v, CloudFloat iw) {
      return CloudFloat(v) *= iw;
    }
    friend CloudFloat operator/(CloudFloat iw, CloudFloat v) {
      return iw /= v;
    }
    friend CloudFloat operator/(CloudFloat iw, float v) {
      return iw /= v;
    }
    friend CloudFloat operator/(CloudFloat iw, int v) {
      return iw /= (float)v;
    }
    friend CloudFloat operator/(CloudFloat iw, double v) {
      return iw /= (float)v;
    }
    friend CloudFloat operator/(float v, CloudFloat iw) {
      return CloudFloat(v) /= iw;
    }
    friend CloudFloat operator/(int v, CloudFloat iw) {
      return CloudFloat(v) /= iw;
    }
    friend CloudFloat operator/(double v, CloudFloat iw) {
      return CloudFloat(v) /= iw;
    }
};

#endif /* CLOUDFLOAT_H_ */
