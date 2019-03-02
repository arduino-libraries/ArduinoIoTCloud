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

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include <Arduino.h>
#include "lib/tinycbor/cbor-lib.h"

/******************************************************************************
   TYPEDEF
 ******************************************************************************/

/******************************************************************************
 * ENUM
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

enum class Permission {
  Read, Write, ReadWrite
};

enum class Type {
  Bool, Int, Float, String
};

enum class UpdatePolicy {
  OnChange, TimeInterval
};

typedef void(*UpdateCallbackFunc)(void);

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class ArduinoCloudProperty {
public:
  ArduinoCloudProperty();
  void init(String const name, Permission const permission);

  /* Composable configuration of the ArduinoCloudProperty class */
  ArduinoCloudProperty & onUpdate       (UpdateCallbackFunc func);
  ArduinoCloudProperty & onSync         (void (*func)(ArduinoCloudProperty &property));
  ArduinoCloudProperty & publishOnChange(float const min_delta_property, unsigned long const min_time_between_updates_millis = 0);
  ArduinoCloudProperty & publishEvery   (unsigned long const seconds);

  inline String name              () const { return _name; }
  inline bool   isReadableByCloud () const { return (_permission == Permission::Read ) || (_permission == Permission::ReadWrite); }
  inline bool   isWriteableByCloud() const { return (_permission == Permission::Write) || (_permission == Permission::ReadWrite); }

  bool shouldBeUpdated        ();
  void execCallbackOnChange   ();
  void execCallbackOnSync     ();
  void setLastCloudChangeTimestamp         (unsigned long cloudChangeTime);
  void setLastLocalChangeTimestamp         (unsigned long localChangeTime);
  unsigned long getLastCloudChangeTimestamp();
  unsigned long getLastLocalChangeTimestamp();

  void updateLocalTimestamp   ();
  void append                 (CborEncoder * encoder);

  virtual bool isDifferentFromCloudShadow() = 0;
  virtual void toShadow() = 0;
  virtual void fromCloudShadow() = 0;
  virtual void appendValue(CborEncoder * mapEncoder) const = 0;
  virtual void setValue(CborMapData const * const map_data) = 0;
  virtual void setCloudShadowValue(CborMapData const * const map_data) = 0;
  virtual bool isPrimitive() { return false; };
  virtual bool isChangedLocally() { return false; };
protected:
  /* Variables used for UpdatePolicy::OnChange */
  float              _min_delta_property;
  unsigned long      _min_time_between_updates_millis;

private:
  String             _name;
  Permission         _permission;
  UpdateCallbackFunc _update_callback_func;
  void               (*_sync_callback_func)(ArduinoCloudProperty &property);

  UpdatePolicy       _update_policy;
  bool               _has_been_updated_once,
                     _has_been_modified_in_callback;
  /* Variables used for UpdatePolicy::TimeInterval */
  unsigned long      _last_updated_millis,
                     _update_interval_millis;
  /* Variables used for reconnection sync*/
  unsigned long      _last_local_change_timestamp;
  unsigned long      _last_cloud_change_timestamp;
        
};

/******************************************************************************
   PROTOTYPE FREE FUNCTIONs
 ******************************************************************************/

inline bool operator == (ArduinoCloudProperty const & lhs, ArduinoCloudProperty const & rhs) { return (lhs.name() == rhs.name()); }

#endif /* ARDUINO_CLOUD_PROPERTY_HPP_ */
