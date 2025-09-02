/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2019 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef ARDUINO_CBOR_CBOR_DECODER_H_
#define ARDUINO_CBOR_CBOR_DECODER_H_

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

class CBORDecoder
{

public:

  /* decode a CBOR payload received from the cloud */
  static void decode(PropertyContainer & property_container, uint8_t const * const payload, size_t const length, bool isSyncMessage = false);


private:

  CBORDecoder() { }
  CBORDecoder(CBORDecoder const &) { }

  enum class MapParserState {
    EnterMap,
    MapKey,
    UndefinedKey,
    BaseVersion,
    BaseName,
    BaseTime,
    Name,
    Value,
    StringValue,
    BooleanValue,
    Time,
    LeaveMap,
    Complete,
    Error
  };

  static MapParserState handle_EnterMap(CborValue * map_iter, CborValue * value_iter);
  static MapParserState handle_MapKey(CborValue * value_iter);
  static MapParserState handle_UndefinedKey(CborValue * value_iter);
  static MapParserState handle_BaseVersion(CborValue * value_iter, CborMapData & map_data);
  static MapParserState handle_BaseName(CborValue * value_iter, CborMapData & map_data);
  static MapParserState handle_BaseTime(CborValue * value_iter, CborMapData & map_data);
  static MapParserState handle_Name(CborValue * value_iter, CborMapData & map_data, PropertyContainer & property_container);
  static MapParserState handle_Value(CborValue * value_iter, CborMapData & map_data);
  static MapParserState handle_StringValue(CborValue * value_iter, CborMapData & map_data);
  static MapParserState handle_BooleanValue(CborValue * value_iter, CborMapData & map_data);
  static MapParserState handle_Time(CborValue * value_iter, CborMapData & map_data);
  static MapParserState handle_LeaveMap(CborValue * map_iter, CborValue * value_iter, CborMapData & map_data, PropertyContainer & property_container, String & current_property_name, unsigned long & current_property_base_time, unsigned long & current_property_time, bool const is_sync_message, std::list<CborMapData> & map_data_list);

  static bool   ifNumericConvertToDouble(CborValue * value_iter, double * numeric_val);
  static double convertCborHalfFloatToDouble(uint16_t const half_val);

};

#endif /* ARDUINO_CBOR_CBOR_DECODER_H_ */
