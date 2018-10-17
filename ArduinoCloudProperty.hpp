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

  bool writeByCloud(T const val);

  /* Composable configuration of the ArduinoCloudProperty class */
  ArduinoCloudProperty<T> & onUpdate       (UpdateCallbackFunc func);
  ArduinoCloudProperty<T> & publishOnChange(T const min_delta_property, unsigned long const min_time_between_updates_milliseconds = 0);
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
  unsigned long      _min_time_between_updates_milliseconds;
  /* Variables used for update policy TimeInterval */
  unsigned long      _last_updated,
                     _update_interval_sec;

  void appendValue(CborEncoder * mapEncoder) const;
  bool isValueDifferent(T const lhs, T const rhs) const;

};

template <typename T>
inline bool operator == (ArduinoCloudProperty<T> const & lhs, ArduinoCloudProperty<T> const & rhs) { return (lhs.name() == rhs.name()); }

template <typename T>
ArduinoCloudProperty<T>::ArduinoCloudProperty(T & property, String const & name, Permission const permission)
: _property(property),
  _shadow_property(property),
  _name(name),
  _permission(permission),
  _update_callback_func(NULL),
  _update_policy(UpdatePolicy::OnChange),
  _has_been_updated_once(false),
  _min_delta_property(T(0)),
  _min_time_between_updates_milliseconds(0),
  _last_updated(0),
  _update_interval_sec(0)
{
}

template <typename T>
bool ArduinoCloudProperty<T>::writeByCloud(T const val) {
  if(!isWriteableByCloud()) return false;
  _property = val;
  /* _shadow_property is not updated so there will be an update the next time around */
  return true;
}

template <typename T>
ArduinoCloudProperty<T> & ArduinoCloudProperty<T>::onUpdate(UpdateCallbackFunc func) {
  _update_callback_func = func;
  return (*this);
}

template <typename T>
ArduinoCloudProperty<T> & ArduinoCloudProperty<T>::publishOnChange(T const min_delta_property, unsigned long const min_time_between_updates_milliseconds) {
  _update_policy = UpdatePolicy::OnChange;
  _min_delta_property = min_delta_property;
  _min_time_between_updates_milliseconds = min_time_between_updates_milliseconds;
  return (*this);
}

template <typename T>
ArduinoCloudProperty<T> & ArduinoCloudProperty<T>::publishEvery(unsigned long const seconds) {
  _update_policy = UpdatePolicy::TimeInterval;
  _update_interval_sec = seconds;
  return (*this);
}

template <typename T>
bool ArduinoCloudProperty<T>::shouldBeUpdated() const {
  if(!_has_been_updated_once) return true;

  if     (_update_policy == UpdatePolicy::OnChange) {
    return (isValueDifferent(_property, _shadow_property) && ((millis() - _last_updated) > (_min_time_between_updates_milliseconds)));
  }
  else if(_update_policy == UpdatePolicy::TimeInterval) {
    return ((millis() - _last_updated) > (_update_interval_sec * 1000));
  }
  else {
    return false;
  }
}

template <typename T>
void ArduinoCloudProperty<T>::execCallbackOnChange() {
  if(isValueDifferent(_property, _shadow_property)) {
    if(_update_callback_func != NULL) {
      _update_callback_func();
    }
  }
}

template <typename T>
void ArduinoCloudProperty<T>::append(CborEncoder * encoder) {
  if (isReadableByCloud()) {
    CborEncoder mapEncoder;

    cbor_encoder_create_map(encoder, &mapEncoder, CborIndefiniteLength);
    cbor_encode_text_stringz(&mapEncoder, "n");
    cbor_encode_text_stringz(&mapEncoder, _name.c_str());
    appendValue(&mapEncoder);
    cbor_encoder_close_container(encoder, &mapEncoder);

    _shadow_property = _property;
    _has_been_updated_once = true;
    _last_updated = millis();
  }
}

template <>
inline void ArduinoCloudProperty<bool>::appendValue(CborEncoder * mapEncoder) const {
  cbor_encode_text_stringz(mapEncoder, "vb");
  cbor_encode_boolean(mapEncoder, _property);
}

template <>
inline void ArduinoCloudProperty<int>::appendValue(CborEncoder * mapEncoder) const {
  cbor_encode_text_stringz(mapEncoder, "v");
  cbor_encode_int(mapEncoder, _property);
}

template <>
inline void ArduinoCloudProperty<float>::appendValue(CborEncoder * mapEncoder) const {
  cbor_encode_text_stringz(mapEncoder, "v");
  cbor_encode_float(mapEncoder, _property);
}

template <>
inline void ArduinoCloudProperty<String>::appendValue(CborEncoder * mapEncoder) const {
  cbor_encode_text_stringz(mapEncoder, "vs");
  cbor_encode_text_stringz(mapEncoder, _property.c_str());
}

template <>
inline bool ArduinoCloudProperty<bool>::isValueDifferent(bool const lhs, bool const rhs) const {
  return (lhs != rhs);
}

template <>
inline bool ArduinoCloudProperty<int>::isValueDifferent(int const lhs, int const rhs) const {
  return (lhs != rhs) && (abs(lhs - rhs) >= _min_delta_property);
}

template <>
inline bool ArduinoCloudProperty<float>::isValueDifferent(float const lhs, float const rhs) const {
  return (lhs != rhs) && (abs(lhs - rhs) >= _min_delta_property);
}

template <>
inline bool ArduinoCloudProperty<String>::isValueDifferent(String const lhs, String const rhs) const {
  return (lhs != rhs);
}

#endif /* ARDUINO_CLOUD_PROPERTY_HPP_ */
