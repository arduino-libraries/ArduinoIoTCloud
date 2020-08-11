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

#ifndef CLOUD_TEMPERATURE_SENSOR_H_
#define CLOUD_TEMPERATURE_SENSOR_H_

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include "../CloudFloat.h"

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class CloudTemperatureSensor : public CloudFloat
{
  public:

    CloudTemperatureSensor & operator = (float v)
    {
      CloudFloat::operator=(v);
      return *this;
    }
};

#endif /* CLOUD_TEMPERATURE_SENSOR_H_ */
