template <typename T>
ArduinoCloudProperty<T>::ArduinoCloudProperty(T& _property, T const _shadow_property, String const & _name, propertyType const property_type, T const _minDelta, permissionType const _permission, long const _updatePolicy, void(*fn)(void)) :
  ArduinoCloudPropertyGeneric(_name, property_type, _permission, _updatePolicy, fn),
  property       (_property       ),
  shadow_property(_shadow_property),
  minDelta       (_minDelta       )
{
}

template <typename T>
bool ArduinoCloudProperty<T>::write(T value) {
  /* permissions are intended as seen from cloud */
  if (canWrite()) {
    property = value;
    return true;
  }
  return false;
}

template <typename T>
T ArduinoCloudProperty<T>::read() {
  /* permissions are intended as seen from cloud */
  if (canRead()) {
    return property;
  }
  /* FIXME: What happens if we can not read? Compiler should complain there
   * because there is no return value in case of the canRead() evaluating
   * to false
   */
}

template <typename T>
void ArduinoCloudProperty<T>::printinfo(Stream& stream) {
  stream.println("name: " + getName() + " value: " + String(property) + " shadow: " + String(shadow_property) + " permission: " + String(getPermission()));
}

template <>
inline bool ArduinoCloudProperty<float>::newData() const {
    return (property != shadow_property && abs(property - shadow_property) >= minDelta );
}

template <>
inline void ArduinoCloudProperty<float>::appendValue(CborEncoder* mapEncoder) {
    cbor_encode_text_stringz(mapEncoder, "v");
    cbor_encode_float(mapEncoder, property);
};
