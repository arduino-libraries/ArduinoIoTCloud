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

#ifndef ARDUINO_CLOUD_PROPERTY_HPP_
#define ARDUINO_CLOUD_PROPERTY_HPP_

#ifdef HOST
  #define substring(...) substr(__VA_ARGS__)
  #define indexOf(x) find(x)
#endif

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include <Arduino.h>

#undef max
#undef min

#include <list>

#include "../cbor/lib/tinycbor/cbor-lib.h"

/******************************************************************************
   CONST
 ******************************************************************************/
namespace PropertyActions
{
  extern const String CLEAR;
}

/******************************************************************************
   ENUM
 ******************************************************************************/
/* Source: https://tools.ietf.org/html/rfc8428#section-6 */
enum class CborIntegerMapKey : int {
  BaseVersion  = -1, /* bver */
  BaseName     = -2, /* bn   */
  BaseTime     = -3, /* bt   */
  BaseUnit     = -4, /* bu   */
  BaseValue    = -5, /* bv   */
  BaseSum      = -6, /* bs   */
  Name         =  0, /* n    */
  Unit         =  1, /* u    */
  Value        =  2, /* v    */
  StringValue  =  3, /* vs   */
  BooleanValue =  4, /* vb   */
  Sum          =  5, /* s    */
  Time         =  6, /* t    */
  UpdateTime   =  7, /* ut   */
  DataValue    =  8  /* vd   */
};

template <typename T>
class MapEntry {
  public:

    MapEntry() : _is_set(false) { }

    inline void    set(T const & entry) {
      _entry = entry;
      _is_set = true;
    }
    inline T const get() const {
      return _entry;
    }

    inline bool    isSet() const {
      return _is_set;
    }
    inline void    reset()       {
      _is_set = false;
    }

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
    MapEntry<int>    name_identifier;
    MapEntry<bool>   light_payload;
    MapEntry<String> attribute_name;
    MapEntry<int>    attribute_identifier;
    MapEntry<int>    property_identifier;
    MapEntry<double> val;
    MapEntry<String> str_val;
    MapEntry<bool>   bool_val;
    MapEntry<double> time;
};

enum class Permission {
  Read, Write, ReadWrite
};

enum class Type {
  Bool, Int, Float, String
};

enum class UpdatePolicy {
  OnChange, TimeInterval, OnDemand
};

enum class WritePolicy {
  Auto, Manual
};

