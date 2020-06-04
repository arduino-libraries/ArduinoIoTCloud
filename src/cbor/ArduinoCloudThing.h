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

#include "ArduinoCloudProperty.h"
#include "lib/LinkedList/LinkedList.h"
#include "types/CloudBool.h"
#include "types/CloudFloat.h"
#include "types/CloudInt.h"
#include "types/CloudString.h"
#include "types/CloudLocation.h"
#include "types/CloudColor.h"
#include "types/CloudWrapperBase.h"

#include "types/automation/CloudColoredLight.h"
#include "types/automation/CloudContactSensor.h"
#include "types/automation/CloudDimmedLight.h"
#include "types/automation/CloudLight.h"
#include "types/automation/CloudMotionSensor.h"
#include "types/automation/CloudSmartPlug.h"
#include "types/automation/CloudSwitch.h"
#include "types/automation/CloudTemperature.h"
#include "types/automation/CloudTelevision.h"


/******************************************************************************
   CONSTANTS
 ******************************************************************************/

static bool const ON  = true;
static bool const OFF = false;

static long const ON_CHANGE = -1;
static long const SECONDS   = 1;
static long const MINUTES   = 60;
static long const HOURS     = 3600;
static long const DAYS      = 86400;

/******************************************************************************
   SYNCHRONIZATION CALLBACKS
 ******************************************************************************/

void onAutoSync(ArduinoCloudProperty & property);
#define MOST_RECENT_WINS onAutoSync
void onForceCloudSync(ArduinoCloudProperty & property);
#define CLOUD_WINS onForceCloudSync
void onForceDeviceSync(ArduinoCloudProperty & property);
#define DEVICE_WINS onForceDeviceSync // The device property value is already the correct one. The cloud property value will be synchronized at the next update cycle.

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class ArduinoCloudThing {

  public:
    ArduinoCloudThing();

    void begin();
    void registerGetTimeCallbackFunc(GetTimeCallbackFunc func);
    //if propertyIdentifier is different from -1, an integer identifier is associated to the added property to be use instead of the property name when the parameter lightPayload is true in the encode method
    ArduinoCloudProperty   & addPropertyReal(ArduinoCloudProperty   & property, String const & name, Permission const permission, int propertyIdentifier = -1);

    /* encode return > 0 if a property has changed and encodes the changed properties in CBOR format into the provided buffer */
    /* if lightPayload is true the integer identifier of the property will be encoded in the message instead of the property name in order to reduce the size of the message payload*/
    int encode(uint8_t * data, size_t const size, bool lightPayload = false);
    /* decode a CBOR payload received from the cloud */
    void decode(uint8_t const * const payload, size_t const length, bool isSyncMessage = false);

    bool isPropertyInContainer(String const & name);
    int appendChangedProperties(CborEncoder * arrayEncoder, bool lightPayload);
    void updateTimestampOnLocallyChangedProperties();
    void updateProperty(String propertyName, unsigned long cloudChangeEventTime);
    String getPropertyNameByIdentifier(int propertyIdentifier);

  private:
    GetTimeCallbackFunc                  _get_time_func;
    LinkedList<ArduinoCloudProperty *>   _property_list;
    /* Keep track of the number of primitive properties in the Thing. If 0 it allows the early exit in updateTimestampOnLocallyChangedProperties() */
    int                                  _numPrimitivesProperties;
    int                                  _numProperties;
    /* Indicates the if the message received to be decoded is a response to the getLastValues inquiry */
    bool                                 _isSyncMessage;
    /* List of map data that will hold all the attributes of a property */
    LinkedList<CborMapData *>            _map_data_list;
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
    void freeMapDataList(LinkedList<CborMapData *> * map_data_list);
    inline void addProperty(ArduinoCloudProperty   * property_obj, int propertyIdentifier) {
      if (propertyIdentifier != -1) {
        property_obj->setIdentifier(propertyIdentifier);
      } else {
        // if property identifier is -1, an incremental value will be assigned as identifier.
        property_obj->setIdentifier(_numProperties);
      }
      _property_list.add(property_obj);
    }
    ArduinoCloudProperty * getProperty(String const & name);
    ArduinoCloudProperty * getProperty(int const & identifier);

};

#endif /* ARDUINO_CLOUD_THING_H_ */
