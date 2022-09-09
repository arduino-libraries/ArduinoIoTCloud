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

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

CborError CBOREncoder::encode(PropertyContainer & property_container, uint8_t * data, size_t const size, int & bytes_encoded, unsigned int & current_property_index, bool lightPayload)
{
  EncoderState current_state = EncoderState::InitPropertyEncoder,
               next_state = EncoderState::InitPropertyEncoder;

  PropertyContainerEncoder propertyEncoder(property_container, current_property_index);

  while (current_state != EncoderState::SendMessage) {

    switch (current_state) {
      case EncoderState::InitPropertyEncoder      : next_state = handle_InitPropertyEncoder(propertyEncoder); break;
      case EncoderState::OpenCBORContainer        : next_state = handle_OpenCBORContainer(propertyEncoder, data, size); break;
      case EncoderState::TryAppend                : next_state = handle_TryAppend(propertyEncoder, lightPayload); break;
      case EncoderState::OutOfMemory              : next_state = handle_OutOfMemory(propertyEncoder); break;
      case EncoderState::SkipProperty             : next_state = handle_SkipProperty(propertyEncoder); break;
      case EncoderState::TrimAppend               : next_state = handle_TrimAppend(propertyEncoder); break;
      case EncoderState::CloseCBORContainer       : next_state = handle_CloseCBORContainer(propertyEncoder); break;
      case EncoderState::TrimClose                : next_state = handle_TrimClose(propertyEncoder); break;
      case EncoderState::FinishAppend             : next_state = handle_FinishAppend(propertyEncoder); break;
      case EncoderState::SendMessage              : /* Nothing to do */ break;
      case EncoderState::Error                    : return CborErrorInternalError; break;
    }

    current_state = next_state;
  }

  if (propertyEncoder.encoded_property_count > 0)
    bytes_encoded = cbor_encoder_get_buffer_size(&propertyEncoder.encoder, data);
  else
    bytes_encoded = 0;

  return CborNoError;
}

/******************************************************************************
   PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

CBOREncoder::EncoderState CBOREncoder::handle_InitPropertyEncoder(PropertyContainerEncoder & propertyEncoder)
{
  propertyEncoder.encoded_property_count = 0;
  propertyEncoder.checked_property_count = 0;
  propertyEncoder.encoded_property_limit = 0;
  propertyEncoder.property_limit_active  = false;
  return EncoderState::OpenCBORContainer;
}

CBOREncoder::EncoderState CBOREncoder::handle_OpenCBORContainer(PropertyContainerEncoder & propertyEncoder, uint8_t * data, size_t const size)
{
  propertyEncoder.encoded_property_count = 0;
  propertyEncoder.checked_property_count = 0;
  cbor_encoder_init(&propertyEncoder.encoder, data, size, 0);
  cbor_encoder_create_array(&propertyEncoder.encoder, &propertyEncoder.arrayEncoder, CborIndefiniteLength);
  return EncoderState::TryAppend;
}

CBOREncoder::EncoderState CBOREncoder::handle_TryAppend(PropertyContainerEncoder & propertyEncoder, bool  & lightPayload)
{
  /* Check if backing storage and cloud has diverged. Time interval may be elapsed or property may be changed
   * and if that's the case encode the property into the CBOR.
   */
  CborError error = CborNoError;
  PropertyContainer::iterator iter = propertyEncoder.property_container.begin();
  std::advance(iter, propertyEncoder.current_property_index);

  for(; iter != propertyEncoder.property_container.end(); iter++)
  {
    Property * p = * iter;

    if (p->shouldBeUpdated() && p->isReadableByCloud())
    {
      error = p->append(&propertyEncoder.arrayEncoder, lightPayload);
      if(error == CborNoError)
        propertyEncoder.encoded_property_count++;
    }
    if(error == CborNoError)
      propertyEncoder.checked_property_count++;

    bool const maximum_number_of_properties_reached = (propertyEncoder.encoded_property_count >= propertyEncoder.encoded_property_limit) && (propertyEncoder.property_limit_active == true);
    bool const cbor_encoder_error = (error != CborNoError);

    if (maximum_number_of_properties_reached || cbor_encoder_error)
      break;
  }

  if (CborErrorOutOfMemory == error)
    return EncoderState::OutOfMemory;
  else if (CborNoError == error)
    return EncoderState::CloseCBORContainer;
  else if (CborErrorSplitItems == error)
    return EncoderState::TrimAppend;
  else
    return EncoderState::Error;
}

