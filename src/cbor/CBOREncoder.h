/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef ARDUINO_CBOR_CBOR_ENCODER_H_
#define ARDUINO_CBOR_CBOR_ENCODER_H_

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include <Arduino.h>

#undef max
#undef min
#include <list>

#include "../property/PropertyContainer.h"

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class CBOREncoder
{

public:
    /* encode return > 0 if a property has changed and encodes the changed properties in CBOR format into the provided buffer */
    /* if lightPayload is true the integer identifier of the property will be encoded in the message instead of the property name in order to reduce the size of the message payload*/
    static CborError encode(PropertyContainer & property_container, uint8_t * data, size_t const size, int & bytes_encoded, unsigned int & current_property_index, bool lightPayload = false);

private:

  CBOREncoder() { }
  CBOREncoder(CborEncoder const &) { }

  enum class EncoderState
  {
    InitPropertyEncoder,
    OpenCBORContainer,
    TryAppend,
    OutOfMemory,
    SkipProperty,
    TrimAppend,
    CloseCBORContainer,
    TrimClose,
    FinishAppend,
    SendMessage,
    Error
  };

  struct PropertyContainerEncoder
  {
    PropertyContainerEncoder(PropertyContainer & _property_container, unsigned int & _current_property_index): property_container(_property_container), current_property_index(_current_property_index) { }
    PropertyContainer & property_container;
    unsigned int & current_property_index;
    int encoded_property_count;
    int checked_property_count;
    int encoded_property_limit;
    bool property_limit_active;
    CborEncoder encoder;
    CborEncoder arrayEncoder;
  };

  static EncoderState handle_InitPropertyEncoder(PropertyContainerEncoder & propertyEncoder);
  static EncoderState handle_OpenCBORContainer(PropertyContainerEncoder & propertyEncoder, uint8_t * data, size_t const size);
  static EncoderState handle_TryAppend(PropertyContainerEncoder & propertyEncoder, bool  & lightPayload);
  static EncoderState handle_OutOfMemory(PropertyContainerEncoder & propertyEncoder);
  static EncoderState handle_SkipProperty(PropertyContainerEncoder & propertyEncoder);
  static EncoderState handle_TrimAppend(PropertyContainerEncoder & propertyEncoder);
  static EncoderState handle_CloseCBORContainer(PropertyContainerEncoder & propertyEncoder);
  static EncoderState handle_TrimClose(PropertyContainerEncoder & propertyEncoder);
  static EncoderState handle_FinishAppend(PropertyContainerEncoder & propertyEncoder);
  static EncoderState handle_AdvancePropertyContainer(PropertyContainerEncoder & propertyEncoder);

};

#endif /* ARDUINO_CBOR_CBOR_ENCODER_H_ */
