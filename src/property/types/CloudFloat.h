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
    CloudFloat()                                            {
      CloudFloat(0.0f);
    }
    CloudFloat(float v) : _value(v), _cloud_value(v) {}
    operator float() const {
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
    virtual CborError appendAttributesToCloud() {
      return appendAttribute(_value);
    }
    virtual void setAttributesFromCloud() {
      setAttribute(_cloud_value);
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
