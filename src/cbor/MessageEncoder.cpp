/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include "CBOREncoder.h"

#include "MessageEncoder.h"

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/


/******************************************************************************
    PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

Encoder::Status OtaBeginCommandEncoder::encode(CborEncoder* encoder, Message *msg) {
  OtaBeginUp * otaBeginUp = (OtaBeginUp*) msg;
  CborEncoder array_encoder;

  if(cbor_encoder_create_array(encoder, &array_encoder, 1) != CborNoError) {
    return Encoder::Status::Error;
  }

  if(cbor_encode_byte_string(&array_encoder, otaBeginUp->params.sha, SHA256_SIZE) != CborNoError) {
    return Encoder::Status::Error;
  }

  if(cbor_encoder_close_container(encoder, &array_encoder) != CborNoError) {
    return Encoder::Status::Error;
  }

  return Encoder::Status::Complete;
}

Encoder::Status ThingBeginCommandEncoder::encode(CborEncoder* encoder, Message *msg) {
  ThingBeginCmd * thingBeginCmd = (ThingBeginCmd*) msg;
  CborEncoder array_encoder;

  if(cbor_encoder_create_array(encoder, &array_encoder, 1) != CborNoError) {
      return Encoder::Status::Error;
  }

  if(cbor_encode_text_stringz(&array_encoder, thingBeginCmd->params.thing_id) != CborNoError) {
    return Encoder::Status::Error;
  }

  if(cbor_encoder_close_container(encoder, &array_encoder) != CborNoError) {
    return Encoder::Status::Error;
  }

  return Encoder::Status::Complete;
}

Encoder::Status LastValuesBeginCommandEncoder::encode(CborEncoder* encoder, Message *msg) {
  // This command contains no parameters, it contains just the id of the message
  // nothing to perform here
  // (void)(encoder);
  (void)(msg);
  CborEncoder array_encoder;

  // FIXME we are encoiding an empty array, this could be avoided
  if (cbor_encoder_create_array(encoder, &array_encoder, 0) != CborNoError){
    return Encoder::Status::Error;
  }

  if(cbor_encoder_close_container(encoder, &array_encoder) != CborNoError) {
    return Encoder::Status::Error;
  }

  return Encoder::Status::Complete;
}

Encoder::Status DeviceBeginCommandEncoder::encode(CborEncoder* encoder, Message *msg) {
  DeviceBeginCmd * deviceBeginCmd = (DeviceBeginCmd*) msg;
  CborEncoder array_encoder;

  if(cbor_encoder_create_array(encoder, &array_encoder, 1) != CborNoError) {
      return Encoder::Status::Error;
  }

  if(cbor_encode_text_stringz(&array_encoder, deviceBeginCmd->params.lib_version) != CborNoError) {
    return Encoder::Status::Error;
  }

  if(cbor_encoder_close_container(encoder, &array_encoder) != CborNoError) {
    return Encoder::Status::Error;
  }

  return Encoder::Status::Complete;
}

Encoder::Status OtaProgressCommandUpEncoder::encode(CborEncoder* encoder, Message *msg) {
  OtaProgressCmdUp * ota = (OtaProgressCmdUp*) msg;
  CborEncoder array_encoder;

  if(cbor_encoder_create_array(encoder, &array_encoder, 4) != CborNoError) {
      return Encoder::Status::Error;
  }

  if(cbor_encode_byte_string(&array_encoder, ota->params.id, ID_SIZE) != CborNoError) {
    return Encoder::Status::Error;
  }

  if(cbor_encode_simple_value(&array_encoder, ota->params.state) != CborNoError) {
    return Encoder::Status::Error;
  }

  if(cbor_encode_int(&array_encoder, ota->params.state_data) != CborNoError) {
    return Encoder::Status::Error;
  }

  if(cbor_encode_uint(&array_encoder, ota->params.time) != CborNoError) {
    return Encoder::Status::Error;
  }

  if(cbor_encoder_close_container(encoder, &array_encoder) != CborNoError) {
    return Encoder::Status::Error;
  }

  return Encoder::Status::Complete;
}

Encoder::Status TimezoneCommandUpEncoder::encode(CborEncoder* encoder, Message *msg) {
  // This command contains no parameters, it contains just the id of the message
  // nothing to perform here
  // (void)(encoder);
  (void)(msg);
  CborEncoder array_encoder;

  // FIXME we are encoiding an empty array, this could be avoided
  if (cbor_encoder_create_array(encoder, &array_encoder, 0) != CborNoError){
    return Encoder::Status::Error;
  }

  if(cbor_encoder_close_container(encoder, &array_encoder) != CborNoError) {
    return Encoder::Status::Error;
  }

  return Encoder::Status::Complete;
}

static OtaBeginCommandEncoder         otaBeginCommandEncoder;
static ThingBeginCommandEncoder       thingBeginCommandEncoder;
static LastValuesBeginCommandEncoder  lastValuesBeginCommandEncoder;
static DeviceBeginCommandEncoder      deviceBeginCommandEncoder;
static OtaProgressCommandUpEncoder    otaProgressCommandUpEncoder;
static TimezoneCommandUpEncoder       timezoneCommandUpEncoder;