CBOREncoder::EncoderState CBOREncoder::handle_OutOfMemory(PropertyContainerEncoder & propertyEncoder)
{
  if(propertyEncoder.encoded_property_count > 0)
    return EncoderState::CloseCBORContainer;
  else
    return EncoderState::SkipProperty;
}

CBOREncoder::EncoderState CBOREncoder::handle_SkipProperty(PropertyContainerEncoder & propertyEncoder)
{
  /* Better to skip this property otherwise we will stay blocked here. This happens only with a message property 
   * that doesn't fit into the CBOR buffer
   */
  propertyEncoder.current_property_index++;
  if(propertyEncoder.current_property_index >= propertyEncoder.property_container.size())
    propertyEncoder.current_property_index = 0;
  return EncoderState::Error;
}

CBOREncoder::EncoderState CBOREncoder::handle_TrimAppend(PropertyContainerEncoder & propertyEncoder)
{
  /* Trim the number of properties to be included in the next message to avoid multivalue property split */
  propertyEncoder.encoded_property_limit = propertyEncoder.encoded_property_count;
  propertyEncoder.property_limit_active = true;
  if(propertyEncoder.encoded_property_limit > 0)
    return EncoderState::OpenCBORContainer;
  else
    return EncoderState::Error;
}

CBOREncoder::EncoderState CBOREncoder::handle_CloseCBORContainer(PropertyContainerEncoder & propertyEncoder)
{
  CborError error = cbor_encoder_close_container(&propertyEncoder.encoder, &propertyEncoder.arrayEncoder);
  if (CborNoError != error)
    return EncoderState::TrimClose;
  else
    return EncoderState::FinishAppend;
}

CBOREncoder::EncoderState CBOREncoder::handle_TrimClose(PropertyContainerEncoder & propertyEncoder)
{
  /* Trim the number of properties to be included in the next message to avoid error closing container */
  propertyEncoder.encoded_property_limit = propertyEncoder.encoded_property_count - 1;
  propertyEncoder.property_limit_active = true;
  if(propertyEncoder.encoded_property_limit > 0)
    return EncoderState::OpenCBORContainer;
  else
    return EncoderState::Error;
}

CBOREncoder::EncoderState CBOREncoder::handle_FinishAppend(PropertyContainerEncoder & propertyEncoder)
{
  /* Restore property message limit to CBOR_ENCODER_NO_PROPERTIES_LIMIT */
  propertyEncoder.property_limit_active = false;

  /* The append process has been successful, so we don't need to try to send this properties set. Cleanup _has_been_appended_but_not_sended flag */
  PropertyContainer::iterator iter = propertyEncoder.property_container.begin();
  std::advance(iter, propertyEncoder.current_property_index);
  int num_appended_properties = 0;

  for(; iter != propertyEncoder.property_container.end(); iter++)
  {
    Property * p = * iter;
    if (num_appended_properties >= propertyEncoder.checked_property_count)
      break;

    p->appendCompleted();
    num_appended_properties++;
  }

  /* Advance property index for the next message */
  propertyEncoder.current_property_index += propertyEncoder.checked_property_count;

  if(propertyEncoder.current_property_index >= propertyEncoder.property_container.size())
    propertyEncoder.current_property_index = 0;

  return EncoderState::SendMessage;
}
