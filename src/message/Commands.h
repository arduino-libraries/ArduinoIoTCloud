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
 * DEFINE
 ******************************************************************************/

#define THING_ID_SIZE               37
#define SHA256_SIZE                 32
#define URL_SIZE                   256
#define ID_SIZE                     16
#define MAX_LIB_VERSION_SIZE        10

/******************************************************************************
    TYPEDEF
 ******************************************************************************/

enum CommandId: uint32_t {

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

  /* OTA commands */
  OtaBeginUpId,
  OtaProgressCmdUpId,
  OtaUpdateCmdDownId,

  /* Timezone commands */
  TimezoneCommandUpId,
  TimezoneCommandDownId,

  /* Unknown command id */
  UnknownCmdId
};

struct Command {
  CommandId id;
};

typedef Command Message;

struct DeviceBeginCmd {
  Command c;
  struct {
    char lib_version[MAX_LIB_VERSION_SIZE];
  } params;
};

struct ThingBeginCmd {
  Command c;
  struct {
    char thing_id[THING_ID_SIZE];
  } params;
};

struct ThingUpdateCmd {
  Command c;
  struct {
    char thing_id[THING_ID_SIZE];
  } params;
};

struct LastValuesBeginCmd {
  Command c;
};

struct LastValuesUpdateCmd {
  Command c;
  struct {
    uint8_t * last_values;
    size_t length;
  } params;
};

struct OtaBeginUp {
  Command c;
  struct {
    uint8_t sha [SHA256_SIZE];
  } params;
};

struct OtaProgressCmdUp {
  Command c;
  struct {
    uint8_t  id[ID_SIZE];
    uint8_t  state;
    int32_t  state_data;
    uint64_t time;
  } params;
};

struct OtaUpdateCmdDown {
  Command c;
  struct {
    uint8_t id[ID_SIZE];
    char    url[URL_SIZE];
    uint8_t initialSha256[SHA256_SIZE];
    uint8_t finalSha256[SHA256_SIZE];
  } params;
};

struct TimezoneCommandUp {
    Command c;
};

struct TimezoneCommandDown {
  Command c;
  struct {
    int32_t offset;
    uint32_t until;
  } params;
};

union CommandDown {
  struct Command                  c;
  struct OtaUpdateCmdDown         otaUpdateCmdDown;
  struct ThingUpdateCmd           thingUpdateCmd;
  struct LastValuesUpdateCmd      lastValuesUpdateCmd;
  struct TimezoneCommandDown      timezoneCommandDown;
};
