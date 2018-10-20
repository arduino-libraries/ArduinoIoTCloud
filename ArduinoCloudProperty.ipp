
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
  _min_time_between_updates_millis(0),
  _last_updated_millis(0),
  _update_interval_millis(0)
{
}

template <typename T>
void ArduinoCloudProperty<T>::writeByCloud(T const val) {
  if(isWriteableByCloud()) {
    _property = val;
    /* _shadow_property is not updated so there will be an update the next time around */
  }
}

template <typename T>
ArduinoCloudProperty<T> & ArduinoCloudProperty<T>::onUpdate(UpdateCallbackFunc func) {
  _update_callback_func = func;
  return (*this);
}

template <typename T>
ArduinoCloudProperty<T> & ArduinoCloudProperty<T>::publishOnChange(T const min_delta_property, unsigned long const min_time_between_updates_millis) {
  _update_policy = UpdatePolicy::OnChange;
  _min_delta_property = min_delta_property;
  _min_time_between_updates_millis = min_time_between_updates_millis;
  return (*this);
}

template <typename T>
ArduinoCloudProperty<T> & ArduinoCloudProperty<T>::publishEvery(unsigned long const seconds) {
  _update_policy = UpdatePolicy::TimeInterval;
  _update_interval_millis = (seconds * 1000);
  return (*this);
}

template <typename T>
bool ArduinoCloudProperty<T>::shouldBeUpdated() const {
  if(!_has_been_updated_once) return true;

  if     (_update_policy == UpdatePolicy::OnChange) {
    return (isValueDifferent(_property, _shadow_property) && ((millis() - _last_updated_millis) >= (_min_time_between_updates_millis)));
  }
  else if(_update_policy == UpdatePolicy::TimeInterval) {
    return ((millis() - _last_updated_millis) >= _update_interval_millis);
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
    _last_updated_millis = millis();
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
