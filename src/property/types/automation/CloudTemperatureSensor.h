/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2019 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

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
