template <typename T>
ArduinoCloudProperty<T>::ArduinoCloudProperty(T& _property,  String _name) :
  property(_property), name(_name)
  {
  }

template <typename T>
bool ArduinoCloudProperty<T>::write(T value) {
  /* permissions are intended as seen from cloud */
  if (permission & WRITE) {
    property = value;
    return true;
  }
  return false;
}

template <typename T>
void ArduinoCloudProperty<T>::printinfo(Stream& stream) {
  stream.println("name: " + name + " value: " + String(property) + " shadow: " + String(shadow_property) + " permission: " + String(permission));
}

template <typename T>
T ArduinoCloudProperty<T>::read() {
  /* permissions are intended as seen from cloud */
  if (permission & READ) {
    return property;
  }
}

template <typename T>
void ArduinoCloudProperty<T>::append(CborEncoder* encoder) {
  if (!canRead()) {
    return;
  }
  CborEncoder mapEncoder;
  cbor_encoder_create_map(encoder, &mapEncoder, CborIndefiniteLength);
  if (tag != -1) {
    cbor_encode_text_stringz(&mapEncoder, "t");
    cbor_encode_int(&mapEncoder, tag);
  }
  else {
    cbor_encode_text_stringz(&mapEncoder, "n");
    cbor_encode_text_stringz(&mapEncoder, name.c_str());
  }
  appendValue(&mapEncoder);
  cbor_encoder_close_container(encoder, &mapEncoder);
  lastUpdated = millis();
}

template <typename T>
bool ArduinoCloudProperty<T>::shouldBeUpdated() {
    if (updatePolicy == ON_CHANGE) {
        return newData();
    }
    return ((millis() - lastUpdated) > (updatePolicy * 1000)) ;
}

template <>
inline bool ArduinoCloudProperty<int>::newData() {
    return (property != shadow_property && abs(property - shadow_property) >= minDelta );
}

template <>
inline bool ArduinoCloudProperty<float>::newData() {
    return (property != shadow_property && abs(property - shadow_property) >= minDelta );
}

// Different appendValue function for different property typer, because the CBOR encoder and message format
template <>
inline void ArduinoCloudProperty<int>::appendValue(CborEncoder* mapEncoder) {
    cbor_encode_text_stringz(mapEncoder, "v");
    cbor_encode_int(mapEncoder, property);
};

template <>
inline void ArduinoCloudProperty<bool>::appendValue(CborEncoder* mapEncoder) {
    cbor_encode_text_stringz(mapEncoder, "vb");
    cbor_encode_boolean(mapEncoder, property);
};

template <>
inline void ArduinoCloudProperty<float>::appendValue(CborEncoder* mapEncoder) {
    cbor_encode_text_stringz(mapEncoder, "v");
    cbor_encode_float(mapEncoder, property);
};

template <>
inline void ArduinoCloudProperty<String>::appendValue(CborEncoder* mapEncoder) {
    cbor_encode_text_stringz(mapEncoder, "vs");
    cbor_encode_text_stringz(mapEncoder, property.c_str());
};

// Return property type
template <>
inline propertyType ArduinoCloudProperty<int>::getType() {
    return INT;
}

template <>
inline propertyType ArduinoCloudProperty<float>::getType() {
    return FLOAT;
}

template <>
inline propertyType ArduinoCloudProperty<bool>::getType() {
    return BOOL;
}

template <>
inline propertyType ArduinoCloudProperty<String>::getType() {
    return STRING;
}
