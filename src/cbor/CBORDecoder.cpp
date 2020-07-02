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

#include "CBORDecoder.h"

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

void CBORDecoder::decode(PropertyContainer & property_container, uint8_t const * const payload, size_t const length, bool isSyncMessage)
{
  CborValue array_iter, map_iter,value_iter;
  CborParser parser;
  CborMapData map_data;
  std::list<CborMapData> map_data_list; /* List of map data that will hold all the attributes of a property */
  String current_property_name; /* Current property name during decoding: use to look for a new property in the senml value array */
  unsigned long current_property_base_time{0}, current_property_time{0};

  if (cbor_parser_init(payload, length, 0, &parser, &array_iter) != CborNoError)
    return;

  if (array_iter.type != CborArrayType)
    return;

  if (cbor_value_enter_container(&array_iter, &map_iter) != CborNoError)
    return;

  MapParserState current_state = MapParserState::EnterMap,
                 next_state = MapParserState::Error;

  while (current_state != MapParserState::Complete) {

    switch (current_state) {
      case MapParserState::EnterMap     : next_state = handle_EnterMap(&map_iter, &value_iter); break;
      case MapParserState::MapKey       : next_state = handle_MapKey(&value_iter); break;
      case MapParserState::UndefinedKey : next_state = handle_UndefinedKey(&value_iter); break;
      case MapParserState::BaseVersion  : next_state = handle_BaseVersion(&value_iter, map_data); break;
      case MapParserState::BaseName     : next_state = handle_BaseName(&value_iter, map_data); break;
      case MapParserState::BaseTime     : next_state = handle_BaseTime(&value_iter, map_data); break;
      case MapParserState::Time         : next_state = handle_Time(&value_iter, map_data); break;
      case MapParserState::Name         : next_state = handle_Name(&value_iter, map_data, property_container); break;
      case MapParserState::Value        : next_state = handle_Value(&value_iter, map_data); break;
      case MapParserState::StringValue  : next_state = handle_StringValue(&value_iter, map_data); break;
      case MapParserState::BooleanValue : next_state = handle_BooleanValue(&value_iter, map_data); break;
      case MapParserState::LeaveMap     : next_state = handle_LeaveMap(&map_iter, &value_iter, map_data, property_container, current_property_name, current_property_base_time, current_property_time, isSyncMessage, map_data_list); break;
      case MapParserState::Complete     : /* Nothing to do */ break;
      case MapParserState::Error        : return; break;
    }

    current_state = next_state;
  }
}

/******************************************************************************
   PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

CBORDecoder::MapParserState CBORDecoder::handle_EnterMap(CborValue * map_iter, CborValue * value_iter) {
  MapParserState next_state = MapParserState::Error;

  if (cbor_value_get_type(map_iter) == CborMapType) {
    if (cbor_value_enter_container(map_iter, value_iter) == CborNoError) {
      next_state = MapParserState::MapKey;
    }
  }

  return next_state;
}

CBORDecoder::MapParserState CBORDecoder::handle_MapKey(CborValue * value_iter) {
  MapParserState next_state = MapParserState::Error;

  if (cbor_value_at_end(value_iter)) {
    next_state = MapParserState::LeaveMap;
  }
  /* The Map use the CBOR Label (protocol V2)
     Example [{0: "temperature", 2: 25}]
  */
  else if (cbor_value_is_integer(value_iter)) {
    int val = 0;
    if (cbor_value_get_int(value_iter, &val) == CborNoError) {
      if (cbor_value_advance(value_iter) == CborNoError) {
        if (val == static_cast<int>(CborIntegerMapKey::Name)) {
          next_state = MapParserState::Name;
        } else if (val == static_cast<int>(CborIntegerMapKey::BaseVersion)) {
          next_state = MapParserState::BaseVersion;
        } else if (val == static_cast<int>(CborIntegerMapKey::BaseName)) {
          next_state = MapParserState::BaseName;
        } else if (val == static_cast<int>(CborIntegerMapKey::BaseTime)) {
          next_state = MapParserState::BaseTime;
        } else if (val == static_cast<int>(CborIntegerMapKey::Value)) {
          next_state = MapParserState::Value;
        } else if (val == static_cast<int>(CborIntegerMapKey::StringValue)) {
          next_state = MapParserState::StringValue;
        } else if (val == static_cast<int>(CborIntegerMapKey::BooleanValue)) {
          next_state = MapParserState::BooleanValue;
        } else if (val == static_cast<int>(CborIntegerMapKey::Time)) {
          next_state = MapParserState::Time;
        } else {
          next_state = MapParserState::UndefinedKey;
        }
      }
    }
  }

  return next_state;
}

