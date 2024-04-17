/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef ARDUINO_CBOR_MESSAGE_ENCODER_H_
#define ARDUINO_CBOR_MESSAGE_ENCODER_H_

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <Arduino.h>

#undef max
#undef min
#include <list>

#include "CBOR.h"
#include "../interfaces/Encoder.h"
#include "lib/tinycbor/cbor-lib.h"

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class CBORMessageEncoder: public Encoder
{

public:
  CBORMessageEncoder() { }
  CBORMessageEncoder(CborEncoder const &) { }
  Encoder::Status encode(Message * message, uint8_t * data, size_t& len);

private:

  enum class EncoderState
  {
    EncodeTag,
    EncodeArray,
    EncodeParam,
    CloseArray,
    MessageNotSupported,
    Complete,
    Error
  };

  EncoderState handle_EncodeTag(CborEncoder * encoder, Message * message);
  EncoderState handle_EncodeArray(CborEncoder * encoder, CborEncoder * array_encoder, Message * message);
  EncoderState handle_EncodeParam(CborEncoder * array_encoder, Message * message);
  EncoderState handle_CloseArray(CborEncoder * encoder, CborEncoder * array_encoder);

  // Message specific encoders
  CborError encodeThingBeginCmd(CborEncoder * array_encoder, Message * message);
  CborError encodeOtaBeginUp(CborEncoder * array_encoder, Message * message);
  CborError encodeDeviceBeginCmd(CborEncoder * array_encoder, Message * message);
  CborError encodeOtaProgressCmdUp(CborEncoder * array_encoder, Message * message);
};

#endif /* ARDUINO_CBOR_MESSAGE_ENCODER_H_ */
