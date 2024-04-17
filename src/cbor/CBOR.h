/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

/******************************************************************************
 * INCLUDE
 ******************************************************************************/
#include <message/Commands.h>

/******************************************************************************
   TYPEDEF
 ******************************************************************************/

enum CBORCommandTag: uint64_t {
  // Commands UP
  CBOROtaBeginUp          = 0x010000,
  CBORThingBeginCmd       = 0x010300,
  CBORLastValuesBeginCmd  = 0x010500,
  CBORDeviceBeginCmd      = 0x010700,
  CBOROtaProgressCmdUp    = 0x010200,
  CBORTimezoneCommandUp   = 0x010800,

  // Commands DOWN
  CBOROtaUpdateCmdDown    = 0x010100,
  CBORThingUpdateCmd      = 0x010400,
  CBORLastValuesUpdate    = 0x010600,
  CBORTimezoneCommandDown = 0x010900,

  // Unknown Command Tag https://www.iana.org/assignments/cbor-tags/cbor-tags.xhtml
  CBORUnknownCmdTag16b    = 0xffff,              // invalid tag
  CBORUnknownCmdTag32b    = 0xffffffff,          // invalid tag
  CBORUnknownCmdTag64b    = 0xffffffffffffffff,  // invalid tag
  CBORUnknownCmdTag       = CBORUnknownCmdTag32b
};

/******************************************************************************
 * FUNCTION DECLARATION
 ******************************************************************************/

CommandId toCommandId(CBORCommandTag tag);
CBORCommandTag toCBORCommandTag(CommandId id);