CBORDecoder::MapParserState CBORDecoder::handle_UndefinedKey(CborValue * value_iter) {
  MapParserState next_state = MapParserState::Error;

  if (cbor_value_advance(value_iter) == CborNoError) {
    next_state = MapParserState::MapKey;
  }

  return next_state;
}

CBORDecoder::MapParserState CBORDecoder::handle_BaseVersion(CborValue * value_iter, CborMapData & map_data) {
  MapParserState next_state = MapParserState::Error;

  if (cbor_value_is_integer(value_iter)) {
    int val = 0;
    if (cbor_value_get_int(value_iter, &val) == CborNoError) {
      map_data.base_version.set(val);

      if (cbor_value_advance(value_iter) == CborNoError) {
        next_state = MapParserState::MapKey;
      }
    }
  }

  return next_state;
}

CBORDecoder::MapParserState CBORDecoder::handle_BaseName(CborValue * value_iter, CborMapData & map_data) {
  MapParserState next_state = MapParserState::Error;

  if (cbor_value_is_text_string(value_iter)) {
    char * val      = 0;
    size_t val_size = 0;
    if (cbor_value_dup_text_string(value_iter, &val, &val_size, value_iter) == CborNoError) {
      map_data.base_name.set(String(val));
      free(val);
      next_state = MapParserState::MapKey;
    }
  }

  return next_state;
}

CBORDecoder::MapParserState CBORDecoder::handle_BaseTime(CborValue * value_iter, CborMapData & map_data) {
  MapParserState next_state = MapParserState::Error;

  double val = 0.0;
  if (ifNumericConvertToDouble(value_iter, &val)) {
    map_data.base_time.set(val);

    if (cbor_value_advance(value_iter) == CborNoError) {
      next_state = MapParserState::MapKey;
    }
  }

  return next_state;
}

CBORDecoder::MapParserState CBORDecoder::handle_Name(CborValue * value_iter, CborMapData & map_data, PropertyContainer & property_container) {
  MapParserState next_state = MapParserState::Error;

  if (cbor_value_is_text_string(value_iter)) {
    // if the value in the cbor message is a string, it corresponds to the name of the property to be updated (int the form [property_name]:[attribute_name])
    char * val      = nullptr;
    size_t val_size = 0;
    if (cbor_value_dup_text_string(value_iter, &val, &val_size, value_iter) == CborNoError) {
      String name = val;
      free(val);
      map_data.name.set(name);
      int colonPos = name.indexOf(":");
      String attribute_name = "";
      if (colonPos != -1) {
        attribute_name = name.substring(colonPos + 1);
      }
      map_data.attribute_name.set(attribute_name);
      next_state = MapParserState::MapKey;
    }
  } else if (cbor_value_is_integer(value_iter)) {
    // if the value in the cbor message is an integer, a light payload has been used and an integer identifier should be decode in order to retrieve the corresponding property and attribute name to be updated
    int val = 0;
    if (cbor_value_get_int(value_iter, &val) == CborNoError) {
      map_data.light_payload.set(true);
      map_data.name_identifier.set(val & 255);
      map_data.attribute_identifier.set(val >> 8);
      map_data.light_payload.set(true);
      String name = getPropertyNameByIdentifier(property_container, val);
      map_data.name.set(name);


      if (cbor_value_advance(value_iter) == CborNoError) {
        next_state = MapParserState::MapKey;
      }
    }
  }



  return next_state;
}

CBORDecoder::MapParserState CBORDecoder::handle_Value(CborValue * value_iter, CborMapData & map_data) {
  MapParserState next_state = MapParserState::Error;

  double val = 0.0;
  if (ifNumericConvertToDouble(value_iter, &val)) {
    map_data.val.set(val);

    if (cbor_value_advance(value_iter) == CborNoError) {
      next_state = MapParserState::MapKey;
    }
  }

  return next_state;
}

CBORDecoder::MapParserState CBORDecoder::handle_StringValue(CborValue * value_iter, CborMapData & map_data) {
  MapParserState next_state = MapParserState::Error;

  if (cbor_value_is_text_string(value_iter)) {
    char * val      = 0;
    size_t val_size = 0;
    if (cbor_value_dup_text_string(value_iter, &val, &val_size, value_iter) == CborNoError) {
      map_data.str_val.set(String(val));
      free(val);
      next_state = MapParserState::MapKey;
    }
  }

  return next_state;
}

