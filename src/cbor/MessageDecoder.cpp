//
// This file is part of CBORDecoder
//
// Copyright 2019 ARDUINO SA (http://www.arduino.cc/)
//
// This software is released under the GNU General Public License version 3,
// which covers the main part of CBORDecoder.
// The terms of this license can be found at:
// https://www.gnu.org/licenses/gpl-3.0.en.html
//
// You can be released from the requirements of the above licenses by purchasing
// a commercial license. Buying such a license is mandatory if you want to modify or
// otherwise use the software for commercial activities involving the Arduino
// software without disclosing the source code of your own applications. To purchase
// a commercial license, send an email to license@arduino.cc.
//

/******************************************************************************
    INCLUDE
 ******************************************************************************/

#include <Arduino.h>

#undef max
#undef min
#include <algorithm>

#include "MessageDecoder.h"
#include <AIoTC_Config.h>

/******************************************************************************
    PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

Decoder::Status CBORMessageDecoder::decode(Message * message, uint8_t const * const payload, size_t& length)
{
  CborValue main_iter, array_iter;
  CborTag tag;
  CborParser parser;

  if (cbor_parser_init(payload, length, 0, &parser, &main_iter) != CborNoError)
    return Decoder::Status::Error;

  if (main_iter.type != CborTagType)
    return Decoder::Status::Error;

  if (cbor_value_get_tag(&main_iter, &tag) == CborNoError) {
    message->id = toCommandId(CBORCommandTag(tag));
  }

  if (cbor_value_advance(&main_iter) != CborNoError) {
    return Decoder::Status::Error;
  }

  ArrayParserState current_state  = ArrayParserState::EnterArray,
                   next_state     = ArrayParserState::Error;

  while (current_state != ArrayParserState::Complete) {
    switch (current_state) {
      case ArrayParserState::EnterArray           : next_state = handle_EnterArray(&main_iter, &array_iter); break;
      case ArrayParserState::ParseParam           : next_state = handle_Param(&array_iter, message); break;
      case ArrayParserState::LeaveArray           : next_state = handle_LeaveArray(&main_iter, &array_iter); break;
      case ArrayParserState::Complete             : return Decoder::Status::Complete;
      case ArrayParserState::MessageNotSupported  : return Decoder::Status::Error;
      case ArrayParserState::Error                : return Decoder::Status::Error;
    }

    current_state = next_state;
  }

  return Decoder::Status::Complete;
}

/******************************************************************************
    PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

bool copyCBORStringToArray(CborValue * param, char * dest, size_t dest_size) {
  if (!cbor_value_is_text_string(param)) {
    return false;
  }

  // NOTE: keep in mind that _cbor_value_copy_string tries to put a \0 at the end of the string
  if(_cbor_value_copy_string(param, dest, &dest_size, NULL) != CborNoError) {
    return false;
  }

  return true;
}

// FIXME dest_size should be also returned, the copied byte array can have a different size from the starting one
// for the time being we need this on SHA256 only
bool copyCBORByteToArray(CborValue * param, uint8_t * dest, size_t dest_size) {
  if (!cbor_value_is_byte_string(param)) {
    return false;
  }

  // NOTE: keep in mind that _cbor_value_copy_string tries to put a \0 at the end of the string
  if(_cbor_value_copy_string(param, dest, &dest_size, NULL) != CborNoError) {

    return false;
  }

  return true;
}

CBORMessageDecoder::ArrayParserState CBORMessageDecoder::handle_EnterArray(CborValue * main_iter, CborValue * array_iter) {
  ArrayParserState next_state = ArrayParserState::Error;
  if (cbor_value_get_type(main_iter) == CborArrayType) {
    if (cbor_value_enter_container(main_iter, array_iter) == CborNoError) {
      next_state = ArrayParserState::ParseParam;
    }
  }

  return next_state;
}

CBORMessageDecoder::ArrayParserState CBORMessageDecoder::handle_LeaveArray(CborValue * main_iter, CborValue * array_iter) {
  // Advance to the next parameter (the last one in the array)
  if (cbor_value_advance(array_iter) != CborNoError) {
    return ArrayParserState::Error;
  }
  // Leave the array
  if (cbor_value_leave_container(main_iter, array_iter) != CborNoError) {
    return ArrayParserState::Error;
  }
  return ArrayParserState::Complete;
}

/******************************************************************************
    MESSAGE DECODE FUNCTIONS
 ******************************************************************************/

