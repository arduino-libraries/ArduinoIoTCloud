/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef ARDUINO_CBOR_MESSAGE_DECODER_H_
#define ARDUINO_CBOR_MESSAGE_DECODER_H_

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include <Arduino.h>

#undef max
#undef min
#include <list>

#include "CBOR.h"
#include "../interfaces/Decoder.h"
#include "lib/tinycbor/cbor-lib.h"

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class CBORMessageDecoder: public Decoder
{
public:
  CBORMessageDecoder() { }
  CBORMessageDecoder(CBORMessageDecoder const &) { }

  /* decode a CBOR payload received from the cloud */
  Decoder::Status decode(Message * msg, uint8_t const * const payload, size_t& length);

private:

  enum class DecoderState {
    Success,
    MessageNotSupported,
    MalformedMessage,
    Error
  };

  enum class ArrayParserState {
    EnterArray,
    ParseParam,
    LeaveArray,
    Complete,
    Error,
    MessageNotSupported
  };

  ArrayParserState handle_EnterArray(CborValue * main_iter, CborValue * array_iter);
  ArrayParserState handle_Param(CborValue * param, Message * message);
  ArrayParserState handle_LeaveArray(CborValue * main_iter, CborValue * array_iter);

  bool   ifNumericConvertToDouble(CborValue * value_iter, double * numeric_val);
  double convertCborHalfFloatToDouble(uint16_t const half_val);

  // Message specific decoders
  ArrayParserState decodeThingUpdateCmd(CborValue * param, Message * message);
  ArrayParserState decodeTimezoneCommandDown(CborValue * param, Message * message);
  ArrayParserState decodeLastValuesUpdateCmd(CborValue * param, Message * message);
  ArrayParserState decodeOtaUpdateCmdDown(CborValue * param, Message * message);

};

#endif /* ARDUINO_CBOR_MESSAGE_DECODER_H_ */