CBORDecoder::MapParserState CBORDecoder::handle_BooleanValue(CborValue * value_iter, CborMapData & map_data) {
  MapParserState next_state = MapParserState::Error;

  bool val = false;
  if (cbor_value_get_boolean(value_iter, &val) == CborNoError) {
    map_data.bool_val.set(val);

    if (cbor_value_advance(value_iter) == CborNoError) {
      next_state = MapParserState::MapKey;
    }
  }

  return next_state;
}

CBORDecoder::MapParserState CBORDecoder::handle_Time(CborValue * value_iter, CborMapData & map_data) {
  MapParserState next_state = MapParserState::Error;

  double val = 0.0;
  if (ifNumericConvertToDouble(value_iter, &val)) {
    map_data.time.set(val);

    if (cbor_value_advance(value_iter) == CborNoError) {
      next_state = MapParserState::MapKey;
    }
  }

  return next_state;
}

CBORDecoder::MapParserState CBORDecoder::handle_LeaveMap(CborValue * map_iter, CborValue * value_iter, CborMapData & map_data, PropertyContainer & property_container, String & current_property_name, unsigned long & current_property_base_time, unsigned long & current_property_time, bool const is_sync_message, std::list<CborMapData> & map_data_list) {
  MapParserState next_state = MapParserState::Error;
  if (map_data.name.isSet()) {
    String propertyName;
    int colonPos = map_data.name.get().indexOf(":");
    if (colonPos != -1) {
      propertyName = map_data.name.get().substring(0, colonPos);
    } else {
      propertyName = map_data.name.get();
    }

    if (current_property_name != "" && propertyName != current_property_name) {
      /* Update the property containers depending on the parsed data */
      updateProperty(property_container, current_property_name, current_property_base_time + current_property_time, is_sync_message, &map_data_list);
      /* Reset current property data */
      map_data_list.clear();
      current_property_base_time = 0;
      current_property_time = 0;
    }
    /* Compute the cloud change event baseTime and Time */
    if (map_data.base_time.isSet()) {
      current_property_base_time = (unsigned long)(map_data.base_time.get());
    }
    if (map_data.time.isSet() && (map_data.time.get() > current_property_time)) {
      current_property_time = (unsigned long)map_data.time.get();
    }
    map_data_list.push_back(map_data);
    current_property_name = propertyName;
  }

  /* Transition into the next map if available, otherwise finish */
  if (cbor_value_leave_container(map_iter, value_iter) == CborNoError) {
    if (!cbor_value_at_end(map_iter)) {
      next_state = MapParserState::EnterMap;
    } else {
      /* Update the property containers depending on the parsed data */
      updateProperty(property_container, current_property_name, current_property_base_time + current_property_time, is_sync_message, &map_data_list);
      /* Reset last property data */
      map_data_list.clear();
      next_state = MapParserState::Complete;
    }
  }

  return next_state;
}

bool CBORDecoder::ifNumericConvertToDouble(CborValue * value_iter, double * numeric_val) {

  if (cbor_value_is_integer(value_iter)) {
    int64_t val = 0;
    if (cbor_value_get_int64(value_iter, &val) == CborNoError) {
      *numeric_val = static_cast<double>(val);
      return true;
    }
  } else if (cbor_value_is_double(value_iter)) {
    double val = 0.0;
    if (cbor_value_get_double(value_iter, &val) == CborNoError) {
      *numeric_val = val;
      return true;
    }
  } else if (cbor_value_is_float(value_iter)) {
    float val = 0.0;
    if (cbor_value_get_float(value_iter, &val) == CborNoError) {
      *numeric_val = static_cast<double>(val);
      return true;
    }
  } else if (cbor_value_is_half_float(value_iter)) {
    uint16_t val = 0;
    if (cbor_value_get_half_float(value_iter, &val) == CborNoError) {
      *numeric_val = static_cast<double>(convertCborHalfFloatToDouble(val));
      return true;
    }
  }

  return false;
}

/* Source Idea from https://tools.ietf.org/html/rfc7049 : Page: 50 */
double CBORDecoder::convertCborHalfFloatToDouble(uint16_t const half_val) {
  int exp = (half_val >> 10) & 0x1f;
  int mant = half_val & 0x3ff;
  double val;
  if (exp == 0) {
    val = ldexp(mant, -24);
  } else if (exp != 31) {
    val = ldexp(mant + 1024, exp - 25);
  } else {
    val = mant == 0 ? INFINITY : NAN;
  }
  return half_val & 0x8000 ? -val : val;
}
