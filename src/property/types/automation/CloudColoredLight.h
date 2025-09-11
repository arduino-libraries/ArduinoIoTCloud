/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2019 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef CLOUDCOLOREDLIGHT_H_
#define CLOUDCOLOREDLIGHT_H_

/******************************************************************************
  INCLUDE
 ******************************************************************************/

#include <math.h>
#include <Arduino.h>
#include "../CloudColor.h"

/******************************************************************************
  CLASS DECLARATION
 ******************************************************************************/

class ColoredLight : public Color {
  public:
    bool swi;
    ColoredLight(bool swi, float h, float s, float b): Color(h, s, b), swi(swi) {
    }

    bool operator==(ColoredLight & aLight) {
      return Color::operator==(aLight) && aLight.swi == swi;
    }

    bool operator!=(ColoredLight & aLight) {
      return !(operator==(aLight));
    }

};

class CloudColoredLight : public CloudColor {
  private:
    ColoredLight _value,
                 _cloud_value;
  public:
    CloudColoredLight() : _value(false, 0, 0, 0), _cloud_value(false, 0, 0, 0) {}
    CloudColoredLight(bool swi, float hue, float saturation, float brightness) : _value(swi, hue, saturation, brightness), _cloud_value(swi, hue, saturation, brightness) {}

    virtual bool isDifferentFromCloud() {

      return _value != _cloud_value;
    }

    CloudColoredLight& operator=(ColoredLight aLight) {
      _value.swi = aLight.swi;
      _value.hue = aLight.hue;
      _value.sat = aLight.sat;
      _value.bri = aLight.bri;
      updateLocalTimestamp();
      return *this;
    }

    ColoredLight getCloudValue() {
      return _cloud_value;
    }

    ColoredLight getValue() {
      return _value;
    }

    bool getSwitch() {
      return _value.swi;
    }

    void setSwitch(bool const swi) {
      _value.swi = swi;
      updateLocalTimestamp();
    }

    float getHue() {
      return _value.hue;
    }

    void setHue(float const hue) {
      _value.hue = hue;
      updateLocalTimestamp();
    }

    float getSaturation() {
      return _value.sat;
    }

    void setSaturation(float const sat) {
      _value.sat = sat;
      updateLocalTimestamp();
    }

    float getBrightness() {
      return _value.bri;
    }

    void setBrightness(float const bri) {
      _value.bri = bri;
      updateLocalTimestamp();
    }

    virtual void fromCloudToLocal() {
      _value = _cloud_value;
    }
    virtual void fromLocalToCloud() {
      _cloud_value = _value;
    }
    virtual CborError appendAttributesToCloud(CborEncoder *encoder) {
      CHECK_CBOR_MULTI(appendAttribute(_value.swi, "swi", encoder));
      CHECK_CBOR_MULTI(appendAttribute(_value.hue, "hue", encoder));
      CHECK_CBOR_MULTI(appendAttribute(_value.sat, "sat", encoder));
      CHECK_CBOR_MULTI(appendAttribute(_value.bri, "bri", encoder));
      return CborNoError;
    }
    virtual void setAttributesFromCloud() {
      setAttribute(_cloud_value.swi, "swi");
      setAttribute(_cloud_value.hue, "hue");
      setAttribute(_cloud_value.sat, "sat");
      setAttribute(_cloud_value.bri, "bri");
    }
};

#endif /* CLOUDCOLOREDLIGHT_H_ */
