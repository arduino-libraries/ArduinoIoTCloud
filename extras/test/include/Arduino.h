/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2019 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef TEST_ARDUINO_H_
#define TEST_ARDUINO_H_

/******************************************************************************
  INCLUDE
 ******************************************************************************/

#include <string>
#include <IPAddress.h>

/******************************************************************************
  DEFINES
 ******************************************************************************/
#ifndef min
  #define min(a,b) ((a)<(b)?(a):(b))
#endif

/******************************************************************************
  TYPEDEF
 ******************************************************************************/

typedef std::string String;

/******************************************************************************
  FUNCTION PROTOTYPES
 ******************************************************************************/

void          set_millis(unsigned long const millis);
unsigned long millis();

#endif /* TEST_ARDUINO_H_ */
