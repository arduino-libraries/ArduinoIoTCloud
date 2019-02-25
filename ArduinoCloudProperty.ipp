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

/******************************************************************************
 * CTOR/DTOR
 ******************************************************************************/

template <typename T>
ArduinoCloudProperty<T>::ArduinoCloudProperty(T & property, String const & name, Permission const permission)
: _property(property),
  _cloud_shadow_property(property),
  _local_shadow_property(property),
  _name(name),
  _permission(permission),
  _update_callback_func(NULL),
  _sync_callback_func(NULL),
  _update_policy(UpdatePolicy::OnChange),
  _has_been_updated_once(false),
  _has_been_modified_in_callback(false),
  _min_delta_property(getInitialMinDeltaPropertyValue()),
  _min_time_between_updates_millis(0),
  _last_updated_millis(0),
  _update_interval_millis(0),
  _last_local_change_timestamp(0),
  _last_cloud_change_timestamp(0),
  _previous_cloud_change_timestamp(0)
{
}

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

template <typename T>
void ArduinoCloudProperty<T>::writeByCloud(T const val) {
  if(isWriteableByCloud()) {
    _property = val;
    /* _cloud_shadow_property and local_shadow_property are not updated so there will be an update the next time around */
  }
}

template <typename T>
ArduinoCloudProperty<T> & ArduinoCloudProperty<T>::onUpdate(UpdateCallbackFunc func) {
  _update_callback_func = func;
  return (*this);
}

