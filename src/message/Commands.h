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

#include <stdint.h>
#include <stddef.h>

/******************************************************************************
 * TYPEDEF
 ******************************************************************************/

enum CommandId : uint16_t {

  /* Device commands */
  DeviceBeginCmdId,
  ThingBeginCmdId,
  ThingUpdateCmdId,
  DeviceRegisteredCmdId,
  DeviceAttachedCmdId,
  DeviceDetachedCmdId,

  /* Thing commands */
  LastValuesBeginCmdId,
  LastValuesUpdateCmdId,
  PropertiesUpdateCmdId,

  /* Generic commands */
  ResetCmdId,

  /* Unknown command id */
  UnknownCmdId
};

struct Command {
  CommandId id;
};

typedef Command Message;
