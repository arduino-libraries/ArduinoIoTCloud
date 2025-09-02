/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2019 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef CLOUDDIMMEDLIGHT_H_
#define CLOUDDIMMEDLIGHT_H_

/******************************************************************************
  INCLUDE
 ******************************************************************************/

#include <math.h>
#include <Arduino.h>
#include "../../Property.h"

/******************************************************************************
  CLASS DECLARATION
 ******************************************************************************/
class DimmedLight {
  public:
    bool swi;
    float bri;
    DimmedLight(bool swi, float bri): swi(swi), bri(bri) {
    }

    bool operator==(DimmedLight & aLight) {
      return aLight.swi == swi && aLight.bri == bri;
    }

    bool operator!=(DimmedLight & aLight) {
      return !(operator==(aLight));
    }

};

class CloudDimmedLight : public Property {
  private:
    DimmedLight _value,
                _cloud_value;

  public:
    CloudDimmedLight() : _value(false, 0), _cloud_value(false, 0) {}
    CloudDimmedLight(bool swi, float brightness) : _value(swi, brightness), _cloud_value(swi, brightness) {}

    virtual bool isDifferentFromCloud() {

      return _value != _cloud_value;
    }

    CloudDimmedLight& operator=(DimmedLight aLight) {
      _value.swi = aLight.swi;
      _value.bri = aLight.bri;
      updateLocalTimestamp();
      return *this;
    }

    DimmedLight getCloudValue() {
      return _cloud_value;
    }

    DimmedLight getValue() {
      return _value;
    }

    float getBrightness() {
      return _value.bri;
    }

    void setBrightness(float const bri) {
      _value.bri = bri;
    }

    bool getSwitch() {
      return _value.swi;
    }

    void setSwitch(bool const swi) {
      _value.swi = swi;
    }

    virtual void fromCloudToLocal() {
      _value = _cloud_value;
    }
    virtual void fromLocalToCloud() {
      _cloud_value = _value;
    }

    virtual CborError appendAttributesToCloud(CborEncoder *encoder) {
      CHECK_CBOR_MULTI(appendAttribute(_value.swi, "swi", encoder));
      CHECK_CBOR_MULTI(appendAttribute(_value.bri, "bri", encoder));
      return CborNoError;
    }

    virtual void setAttributesFromCloud() {
      setAttribute(_cloud_value.swi, "swi");
      setAttribute(_cloud_value.bri, "bri");
    }
};

#endif /* CLOUDDIMMEDLIGHT_H_ */