template <typename T>
ArduinoCloudProperty<T> & ArduinoCloudProperty<T>::onSync(void (*func)(ArduinoCloudProperty<T> property)) {
  _sync_callback_func = func;
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
bool ArduinoCloudProperty<T>::shouldBeUpdated() {
  if(!_has_been_updated_once) return true;

  if(_has_been_modified_in_callback) {
    _has_been_modified_in_callback = false;
    return true;
  }

  if     (_update_policy == UpdatePolicy::OnChange) {
    return (isValueDifferent(_property, _cloud_shadow_property) && ((millis() - _last_updated_millis) >= (_min_time_between_updates_millis)));
  }
  else if(_update_policy == UpdatePolicy::TimeInterval) {
    return ((millis() - _last_updated_millis) >= _update_interval_millis);
  }
  else {
    return false;
  }
}

template <typename T>
void ArduinoCloudProperty<T>::forceCallbackOnChange() {
  if(_update_callback_func != NULL) {
    _update_callback_func();
  }
}

template <typename T>
void ArduinoCloudProperty<T>::execCallbackOnChange() {
  if(isValueDifferent(_property, _cloud_shadow_property)) {
    if(_update_callback_func != NULL) {
      _update_callback_func();
    }
    if(!isValueDifferent(_property, _cloud_shadow_property)) {
      _has_been_modified_in_callback = true;
    }
  }
}

template <typename T>
void ArduinoCloudProperty<T>::execCallbackOnSync() {
  if(_sync_callback_func != NULL) {
    _sync_callback_func(*this);
  }
}

template <typename T>
void ArduinoCloudProperty<T>::append(CborEncoder * encoder) {
  if (isReadableByCloud()) {
    CborEncoder mapEncoder;

    cbor_encoder_create_map     (encoder, &mapEncoder, CborIndefiniteLength);
    cbor_encode_int             (&mapEncoder, static_cast<int>(CborIntegerMapKey::Name));    
    cbor_encode_text_stringz    (&mapEncoder, _name.c_str());
    appendValue                 (&mapEncoder);
    cbor_encoder_close_container(encoder, &mapEncoder);

    _cloud_shadow_property = _property;
 
    _has_been_updated_once = true;
    _last_updated_millis = millis();
  }
}

template <typename T>
bool ArduinoCloudProperty<T>::isChangedLocally() {
  return isValueDifferent(_property, _local_shadow_property);
}

template <typename T>
void ArduinoCloudProperty<T>::updateTime(unsigned long changeEventTime) {
  if (isReadableByCloud()) {
    _local_shadow_property = _property;
    _last_local_change_timestamp = changeEventTime;
  }
}

template <typename T>
void ArduinoCloudProperty<T>::setPropertyValue(T const val) {
  if(isWriteableByCloud()) {
    _property = val;
  }
}

template <typename T>
void ArduinoCloudProperty<T>::setCloudShadowValue(T const val) {
  if(isWriteableByCloud()) {
    _cloud_shadow_property = val;
  }
}

template <typename T>
void ArduinoCloudProperty<T>::setLocalShadowValue(T const val) {
  if(isWriteableByCloud()) {
    _local_shadow_property = val;
  }
}

template <typename T>
T ArduinoCloudProperty<T>::getCloudShadowValue() {
  return _cloud_shadow_property;
}

template <typename T>
T ArduinoCloudProperty<T>::getLocalShadowValue() {
  return _local_shadow_property;
}

template <typename T>
void ArduinoCloudProperty<T>::setPreviousCloudChangeTimestamp(unsigned long cloudChangeEventTime) {
  _previous_cloud_change_timestamp = cloudChangeEventTime;
}

template <typename T>
void ArduinoCloudProperty<T>::setLastCloudChangeTimestamp(unsigned long cloudChangeEventTime) {
  _previous_cloud_change_timestamp = _last_cloud_change_timestamp;
  _last_cloud_change_timestamp = cloudChangeEventTime;
}

template <typename T>
void ArduinoCloudProperty<T>::setLastLocalChangeTimestamp(unsigned long localChangeEventTime) {
  _last_local_change_timestamp = localChangeEventTime;
}

template <typename T>
unsigned long ArduinoCloudProperty<T>::getPreviousCloudChangeTimestamp() {
  return _previous_cloud_change_timestamp;
}

template <typename T>
unsigned long ArduinoCloudProperty<T>::getLastCloudChangeTimestamp() {
  return _last_cloud_change_timestamp;
}

template <typename T>
unsigned long ArduinoCloudProperty<T>::getLastLocalChangeTimestamp() {
  return _last_local_change_timestamp;
}


/******************************************************************************
 * PRIVATE MEMBER FUNCTIONS 
 ******************************************************************************/

template <>
inline void ArduinoCloudProperty<bool>::appendValue(CborEncoder * mapEncoder) const {
  cbor_encode_int    (mapEncoder, static_cast<int>(CborIntegerMapKey::BooleanValue));    
  cbor_encode_boolean(mapEncoder, _property);
}

template <>
inline void ArduinoCloudProperty<int>::appendValue(CborEncoder * mapEncoder) const {
  cbor_encode_int(mapEncoder, static_cast<int>(CborIntegerMapKey::Value));    
  cbor_encode_int(mapEncoder, _property);
}

template <>
inline void ArduinoCloudProperty<float>::appendValue(CborEncoder * mapEncoder) const {
  cbor_encode_int  (mapEncoder, static_cast<int>(CborIntegerMapKey::Value));    
  cbor_encode_float(mapEncoder, _property);
}

template <>
inline void ArduinoCloudProperty<String>::appendValue(CborEncoder * mapEncoder) const {
  cbor_encode_int         (mapEncoder, static_cast<int>(CborIntegerMapKey::StringValue));
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

template <>
inline bool ArduinoCloudProperty<bool>::getInitialMinDeltaPropertyValue() const {
  return false;
}

template <>
inline int ArduinoCloudProperty<int>::getInitialMinDeltaPropertyValue() const {
  return 0;
}

template <>
inline float ArduinoCloudProperty<float>::getInitialMinDeltaPropertyValue() const {
  return 0.0f;
}

template <>
inline String ArduinoCloudProperty<String>::getInitialMinDeltaPropertyValue() const {
  return String("");
}
