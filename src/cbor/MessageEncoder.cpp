/*
   This file is part of ArduinoIoTCloud.

   Copyright 2020 ARDUINO SA (http://www.arduino.cc/)

   This software is released under the GNU General Public License version 3,
   which covers the main part of arduino-cli.
   The terms of this license can be found at:
   https://www.gnu.org/licenses/gpl-3.0.en.html

   You can be released from the requirements of the above licenses by purchasing
   a commercial license. Buying such a license is mandatory if you want to modify or
   otherwise use the software for commercial activities involving the Arduino
   software without disclosing the source code of your own applications. To purchase
   a commercial license, send an email to license@arduino.cc.
*/

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include "CBOREncoder.h"

#undef max
#undef min
#include <algorithm>
#include <iterator>

#include "lib/tinycbor/cbor-lib.h"
#include "MessageEncoder.h"

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

Encoder::Status CBORMessageEncoder::encode(Message * message, uint8_t * data, size_t& len)
{
  EncoderState current_state = EncoderState::EncodeTag,
                 next_state  = EncoderState::Error;

  CborEncoder encoder;
  CborEncoder arrayEncoder;

  cbor_encoder_init(&encoder, data, len, 0);

  while (current_state != EncoderState::Complete) {

    switch (current_state) {
      case EncoderState::EncodeTag            : next_state = handle_EncodeTag(&encoder, message); break;
      case EncoderState::EncodeArray          : next_state = handle_EncodeArray(&encoder, &arrayEncoder, message); break;
      case EncoderState::EncodeParam          : next_state = handle_EncodeParam(&arrayEncoder, message); break;
      case EncoderState::CloseArray           : next_state = handle_CloseArray(&encoder, &arrayEncoder); break;
      case EncoderState::Complete             : /* Nothing to do */ break;
      case EncoderState::MessageNotSupported  :
      case EncoderState::Error                : return Encoder::Status::Error;
    }

    current_state = next_state;
  }

  len = cbor_encoder_get_buffer_size(&encoder, data);

  return Encoder::Status::Complete;
}

/******************************************************************************
    PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

CBORMessageEncoder::EncoderState CBORMessageEncoder::handle_EncodeTag(CborEncoder * encoder, Message * message)
{
  CborTag commandTag = toCBORCommandTag(message->id);
  if (commandTag == CBORCommandTag::CBORUnknownCmdTag16b ||
      commandTag == CBORCommandTag::CBORUnknownCmdTag32b ||
      commandTag == CBORCommandTag::CBORUnknownCmdTag64b ||
      cbor_encode_tag(encoder, commandTag) != CborNoError) {
    return EncoderState::Error;
  }

  return EncoderState::EncodeArray;
}

CBORMessageEncoder::EncoderState CBORMessageEncoder::handle_EncodeArray(CborEncoder * encoder, CborEncoder * array_encoder, Message * message)
{
  // Set array size based on the message id
  size_t array_size = 0;
  switch (message->id)
  {
  case CommandId::OtaBeginUpId:
    array_size = 1;
    break;
  case CommandId::ThingBeginCmdId:
    array_size = 1;
    break;
  case CommandId::DeviceBeginCmdId:
    array_size = 1;
    break;
  case CommandId::LastValuesBeginCmdId:
    break;
  case CommandId::OtaProgressCmdUpId:
    array_size = 4;
    break;
  case CommandId::TimezoneCommandUpId:
    break;
  default:
    return EncoderState::MessageNotSupported;
  }

  // Start an array with fixed width based on message type
  if (cbor_encoder_create_array(encoder, array_encoder, array_size) != CborNoError){
    return EncoderState::Error;
  }

  return EncoderState::EncodeParam;
}

CBORMessageEncoder::EncoderState CBORMessageEncoder::handle_EncodeParam(CborEncoder * array_encoder, Message * message)
{
  CborError error = CborNoError;
  switch (message->id)
  {
  case CommandId::OtaBeginUpId:
    error = CBORMessageEncoder::encodeOtaBeginUp(array_encoder, message);
    break;
  case CommandId::ThingBeginCmdId:
    error = CBORMessageEncoder::encodeThingBeginCmd(array_encoder, message);
    break;
  case CommandId::DeviceBeginCmdId:
    error = CBORMessageEncoder::encodeDeviceBeginCmd(array_encoder, message);
    break;
  case CommandId::LastValuesBeginCmdId:
    break;
  case CommandId::OtaProgressCmdUpId:
    error = CBORMessageEncoder::encodeOtaProgressCmdUp(array_encoder, message);
    break;
  case CommandId::TimezoneCommandUpId:
    break;
  default:
    return EncoderState::MessageNotSupported;
  }

  if (error != CborNoError) {
    return EncoderState::Error;
  }

  return EncoderState::CloseArray;
}

CBORMessageEncoder::EncoderState CBORMessageEncoder::handle_CloseArray(CborEncoder * encoder, CborEncoder * array_encoder)
{

  if (cbor_encoder_close_container(encoder, array_encoder) != CborNoError) {
    return EncoderState::Error;
  }

  return EncoderState::Complete;
}

// Message specific encoders
CborError CBORMessageEncoder::encodeOtaBeginUp(CborEncoder * array_encoder, Message * message)
{
  OtaBeginUp * otaBeginUp = (OtaBeginUp *) message;
  CHECK_CBOR(cbor_encode_byte_string(array_encoder, otaBeginUp->params.sha, SHA256_SIZE));
  return CborNoError;
}

CborError CBORMessageEncoder::encodeThingBeginCmd(CborEncoder * array_encoder, Message * message)
{
  ThingBeginCmd * thingBeginCmd = (ThingBeginCmd *) message;
  CHECK_CBOR(cbor_encode_text_stringz(array_encoder, thingBeginCmd->params.thing_id));
  return CborNoError;
}

CborError CBORMessageEncoder::encodeDeviceBeginCmd(CborEncoder * array_encoder, Message * message)
{
  DeviceBeginCmd * deviceBeginCmd = (DeviceBeginCmd *) message;
  CHECK_CBOR(cbor_encode_text_stringz(array_encoder, deviceBeginCmd->params.lib_version));
  return CborNoError;
}

CborError CBORMessageEncoder::encodeOtaProgressCmdUp(CborEncoder * array_encoder, Message * message)
{
  OtaProgressCmdUp * ota = (OtaProgressCmdUp *)message;
  CHECK_CBOR(cbor_encode_byte_string(array_encoder, ota->params.id, ID_SIZE));
  CHECK_CBOR(cbor_encode_simple_value(array_encoder, ota->params.state));
  CHECK_CBOR(cbor_encode_int(array_encoder, ota->params.state_data));
  CHECK_CBOR(cbor_encode_uint(array_encoder, ota->params.time));
  return CborNoError;
}

