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

#ifdef __AVR__
# include <Arduino_AVRSTL.h>
# include <nonstd/nonstd.h>
#else
# include <functional>
#endif

#include <list>

#include "../cbor/lib/tinycbor/cbor-lib.h"

/******************************************************************************
   DEFINE
 ******************************************************************************/

#define appendAttributesToCloud() appendAttributesToCloudReal(CborEncoder *encoder)
#define appendAttribute(x) appendAttributeReal(x, getAttributeName(#x, '.'), encoder)
#define setAttribute(x) setAttributeReal(x, getAttributeName(#x, '.'))

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

typedef void(*UpdateCallbackFunc)(void);
typedef unsigned long(*GetTimeCallbackFunc)();
class Property;
typedef void(*OnSyncCallbackFunc)(Property &);

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

#ifdef __AVR__
#include "nonstd/nonstd.h"
#endif

class Property
{
  public:
    Property();
    void init(String const name, Permission const permission, GetTimeCallbackFunc func);

    /* Composable configuration of the Property class */
    Property & onUpdate(UpdateCallbackFunc func);
    Property & onSync(OnSyncCallbackFunc func);
    Property & publishOnChange(float const min_delta_property, unsigned long const min_time_between_updates_millis = 0);
    Property & publishEvery(unsigned long const seconds);
    Property & publishOnDemand();
    Property & encodeTimestamp();

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
    CborError appendAttributeReal(bool value, String attributeName = "", CborEncoder *encoder = nullptr);
    CborError appendAttributeReal(int value, String attributeName = "", CborEncoder *encoder = nullptr);
    CborError appendAttributeReal(unsigned int value, String attributeName = "", CborEncoder *encoder = nullptr);
    CborError appendAttributeReal(float value, String attributeName = "", CborEncoder *encoder = nullptr);
    CborError appendAttributeReal(String value, String attributeName = "", CborEncoder *encoder = nullptr);
#ifndef __AVR__
    CborError appendAttributeName(String attributeName, std::function<CborError (CborEncoder& mapEncoder)>f, CborEncoder *encoder);
    void setAttributeReal(String attributeName, std::function<void (CborMapData & md)>setValue);
#else
    CborError appendAttributeName(String attributeName, nonstd::function<CborError (CborEncoder& mapEncoder)>f, CborEncoder *encoder);
    void setAttributeReal(String attributeName, nonstd::function<void (CborMapData & md)>setValue);
#endif
    void setAttributesFromCloud(std::list<CborMapData> * map_data_list);
    void setAttributeReal(bool& value, String attributeName = "");
    void setAttributeReal(int& value, String attributeName = "");
    void setAttributeReal(unsigned int& value, String attributeName = "");
    void setAttributeReal(float& value, String attributeName = "");
    void setAttributeReal(String& value, String attributeName = "");
    String getAttributeName(String propertyName, char separator);

    virtual bool isDifferentFromCloud() = 0;
    virtual void fromCloudToLocal() = 0;
    virtual void fromLocalToCloud() = 0;
    virtual CborError appendAttributesToCloudReal(CborEncoder *encoder) = 0;
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
    Permission         _permission;
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
