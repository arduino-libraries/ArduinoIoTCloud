/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/******************************************************************************
    INCLUDE
 ******************************************************************************/

#include <Arduino.h>

#include "MessageDecoder.h"
#include <AIoTC_Config.h>

/******************************************************************************
    PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

/******************************************************************************
    PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

static bool copyCBORStringToArray(CborValue * param, char * dest, size_t dest_size) {
  if (cbor_value_is_text_string(param)) {
    // NOTE: keep in mind that _cbor_value_copy_string tries to put a \0 at the end of the string
    if(_cbor_value_copy_string(param, dest, &dest_size, NULL) == CborNoError) {
      return true;
    }
  }

  return false;
}

// FIXME dest_size should be also returned, the copied byte array can have a different size from the starting one
// for the time being we need this on SHA256 only
static bool copyCBORByteToArray(CborValue * param, uint8_t * dest, size_t dest_size) {
  if (cbor_value_is_byte_string(param)) {
    // NOTE: keep in mind that _cbor_value_copy_string tries to put a \0 at the end of the string
    if(_cbor_value_copy_string(param, dest, &dest_size, NULL) == CborNoError) {
      return true;
    }
  }

  return false;
}

/******************************************************************************
    MESSAGE DECODE FUNCTIONS
 ******************************************************************************/

Decoder::Status ThingUpdateCommandDecoder::decode(CborValue* iter, Message *msg) {
  ThingUpdateCmd * thingCommand = (ThingUpdateCmd *) msg;

  // Message is composed of a single parameter, a string (thing_id)
  if (!copyCBORStringToArray(iter, thingCommand->params.thing_id, sizeof(thingCommand->params.thing_id))) {
    return Decoder::Status::Error;
  }

  return Decoder::Status::Complete;
}

Decoder::Status ThingDetachCommandDecoder::decode(CborValue* iter, Message *msg) {
  ThingDetachCmd * thingCommand = (ThingDetachCmd *) msg;

  // Message is composed of a single parameter, a string (thing_id)
  if (!copyCBORStringToArray(iter, thingCommand->params.thing_id, sizeof(thingCommand->params.thing_id))) {
    return Decoder::Status::Error;
  }

  return Decoder::Status::Complete;
}

Decoder::Status TimezoneCommandDownDecoder::decode(CborValue* iter, Message *msg) {
  TimezoneCommandDown * setTz = (TimezoneCommandDown *) msg;

  // Message is composed of 2 parameters, offset 32-bit signed integer and until 32-bit unsigned integer
  // Get offset
  if (cbor_value_is_integer(iter)) {
    int64_t val = 0;
    if (cbor_value_get_int64(iter, &val) == CborNoError) {
      setTz->params.offset = static_cast<int32_t>(val);
    }
  }

  // Next
  if (cbor_value_advance(iter) != CborNoError) {
    return Decoder::Status::Error;
  }

  // Get until
  if (cbor_value_is_integer(iter)) {
    uint64_t val = 0;
    if (cbor_value_get_uint64(iter, &val) == CborNoError) {
      setTz->params.until = static_cast<uint32_t>(val);
    }
  }

  return Decoder::Status::Complete;
}

Decoder::Status LastValuesUpdateCommandDecoder::decode(CborValue* iter, Message *msg) {
  LastValuesUpdateCmd * setLv = (LastValuesUpdateCmd *) msg;

  // Message is composed by a single parameter, a variable length byte array.
  if (cbor_value_is_byte_string(iter)) {
    // Cortex M0 is not able to assign a value to pointed memory that is not 32bit aligned
    // we use a support variable to cope with that
    size_t s;
    if (cbor_value_dup_byte_string(iter, &setLv->params.last_values, &s, NULL) != CborNoError) {
      return Decoder::Status::Error;
    }

    setLv->params.length = s;
  }

  return Decoder::Status::Complete;
}

Decoder::Status OtaUpdateCommandDecoder::decode(CborValue* iter, Message *msg) {
  CborError error = CborNoError;
  OtaUpdateCmdDown * ota = (OtaUpdateCmdDown *) msg;

  // Message is composed 4 parameters: id, url, initialSha, finalSha
  if (!copyCBORByteToArray(iter, ota->params.id, sizeof(ota->params.id))) {
    return Decoder::Status::Error;
  }

  error = cbor_value_advance(iter);

  if ((error != CborNoError) || !copyCBORStringToArray(iter, ota->params.url, sizeof(ota->params.url))) {
    return Decoder::Status::Error;
  }

  error = cbor_value_advance(iter);

  if ((error != CborNoError) || !copyCBORByteToArray(iter, ota->params.initialSha256, sizeof(ota->params.initialSha256))) {
    return Decoder::Status::Error;
  }

  error = cbor_value_advance(iter);

  if ((error != CborNoError) || !copyCBORByteToArray(iter, ota->params.finalSha256, sizeof(ota->params.finalSha256))) {
    return Decoder::Status::Error;
  }

  return Decoder::Status::Complete;
}

static OtaUpdateCommandDecoder        otaUpdateCommandDecoder;
static ThingUpdateCommandDecoder      thingUpdateCommandDecoder;
static ThingDetachCommandDecoder      thingDetachCommandDecoder;
static LastValuesUpdateCommandDecoder lastValuesUpdateCommandDecoder;
static TimezoneCommandDownDecoder     timezoneCommandDownDecoder;
