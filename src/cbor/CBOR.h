/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

/******************************************************************************
  INCLUDE
 ******************************************************************************/
#include <message/Commands.h>
#include <Arduino_CBOR.h>

/******************************************************************************
  TYPEDEF
 ******************************************************************************/

enum CBORCommandTag: CBORTag {
  // Commands UP
  CBOROtaBeginUp           = 0x010000,
  CBORThingBeginCmd        = 0x010300,
  CBORLastValuesBeginCmd   = 0x010500,
  CBORDeviceBeginCmd       = 0x010700,
  CBOROtaProgressCmdUp     = 0x010200,
  CBORTimezoneCommandUp    = 0x010800,
  CBORDeviceNetConfigCmdUp = 0x011100,

  // Commands DOWN
  CBOROtaUpdateCmdDown     = 0x010100,
  CBORThingUpdateCmd       = 0x010400,
  CBORThingDetachCmd       = 0x011000,
  CBORLastValuesUpdate     = 0x010600,
  CBORTimezoneCommandDown  = 0x010900,
};
