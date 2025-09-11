/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2020 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef ARDUINO_AIOTC_UTILITY_WATCHDOG_H_
#define ARDUINO_AIOTC_UTILITY_WATCHDOG_H_

/******************************************************************************
  INCLUDE
 ******************************************************************************/

#include <Arduino_ConnectionHandler.h>

/******************************************************************************
  FUNCTION DECLARATION
 ******************************************************************************/

#if defined (ARDUINO_ARCH_SAMD) || defined (ARDUINO_ARCH_MBED)
void watchdog_enable();
void watchdog_reset();
void watchdog_enable_network_feed(NetworkAdapter ni);
#endif /* (ARDUINO_ARCH_SAMD) || (ARDUINO_ARCH_MBED) */

#endif /* ARDUINO_AIOTC_UTILITY_WATCHDOG_H_ */
