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
  protected:
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

    virtual void fromCloudToLocal() {
      _value = _cloud_value;
    }
    virtual void fromLocalToCloud() {
      _cloud_value = _value;
    }
    virtual void appendAttributesToCloud() {
      appendAttribute(_value.swi);
      appendAttribute(_value.hue);
      appendAttribute(_value.sat);
      // To allow visualization through color widget
      if (_value.swi) {
        appendAttribute(_value.bri);
      } else {
        float bri = 0;
        appendAttributeReal(bri, getAttributeName(".bri", '.'), encoder);
      }
    }
    virtual void setAttributesFromCloud() {
      setAttribute(_cloud_value.swi);
      setAttribute(_cloud_value.hue);
      setAttribute(_cloud_value.sat);
      setAttribute(_cloud_value.bri);
    }
};

#endif /* CLOUDCOLOREDLIGHT_H_ */