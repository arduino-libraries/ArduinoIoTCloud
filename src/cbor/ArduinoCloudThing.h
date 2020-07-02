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
   INCLUDE
 ******************************************************************************/

#undef max
#undef min
#include <list>

#include "../property/PropertyContainer.h"

#include "../property/types/CloudBool.h"
#include "../property/types/CloudFloat.h"
#include "../property/types/CloudInt.h"
#include "../property/types/CloudString.h"
#include "../property/types/CloudLocation.h"
#include "../property/types/CloudColor.h"
#include "../property/types/CloudWrapperBase.h"

#include "../property/types/automation/CloudColoredLight.h"
#include "../property/types/automation/CloudContactSensor.h"
#include "../property/types/automation/CloudDimmedLight.h"
#include "../property/types/automation/CloudLight.h"
#include "../property/types/automation/CloudMotionSensor.h"
#include "../property/types/automation/CloudSmartPlug.h"
#include "../property/types/automation/CloudSwitch.h"
#include "../property/types/automation/CloudTemperature.h"
#include "../property/types/automation/CloudTelevision.h"

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class ArduinoCloudThing {

  public:
    ArduinoCloudThing();

    void begin(PropertyContainer * property_container);

    /* decode a CBOR payload received from the cloud */
    void decode(uint8_t const * const payload, size_t const length, bool isSyncMessage = false);


  private:
    PropertyContainer * _property_container;
    /* Indicates the if the message received to be decoded is a response to the getLastValues inquiry */
    bool                                 _isSyncMessage;
    /* List of map data that will hold all the attributes of a property */
    std::list<CborMapData *>             _map_data_list;
    /* Current property name during decoding: use to look for a new property in the senml value array */
    String                               _currentPropertyName;
    unsigned long                        _currentPropertyBaseTime,
             _currentPropertyTime;

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

    MapParserState handle_EnterMap(CborValue * map_iter, CborValue * value_iter, CborMapData **map_data);
    MapParserState handle_MapKey(CborValue * value_iter);
    MapParserState handle_UndefinedKey(CborValue * value_iter);
    MapParserState handle_BaseVersion(CborValue * value_iter, CborMapData * map_data);
    MapParserState handle_BaseName(CborValue * value_iter, CborMapData * map_data);
    MapParserState handle_BaseTime(CborValue * value_iter, CborMapData * map_data);
    MapParserState handle_Name(CborValue * value_iter, CborMapData * map_data);
    MapParserState handle_Value(CborValue * value_iter, CborMapData * map_data);
    MapParserState handle_StringValue(CborValue * value_iter, CborMapData * map_data);
    MapParserState handle_BooleanValue(CborValue * value_iter, CborMapData * map_data);
    MapParserState handle_Time(CborValue * value_iter, CborMapData * map_data);
    MapParserState handle_LeaveMap(CborValue * map_iter, CborValue * value_iter, CborMapData * map_data);

    static bool   ifNumericConvertToDouble(CborValue * value_iter, double * numeric_val);
    static double convertCborHalfFloatToDouble(uint16_t const half_val);
    void freeMapDataList(std::list<CborMapData *> * map_data_list);

};

#endif /* ARDUINO_CLOUD_THING_H_ */