typedef void(*UpdateCallbackFunc)(void);
typedef unsigned long(*GetTimeCallbackFunc)();
class Property;
typedef void(*OnSyncCallbackFunc)(Property &);

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class Property
{
  public:
    Property();
    void init(String const name, Permission const permission, GetTimeCallbackFunc func);

    /* Composable configuration of the Property class */
    Property & onUpdate(UpdateCallbackFunc func);
    Property & onSync(OnSyncCallbackFunc func);
    Property & publishOnChange(float const min_delta_property, unsigned long const min_time_between_updates_millis = DEFAULT_MIN_TIME_BETWEEN_UPDATES_MILLIS);
    Property & publishEvery(unsigned long const seconds);
    Property & publishOnDemand();
    Property & encodeTimestamp();
    Property & writeOnChange();
    Property & writeOnDemand();

    inline String name() const {
      return _name;
    }
    inline int identifier() const {
      return _identifier;
    }
    inline bool   isReadableByCloud() const {
      return (_permission == Permission::Read) || (_permission == Permission::ReadWrite);
    }
    inline bool   isWriteableByCloud() const {
      return (_permission == Permission::Write) || (_permission == Permission::ReadWrite);
    }
    inline bool   isWritableOnChange() const {
      return _write_policy == WritePolicy::Auto;
    }

    void setTimestamp(unsigned long const timestamp);
    bool shouldBeUpdated();
    void requestUpdate();
    void appendCompleted();
    void provideEcho();
    void execCallbackOnChange();
    void execCallbackOnSync();
    void setLastCloudChangeTimestamp(unsigned long cloudChangeTime);
    void setLastLocalChangeTimestamp(unsigned long localChangeTime);
    unsigned long getLastCloudChangeTimestamp();
    unsigned long getLastLocalChangeTimestamp();
    void setIdentifier(int identifier);

    void updateLocalTimestamp();
    CborError append(CborEncoder * encoder, bool lightPayload);
    template <typename T> CborError appendAttribute(T value, String attributeName = "", CborEncoder *encoder = nullptr) {
      return appendAttributeName<T>(attributeName, encodeAppendedAttribute<T>, value, encoder);
    }
    template <typename T> CborError appendAttributeName(String attributeName, CborError(*appendValue)(CborEncoder& mapEncoder, const T), const T & value, CborEncoder *encoder)
    {
      if (attributeName != "") {
        // when the attribute name string is not empty, the attribute identifier is incremented in order to be encoded in the message if the _lightPayload flag is set
        _attributeIdentifier++;
      }
      CborEncoder mapEncoder;
      unsigned int num_map_properties = _encode_timestamp ? 3 : 2;
      CHECK_CBOR(cbor_encoder_create_map(encoder, &mapEncoder, num_map_properties));
      CHECK_CBOR(cbor_encode_int(&mapEncoder, static_cast<int>(CborIntegerMapKey::Name)));

      // if _lightPayload is true, the property and attribute identifiers will be encoded instead of the property name
      if (_lightPayload)
      {
        // the most significant byte of the identifier to be encoded represent the property identifier
        int completeIdentifier = _attributeIdentifier * 256;
        // the least significant byte of the identifier to be encoded represent the attribute identifier
        completeIdentifier += _identifier;
        CHECK_CBOR(cbor_encode_int(&mapEncoder, completeIdentifier));
      }
      else
      {
        String completeName = _name;
        if (attributeName != "") {
          completeName += ":" + attributeName;
        }
        CHECK_CBOR(cbor_encode_text_stringz(&mapEncoder, completeName.c_str()));
      }
      /* Encode the value */
      CHECK_CBOR(appendValue(mapEncoder, value));

      /* Encode the timestamp if that has been required. */
      if(_encode_timestamp)
      {
        CHECK_CBOR(cbor_encode_int (&mapEncoder, static_cast<int>(CborIntegerMapKey::Time)));
        CHECK_CBOR(cbor_encode_uint(&mapEncoder, _timestamp));
      }
      /* Close the container */
      CHECK_CBOR(cbor_encoder_close_container(encoder, &mapEncoder));
      return CborNoError;
    }
    template <typename T> void setAttribute(const String attributeName, void(* const setValue)(CborMapData & md, T &), T & value)
    {
      if (attributeName != "") {
        _attributeIdentifier++;
      }

      std::list<CborMapData>::iterator it = _map_data_list->begin();
      for (; it != _map_data_list->end(); ++it) {
        CborMapData & map = *it;
        if (map.light_payload.isSet() && map.light_payload.get())
        {
          // if a light payload is detected, the attribute identifier is retrieved
          // from the cbor map and the corresponding attribute is updated
          int attid = map.attribute_identifier.get();
          if (attid == _attributeIdentifier) {
            setValue(map, value);
            return;
          }
        }
        else
        {
          // if a normal payload is detected, the name of the attribute to be
          // updated is extracted directly from the cbor map
          String an = map.attribute_name.get();
          if (an == attributeName) {
            setValue(map, value);
            return;
          }
        }
      }
    }

    template <typename T> void setAttribute(T& value, String attributeName = "") {
      setAttribute<T>(attributeName, setValueAttribute<T>, value);
    }
    void setAttributesFromCloud(std::list<CborMapData> * map_data_list);

    virtual bool isDifferentFromCloud() = 0;
    virtual void fromCloudToLocal() = 0;
    virtual void fromLocalToCloud() = 0;
    virtual CborError appendAttributesToCloud(CborEncoder *encoder) = 0;
    virtual void setAttributesFromCloud() = 0;
    virtual bool isPrimitive() {
      return false;
    };

    static unsigned long const DEFAULT_MIN_TIME_BETWEEN_UPDATES_MILLIS = 500; /* Data rate throttled to 2 Hz */

  protected:
    /* Variables used for UpdatePolicy::OnChange */
    String             _name;
    float              _min_delta_property;
    unsigned long      _min_time_between_updates_millis;

  private:
    template <typename T> static CborError encodeAppendedAttribute(CborEncoder & mapEncoder, const T value) {
      CHECK_CBOR(cbor_encode_int(&mapEncoder, static_cast<int>(CborIntegerMapKey::Value)));
      CHECK_CBOR(cbor_encode_int(&mapEncoder, value));
      return CborNoError;
    }

    template <typename T> static void setValueAttribute(CborMapData & md, T & value) {
      if (md.val.isSet()) {
        value = md.val.get();
      }
    }

    Permission         _permission;
    WritePolicy        _write_policy;
    GetTimeCallbackFunc _get_time_func;
    UpdateCallbackFunc _update_callback_func;
    OnSyncCallbackFunc _on_sync_callback_func;

    UpdatePolicy       _update_policy;
    bool               _has_been_updated_once,
                       _has_been_modified_in_callback,
                       _has_been_appended_but_not_sended;
    /* Variables used for UpdatePolicy::TimeInterval */
    unsigned long      _last_updated_millis,
                       _update_interval_millis;
    /* Variables used for reconnection sync*/
    unsigned long      _last_local_change_timestamp;
    unsigned long      _last_cloud_change_timestamp;
    std::list<CborMapData> * _map_data_list;
    /* Store the identifier of the property in the array list */
    int                _identifier;
    int                _attributeIdentifier;
    /* Indicates if the property shall be encoded using the identifier instead of the name */
    bool               _lightPayload;
    /* Indicates whether a property update has been requested in case of the OnDemand update policy. */
    bool               _update_requested;
    /* Indicates whether the timestamp shall be encoded in the property or not */
    bool               _encode_timestamp;
    /* Indicates if the property shall be echoed back to the cloud even if unchanged */
    bool               _echo_requested;
    unsigned long      _timestamp;
};

