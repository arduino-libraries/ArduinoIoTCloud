//
// This file is part of ArduinoCloudThing
//
// Copyright 2019 ARDUINO SA (http://www.arduino.cc/)
//
// This software is released under the GNU General Public License version 3,
// which covers the main part of ArduinoCloudThing.
// The terms of this license can be found at:
// https://www.gnu.org/licenses/gpl-3.0.en.html
//
// You can be released from the requirements of the above licenses by purchasing
// a commercial license. Buying such a license is mandatory if you want to modify or
// otherwise use the software for commercial activities involving the Arduino
// software without disclosing the source code of your own applications. To purchase
// a commercial license, send an email to license@arduino.cc.
//

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

  void setMessage(__attribute__((unused))Message* msg) {} // FIXME not implemented yet
  Decoder::Status feed(__attribute__((unused)) uint8_t* buf, __attribute__((unused)) size_t &len) { return Decoder::Status::Error; } // FIXME not implemented yet

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
  ArrayParserState decodeThingBeginCmd(CborValue * param, Message * message);
  ArrayParserState decodeTimezoneCommandDown(CborValue * param, Message * message);
  ArrayParserState decodeLastValuesUpdateCmd(CborValue * param, Message * message);
  ArrayParserState decodeOtaUpdateCmdDown(CborValue * param, Message * message);

};

#endif /* ARDUINO_CBOR_MESSAGE_DECODER_H_ */
