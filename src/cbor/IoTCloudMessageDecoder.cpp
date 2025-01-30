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

#include "IoTCloudMessageDecoder.h"
#include <AIoTC_Config.h>

static inline bool copyCBORStringToArray(CborValue * param, char * dest, size_t dest_size) {
  if (cbor_value_is_text_string(param)) {
    // NOTE: keep in mind that _cbor_value_copy_string tries to put a \0 at the end of the string
    if(_cbor_value_copy_string(param, dest, &dest_size, NULL) == CborNoError) {
      return true;
    }
  }

  return false;
}

static inline size_t copyCBORByteToArray(CborValue * param, uint8_t * dest, size_t dest_size) {
  if (cbor_value_is_byte_string(param)) {
    // NOTE: keep in mind that _cbor_value_copy_string tries to put a \0 at the end of the string
    if(_cbor_value_copy_string(param, dest, &dest_size, NULL) == CborNoError) {
      return dest_size;
    }
  }

  return 0;
}

/******************************************************************************
    MESSAGE DECODE FUNCTIONS
 ******************************************************************************/

MessageDecoder::Status ThingUpdateCommandDecoder::decode(CborValue* iter, Message *msg) {
  ThingUpdateCmd * thingCommand = (ThingUpdateCmd *) msg;

  // Message is composed of a single parameter, a string (thing_id)
  if (!copyCBORStringToArray(iter, thingCommand->params.thing_id, sizeof(thingCommand->params.thing_id))) {
    return MessageDecoder::Status::Error;
  }

  return MessageDecoder::Status::Complete;
}

MessageDecoder::Status ThingDetachCommandDecoder::decode(CborValue* iter, Message *msg) {
  ThingDetachCmd * thingCommand = (ThingDetachCmd *) msg;

  // Message is composed of a single parameter, a string (thing_id)
  if (!copyCBORStringToArray(iter, thingCommand->params.thing_id, sizeof(thingCommand->params.thing_id))) {
    return MessageDecoder::Status::Error;
  }

  return MessageDecoder::Status::Complete;
}

MessageDecoder::Status TimezoneCommandDownDecoder::decode(CborValue* iter, Message *msg) {
  TimezoneCommandDown * setTz = (TimezoneCommandDown *) msg;

  // Message is composed of 2 parameters, offset 32-bit signed integer and until 32-bit unsigned integer
  // Get offset
  if (!cbor_value_is_integer(iter)) {
    return MessageDecoder::Status::Error;
  }

  int64_t val = 0;
  if (cbor_value_get_int64(iter, &val) != CborNoError) {
    return MessageDecoder::Status::Error;
  }

  setTz->params.offset = static_cast<int32_t>(val);

  // Next
  if (cbor_value_advance(iter) != CborNoError) {
    return MessageDecoder::Status::Error;
  }

  // Get until
  if (!cbor_value_is_integer(iter)) {
    return MessageDecoder::Status::Error;
  }

  uint64_t val1 = 0;
  if (cbor_value_get_uint64(iter, &val1) != CborNoError) {
    return MessageDecoder::Status::Error;
  }

  setTz->params.until = static_cast<uint32_t>(val1);

  return MessageDecoder::Status::Complete;
}

MessageDecoder::Status LastValuesUpdateCommandDecoder::decode(CborValue* iter, Message *msg) {
  LastValuesUpdateCmd * setLv = (LastValuesUpdateCmd *) msg;

  if(!cbor_value_is_byte_string(iter)) {
    return MessageDecoder::Status::Error;
  }

  // Cortex M0 is not able to assign a value to pointed memory that is not 32bit aligned
  // we use a support variable to cope with that
  size_t s;

  // NOTE: cbor_value_dup_byte_string calls malloc and assigns it to the second parameter of the call,
  //       free must be called. Free has to be called only if decode succeeds.
  //       Read tinyCbor documentation for more information.
  if (cbor_value_dup_byte_string(iter, &setLv->params.last_values, &s, NULL) != CborNoError) {
    return MessageDecoder::Status::Error;
  }

  setLv->params.length = s;

  return MessageDecoder::Status::Complete;
}

MessageDecoder::Status OtaUpdateCommandDecoder::decode(CborValue* iter, Message *msg) {
  CborError error = CborNoError;
  OtaUpdateCmdDown * ota = (OtaUpdateCmdDown *) msg;

  // Message is composed 4 parameters: id, url, initialSha, finalSha
  if (!copyCBORByteToArray(iter, ota->params.id, sizeof(ota->params.id))) {
    return MessageDecoder::Status::Error;
  }

  error = cbor_value_advance(iter);

  if ((error != CborNoError) || !copyCBORStringToArray(iter, ota->params.url, sizeof(ota->params.url))) {
    return MessageDecoder::Status::Error;
  }

  error = cbor_value_advance(iter);

  if ((error != CborNoError) ||
      copyCBORByteToArray(iter, ota->params.initialSha256,
        sizeof(ota->params.initialSha256)) != sizeof(ota->params.initialSha256)) {
    return MessageDecoder::Status::Error;
  }

  error = cbor_value_advance(iter);

  if ((error != CborNoError) ||
      copyCBORByteToArray(iter, ota->params.finalSha256,
        sizeof(ota->params.finalSha256)) != sizeof(ota->params.finalSha256)) {
    return MessageDecoder::Status::Error;
  }

  return MessageDecoder::Status::Complete;
}

static OtaUpdateCommandDecoder        otaUpdateCommandDecoder;
static ThingUpdateCommandDecoder      thingUpdateCommandDecoder;
static ThingDetachCommandDecoder      thingDetachCommandDecoder;
static LastValuesUpdateCommandDecoder lastValuesUpdateCommandDecoder;
static TimezoneCommandDownDecoder     timezoneCommandDownDecoder;
