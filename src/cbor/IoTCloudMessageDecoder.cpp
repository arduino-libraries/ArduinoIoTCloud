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
#include <cbor/utils/decoder.h>
#include <AIoTC_Config.h>

/******************************************************************************
  MESSAGE DECODE FUNCTIONS
 ******************************************************************************/

MessageDecoder::Status ThingUpdateCommandDecoder::decode(CborValue* iter, Message *msg) {
  ThingUpdateCmd * thingCommand = (ThingUpdateCmd *) msg;

  size_t dest_size = sizeof(thingCommand->params.thing_id);

  // Message is composed of a single parameter, a string (thing_id)
  if (cbor::utils::copyCBORStringToArray(
      iter, thingCommand->params.thing_id,
      dest_size) == MessageDecoder::Status::Error) {
    return MessageDecoder::Status::Error;
  }

  return MessageDecoder::Status::Complete;
}

MessageDecoder::Status ThingDetachCommandDecoder::decode(CborValue* iter, Message *msg) {
  ThingDetachCmd * thingCommand = (ThingDetachCmd *) msg;

  size_t dest_size = sizeof(thingCommand->params.thing_id);


  // Message is composed of a single parameter, a string (thing_id)
  if (cbor::utils::copyCBORStringToArray(
      iter,
      thingCommand->params.thing_id,
      dest_size) == MessageDecoder::Status::Error) {
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
  OtaUpdateCmdDown * ota = (OtaUpdateCmdDown *) msg;
  size_t dest_size = sizeof(ota->params.id);

  // Message is composed 4 parameters: id, url, initialSha, finalSha

  // decoding parameter id
  if (cbor::utils::copyCBORByteToArray(
      iter,
      ota->params.id,
      dest_size) == MessageDecoder::Status::Error) {
    return MessageDecoder::Status::Error;
  }

  // decoding parameter url
  if(cbor_value_advance(iter) != CborNoError) {
    return MessageDecoder::Status::Error;
  }

  dest_size = sizeof(ota->params.url);

  if (cbor::utils::copyCBORStringToArray(iter,
      ota->params.url,
      dest_size) == MessageDecoder::Status::Error) {
    return MessageDecoder::Status::Error;
  }

  // decoding parameter initialSha256
  if(cbor_value_advance(iter) != CborNoError) {
    return MessageDecoder::Status::Error;
  }

  dest_size = sizeof(ota->params.initialSha256);

  if (cbor::utils::copyCBORByteToArray(iter,
        ota->params.initialSha256,
        dest_size) == MessageDecoder::Status::Error ||
      dest_size != sizeof(ota->params.initialSha256)) {
    return MessageDecoder::Status::Error;
  }

  // decoding parameter finalSha256
  if(cbor_value_advance(iter) != CborNoError) {
    return MessageDecoder::Status::Error;
  }

  dest_size = sizeof(ota->params.finalSha256);

  if (cbor::utils::copyCBORByteToArray(iter,
        ota->params.finalSha256,
        dest_size) == MessageDecoder::Status::Error ||
      dest_size != sizeof(ota->params.finalSha256)) {
    return MessageDecoder::Status::Error;
  }

  return MessageDecoder::Status::Complete;
}

static OtaUpdateCommandDecoder        otaUpdateCommandDecoder;
static ThingUpdateCommandDecoder      thingUpdateCommandDecoder;
static ThingDetachCommandDecoder      thingDetachCommandDecoder;
static LastValuesUpdateCommandDecoder lastValuesUpdateCommandDecoder;
static TimezoneCommandDownDecoder     timezoneCommandDownDecoder;

namespace cbor { namespace decoder { namespace iotcloud {
  void commandDecoders() {
    (void) otaUpdateCommandDecoder;
    (void) thingUpdateCommandDecoder;
    (void) thingDetachCommandDecoder;
    (void) lastValuesUpdateCommandDecoder;
    (void) timezoneCommandDownDecoder;
  }
}}}
