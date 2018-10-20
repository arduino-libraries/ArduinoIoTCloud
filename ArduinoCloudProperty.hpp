#ifndef ARDUINO_CLOUD_PROPERTY_HPP_
#define ARDUINO_CLOUD_PROPERTY_HPP_

#include <Arduino.h>

#include "lib/tinycbor/cbor-lib.h"

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


template <typename T>
class ArduinoCloudProperty {
public:

  ArduinoCloudProperty(T & property, String const & name, Permission const permission);

  void writeByCloud(T const val);

  /* Composable configuration of the ArduinoCloudProperty class */
  ArduinoCloudProperty<T> & onUpdate       (UpdateCallbackFunc func);
  ArduinoCloudProperty<T> & publishOnChange(T const min_delta_property, unsigned long const min_time_between_updates_millis = 0);
  ArduinoCloudProperty<T> & publishEvery   (unsigned long const seconds);

  inline String name              () const { return _name; }
  inline bool   isReadableByCloud () const { return (_permission == Permission::Read ) || (_permission == Permission::ReadWrite); }
  inline bool   isWriteableByCloud() const { return (_permission == Permission::Write) || (_permission == Permission::ReadWrite); }

  bool shouldBeUpdated        () const;
  void execCallbackOnChange   ();

  void append                 (CborEncoder * encoder);

private:

  T                & _property,
                     _shadow_property;
  String             _name;
  Permission         _permission;
  UpdateCallbackFunc _update_callback_func;

  UpdatePolicy       _update_policy;
  bool               _has_been_updated_once;
  /* Variables used for update_policy OnChange */
  T                  _min_delta_property;
  unsigned long      _min_time_between_updates_millis;
  /* Variables used for update policy TimeInterval */
  unsigned long      _last_updated_millis,
                     _update_interval_millis;

  void appendValue(CborEncoder * mapEncoder) const;
  bool isValueDifferent(T const lhs, T const rhs) const;

};

template <typename T>
inline bool operator == (ArduinoCloudProperty<T> const & lhs, ArduinoCloudProperty<T> const & rhs) { return (lhs.name() == rhs.name()); }

#include "ArduinoCloudProperty.ipp"

#endif /* ARDUINO_CLOUD_PROPERTY_HPP_ */
