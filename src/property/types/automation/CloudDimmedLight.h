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

    virtual CborError appendAttributesToCloud() {
      CHECK_CBOR_MULTI(appendAttribute(_value.swi));
      // To allow visualization through color widget
      // Start
      float hue = 0;
      float sat = 0;
      CHECK_CBOR_MULTI(appendAttributeReal(hue, getAttributeName(".hue", '.'), encoder));
      CHECK_CBOR_MULTI(appendAttributeReal(sat, getAttributeName(".sat", '.'), encoder));
      CHECK_CBOR_MULTI(appendAttribute(_value.bri));
      // should be only:
      // appendAttribute(_value.bri);
      // end
      return CborNoError;
    }

    virtual void setAttributesFromCloud() {
      setAttribute(_cloud_value.swi);
      setAttribute(_cloud_value.bri);
    }
};

#endif /* CLOUDDIMMEDLIGHT_H_ */