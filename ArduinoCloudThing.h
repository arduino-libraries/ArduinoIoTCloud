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

#ifndef ARDUINO_CLOUD_THING_H_
#define ARDUINO_CLOUD_THING_H_

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include "ArduinoCloudProperty.hpp"
#include "ArduinoCloudPropertyContainer.hpp"

#include "lib/LinkedList/LinkedList.h"

/******************************************************************************
 * CONSTANTS
 ******************************************************************************/

static bool ON  = true;
static bool OFF = false;

static long const ON_CHANGE = -1;
static long const SECONDS   = 1;
static long const MINUTES   = 60;
static long const HOURS     = 3600;
static long const DAYS      = 86400;

/******************************************************************************
 * SYNCHRONIZATION CALLBACKS
 ******************************************************************************/

#define MOST_RECENT_WINS onAutoSync
template<typename T>
void onAutoSync(ArduinoCloudProperty<T> property) {
  if( property.getLastCloudChangeTimestamp() > property.getLastLocalChangeTimestamp()){
    property.setPropertyValue(property.getCloudShadowValue());
    property.forceCallbackOnChange();
  }
}

#define CLOUD_WINS onForceCloudSync
template<typename T>
void onForceCloudSync(ArduinoCloudProperty<T> property) {
  property.setPropertyValue(property.getCloudShadowValue());
  property.forceCallbackOnChange();
}

#define DEVICE_WINS onForceDeviceSync // The device property value is already the correct one. The cloud property value will be synchronized at the next update cycle.
template<typename T>
void onForceDeviceSync(ArduinoCloudProperty<T> property) {
}

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class ArduinoCloudThing {

public:
  ArduinoCloudThing();

  void begin();

  ArduinoCloudProperty<bool>   & addPropertyReal(bool   & property, String const & name, Permission const permission);
  ArduinoCloudProperty<int>    & addPropertyReal(int    & property, String const & name, Permission const permission);
  ArduinoCloudProperty<float>  & addPropertyReal(float  & property, String const & name, Permission const permission);
  ArduinoCloudProperty<String> & addPropertyReal(String & property, String const & name, Permission const permission);

  // compute the timestamp of the local properties changes 
  int updateTimestampOnChangedProperties(unsigned long changeEventTime);
  /* encode return > 0 if a property has changed and encodes the changed properties in CBOR format into the provided buffer */
  int encode(uint8_t * data, size_t const size);
  int encode(ArduinoCloudPropertyContainer *property_cont, uint8_t * data, size_t const size);
  /* decode a CBOR payload received from the cloud */
  void decode(uint8_t const * const payload, size_t const length, bool syncMessage = false);


private:

  bool                          _status = OFF;
  char                          _uuid[33];
  ArduinoCloudPropertyContainer _property_cont;
  bool                          _syncMessage;

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

  template <typename T>
  class MapEntry {
  public:

    MapEntry() : _is_set(false) { }

    inline void    set  (T const & entry) { _entry = entry; _is_set = true; }
    inline T const get  () const { return _entry; }

    inline bool    isSet() const { return _is_set; }
    inline void    reset()       { _is_set = false; }

  private:

    T    _entry;
    bool _is_set;

  };

  class CborMapData {

  public:
    MapEntry<int>    base_version;
    MapEntry<String> base_name;
    MapEntry<double> base_time;
    MapEntry<String> name;
    MapEntry<float>  val;
    MapEntry<String> str_val;
    MapEntry<bool>   bool_val;
    MapEntry<double> time;
    
    void resetNotBase() {
      name.reset        ();
      val.reset         ();
      str_val.reset     ();
      bool_val.reset    ();
      time.reset        ();
    }
  };

  MapParserState handle_EnterMap     (CborValue * map_iter, CborValue * value_iter, CborMapData * map_data);
  MapParserState handle_MapKey       (CborValue * value_iter);
  MapParserState handle_UndefinedKey (CborValue * value_iter);
  MapParserState handle_BaseVersion  (CborValue * value_iter, CborMapData * map_data);
  MapParserState handle_BaseName     (CborValue * value_iter, CborMapData * map_data);
  MapParserState handle_BaseTime     (CborValue * value_iter, CborMapData * map_data);
  MapParserState handle_Name         (CborValue * value_iter, CborMapData * map_data);
  MapParserState handle_Value        (CborValue * value_iter, CborMapData * map_data);
  MapParserState handle_StringValue  (CborValue * value_iter, CborMapData * map_data);
  MapParserState handle_BooleanValue (CborValue * value_iter, CborMapData * map_data);
  MapParserState handle_Time         (CborValue * value_iter, CborMapData * map_data);
  MapParserState handle_LeaveMap     (CborValue * map_iter, CborValue * value_iter, CborMapData const * const map_data);

  static bool   ifNumericConvertToDouble    (CborValue * value_iter, double * numeric_val);
  static double convertCborHalfFloatToDouble(uint16_t const half_val);

};

#endif /* ARDUINO_CLOUD_THING_H_ */
