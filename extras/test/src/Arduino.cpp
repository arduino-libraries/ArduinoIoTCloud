/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2019 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/******************************************************************************
  INCLUDE
 ******************************************************************************/

#include <Arduino.h>

/******************************************************************************
  GLOBAL VARIABLES
 ******************************************************************************/

static unsigned long current_millis = 0;

/******************************************************************************
  PUBLIC FUNCTIONS
 ******************************************************************************/

void set_millis(unsigned long const millis)
{
  current_millis = millis;
}

unsigned long millis()
{
  return current_millis;
}