template <> inline CborError Property::encodeAppendedAttribute<bool>(CborEncoder & mapEncoder, const bool value) {
  CHECK_CBOR(cbor_encode_int(&mapEncoder, static_cast<int>(CborIntegerMapKey::BooleanValue)));
  CHECK_CBOR(cbor_encode_boolean(&mapEncoder, value));
  return CborNoError;
}

template <> inline CborError Property::encodeAppendedAttribute<float>(CborEncoder & mapEncoder, const float value) {
  CHECK_CBOR(cbor_encode_int(&mapEncoder, static_cast<int>(CborIntegerMapKey::Value)));
  CHECK_CBOR(cbor_encode_float(&mapEncoder, value));
  return CborNoError;
}

template <> inline CborError Property::encodeAppendedAttribute<String>(CborEncoder & mapEncoder, const String value) {
  CHECK_CBOR(cbor_encode_int(&mapEncoder, static_cast<int>(CborIntegerMapKey::StringValue)));
  CHECK_CBOR(cbor_encode_text_stringz(&mapEncoder, value.c_str()));
  return CborNoError;
}

template <> inline void Property::setValueAttribute<String>(CborMapData & md, String & value) {
  if (md.str_val.isSet()) {
    value = md.str_val.get();
  }
}

template <> inline void Property::setValueAttribute<bool>(CborMapData & md, bool & value) {
  // Manage the case to have boolean values received as integers 0/1
  if (md.bool_val.isSet()) {
    value = md.bool_val.get();
  } else if (md.val.isSet()) {
    if (md.val.get() == 0) {
      value = false;
    } else if (md.val.get() == 1) {
      value = true;
    } else {
      /* This should not happen. Leave the previous value */
    }
  }
}

/******************************************************************************
   PROTOTYPE FREE FUNCTIONs
 ******************************************************************************/

inline bool operator == (Property const & lhs, Property const & rhs) {
  return (lhs.name() == rhs.name());
}

/******************************************************************************
   SYNCHRONIZATION CALLBACKS
 ******************************************************************************/

void onAutoSync(Property & property);
#define MOST_RECENT_WINS onAutoSync
void onForceCloudSync(Property & property);
#define CLOUD_WINS onForceCloudSync
void onForceDeviceSync(Property & property);
#define DEVICE_WINS onForceDeviceSync // The device property value is already the correct one. The cloud property value will be synchronized at the next update cycle.

#endif /* ARDUINO_CLOUD_PROPERTY_HPP_ */
