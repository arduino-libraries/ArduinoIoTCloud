#ifndef ARDUINO_CLOUD_PROPERTY_HPP_
#define ARDUINO_CLOUD_PROPERTY_HPP_

enum class Permission {
  Read, Write, ReadWrite
};

enum class Type {
  Bool, Int, Float, String
};

enum class UpdatePolicy {
  OnChange, TimeInterval
};

template <typename T>
class ArduinoCloudProperty {
public:

  ArduinoCloudProperty(String const & name, T & property, Permission const permission, UpdatePolicy const update_policy);

  bool read (T * val) const;

  inline void publishEvery(unsigned long const seconds  ) { _update_interval_sec = seconds; }
  inline void setMinDelta (T             const min_delta) { _min_delta_property_val = min_delta; }

  inline String name    () const { return _name; }
         Type   type    () const;
  inline bool   canRead () const { return (_permission == Permission::Read ) || (_permission == Permission::ReadWrite); }
  inline bool   canWrite() const { return (_permission == Permission::Write) || (_permission == Permission::ReadWrite); }

  bool shouldBeUpdated() const;
  void append(CborEncoder * encoder);

private:

  String          _name;
  T             & _property,
                  _shadow_property;
  Permission      _permission;
  UpdatePolicy    _update_policy;
  /* Variables used for update_policy OnChange */
  T               _min_delta_property_val;
  /* Variables used for update policy TimeInterval */
  unsigned long   _last_updated,
                  _update_interval_sec;

  void appendValue(CborEncoder * mapEncoder) const;
  bool isValueDifferent(T const lhs, T const rhs) const;

};

template <typename T>
inline bool operator == (ArduinoCloudProperty<T> const & lhs, ArduinoCloudProperty<T> const & rhs) { return (lhs.name() == rhs.name()); }

template <typename T>
ArduinoCloudProperty<T>::ArduinoCloudProperty(String const & name, T & property, Permission const permission, UpdatePolicy const update_policy)
: _name(name),
  _property(property),
  _shadow_property(property),
  _permission(permission),
  _update_policy(update_policy),
  _last_updated(0),
  _update_interval_sec(0)
{
}

template <typename T>
bool ArduinoCloudProperty<T>::read(T * val) const {
  if(!canRead()) return false;
  *val = _property;
  return true;
}

template <>
inline Type ArduinoCloudProperty<bool>::type() const {
  return Type::Bool;
}

template <>
inline Type ArduinoCloudProperty<int>::type() const {
  return Type::Int;
}

template <>
inline Type ArduinoCloudProperty<float>::type() const {
  return Type::Float;
}

template <>
inline Type ArduinoCloudProperty<String>::type() const {
  return Type::String;
}

template <typename T>
bool ArduinoCloudProperty<T>::shouldBeUpdated() const {
  if     (_update_policy == UpdatePolicy::OnChange) {
    return isValueDifferent(_property, _shadow_property);
  }
  else if(_update_policy == UpdatePolicy::TimeInterval) {
    return ((millis() - _last_updated) > (_update_interval_sec * 1000));
  }
  else {
    return false;
  }
}

template <typename T>
void ArduinoCloudProperty<T>::append(CborEncoder * encoder) {
  if (canRead()) {
    CborEncoder mapEncoder;

    cbor_encoder_create_map(encoder, &mapEncoder, CborIndefiniteLength);
    cbor_encode_text_stringz(&mapEncoder, "n");
    cbor_encode_text_stringz(&mapEncoder, _name.c_str());
    appendValue(&mapEncoder);
    cbor_encoder_close_container(encoder, &mapEncoder);

    _shadow_property = _property;
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
  return (lhs != rhs) && (abs(lhs - rhs) >= _min_delta_property_val);
}

template <>
inline bool ArduinoCloudProperty<float>::isValueDifferent(float const lhs, float const rhs) const {
  return (lhs != rhs) && (abs(lhs - rhs) >= _min_delta_property_val);
}

template <>
inline bool ArduinoCloudProperty<String>::isValueDifferent(String const lhs, String const rhs) const {
  return (lhs != rhs);
}

#endif /* ARDUINO_CLOUD_PROPERTY_HPP_ */
