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

#ifndef CLOUDDIMMEREDLIGHT_H_
#define CLOUDDIMMEREDLIGHT_H_

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include <math.h>
#include <Arduino.h>
#include "CloudColoredLight.h"

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class CloudDimmeredLight : public CloudColoredLight {
  private:
  public:
    float getBrightness() {
      return _value.bri;
    }
    bool getSwitch() {
      return _value.swi;
    }

    virtual void appendAttributesToCloud() {
      appendAttribute(_value.swi);
      appendAttribute(_value.bri);
    }
    virtual void setAttributesFromCloud() {
      setAttribute(_cloud_value.swi);
      setAttribute(_cloud_value.bri);
    }
};

#endif /* CLOUDDIMMEREDLIGHT_H_ */