/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2019 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef CLOUDMOTIONSENSOR_H_
#define CLOUDMOTIONSENSOR_H_

/******************************************************************************
  INCLUDE
 ******************************************************************************/

#include <Arduino.h>
#include "../CloudBool.h"

/******************************************************************************
  CLASS DECLARATION
 ******************************************************************************/

class CloudMotionSensor : public CloudBool {
  private:
  public:
    operator bool() const {
      return _value;
    }
    CloudMotionSensor& operator=(bool v) {
      CloudBool::operator=(v);
      return *this;
    }
};

#endif /* CLOUDMOTIONSENSOR_H_ */