CBORMessageDecoder::ArrayParserState CBORMessageDecoder::decodeThingBeginCmd(CborValue * param, Message * message) {
  ThingBeginCmd * thingCommand = (ThingBeginCmd *) message;

  // Message is composed of a single parameter, a string (thing_id)
  if (!copyCBORStringToArray(param, thingCommand->params.thing_id, sizeof(thingCommand->params.thing_id))) {
    return ArrayParserState::Error;
  }

  return ArrayParserState::LeaveArray;
}

CBORMessageDecoder::ArrayParserState CBORMessageDecoder::decodeTimezoneCommandDown(CborValue * param, Message * message) {
  TimezoneCommandDown * setTz = (TimezoneCommandDown *) message;

  // Message is composed of 2 parameters, offset 32-bit signed integer and until 32-bit unsigned integer
  // Get offset
  if (cbor_value_is_integer(param)) {
    int64_t val = 0;
    if (cbor_value_get_int64(param, &val) == CborNoError) {
      setTz->params.offset = static_cast<int32_t>(val);
    }
  }

  // Next
  if (cbor_value_advance(param) != CborNoError) {
    return ArrayParserState::Error;
  }

  // Get until
  if (cbor_value_is_integer(param)) {
    uint64_t val = 0;
    if (cbor_value_get_uint64(param, &val) == CborNoError) {
      setTz->params.until = static_cast<uint32_t>(val);
    }
  }

  return ArrayParserState::LeaveArray;
}

CBORMessageDecoder::ArrayParserState CBORMessageDecoder::decodeLastValuesUpdateCmd(CborValue * param, Message * message) {
  LastValuesUpdateCmd * setLv = (LastValuesUpdateCmd *) message;

  // Message is composed by a single parameter, a variable length byte array.
  if (cbor_value_is_byte_string(param)) {
    // Cortex M0 is not able to assign a value to pointed memory that is not 32bit aligned
    // we use a support variable to cope with that
    size_t s;
    if (cbor_value_dup_byte_string(param, &setLv->params.last_values, &s, NULL) != CborNoError) {
      return ArrayParserState::Error;
    }

    setLv->params.length = s;
  }

  return ArrayParserState::LeaveArray;
}

CBORMessageDecoder::ArrayParserState CBORMessageDecoder::decodeOtaUpdateCmdDown(CborValue * param, Message * message) {
  OtaUpdateCmdDown * ota = (OtaUpdateCmdDown *) message;

  // Message is composed 4 parameters: id, url, initialSha, finalSha
  if (!copyCBORByteToArray(param, ota->params.id, sizeof(ota->params.id))) {
    return ArrayParserState::Error;
  }

  if (cbor_value_advance(param) != CborNoError) {
    return ArrayParserState::Error;
  }

  if (!copyCBORStringToArray(param, ota->params.url, sizeof(ota->params.url))) {
    return ArrayParserState::Error;
  }

  if (cbor_value_advance(param) != CborNoError) {
    return ArrayParserState::Error;
  }

  if (!copyCBORByteToArray(param, ota->params.initialSha256, sizeof(ota->params.initialSha256))) {
    return ArrayParserState::Error;
  }

  if (cbor_value_advance(param) != CborNoError) {
    return ArrayParserState::Error;
  }

  if (!copyCBORByteToArray(param, ota->params.finalSha256, sizeof(ota->params.finalSha256))) {
    return ArrayParserState::Error;
  }

  return ArrayParserState::LeaveArray;
}

CBORMessageDecoder::ArrayParserState CBORMessageDecoder::handle_Param(CborValue * param, Message * message) {

  switch (message->id)
  {
  case CommandId::ThingUpdateCmdId:
    return CBORMessageDecoder::decodeThingBeginCmd(param, message);

  case CommandId::TimezoneCommandDownId:
    return CBORMessageDecoder::decodeTimezoneCommandDown(param, message);

  case CommandId::LastValuesUpdateCmdId:
    return CBORMessageDecoder::decodeLastValuesUpdateCmd(param, message);

  case CommandId::OtaUpdateCmdDownId:
    return CBORMessageDecoder::decodeOtaUpdateCmdDown(param, message);

  default:
    return ArrayParserState::MessageNotSupported;
  }

  return ArrayParserState::LeaveArray;
}
