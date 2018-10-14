#ifndef ARDUINO_CLOUD_PROPERTY_INT_H_
#define ARDUINO_CLOUD_PROPERTY_INT_H_

#include "ArduinoCloudPropertyGeneric.h"

class ArduinoCloudPropertyInt : public ArduinoCloudPropertyGeneric {

public:
  ArduinoCloudPropertyInt(int & property,
                          int const min_delta,
                          String const & name,
                          permissionType const permission,
                          long const update_policy,
                          void(*fn)(void)) :
  ArduinoCloudPropertyGeneric(name, STRING, permission, update_policy, fn),
  _property(property),
  _shadow_property(property + 1),
  _min_delta(min_delta)
  {
  }

  virtual bool newData() const override {
    return (_property != _shadow_property && abs(_property - _shadow_property) >= _min_delta);
  }

  virtual void updateShadow() override {
    _shadow_property = _property;
  }

  virtual void printinfo(Stream& stream) override {
    stream.println("name: " + getName() + " value: " + String(_property) + " shadow: " + String(_shadow_property) + " permission: " + String(getPermission()));
  }

  virtual void appendValue(CborEncoder* mapEncoder) override {
    cbor_encode_text_stringz(mapEncoder, "v");
    cbor_encode_int(mapEncoder, _property);
  };

  bool write(bool const value) {
    /* permissions are intended as seen from cloud */
    if (canWrite()) {
      _property = value;
      return true;
    }
    return false;
  }

  bool read() const {
    /* permissions are intended as seen from cloud */
    if (canRead()) {
      return _property;
    }
    /* FIXME: What happens if we can not read? Compiler should complain there
     * because there is no return value in case of the canRead() evaluating
     * to false
     */
  }

private:

  int & _property;
  int   _shadow_property;
  int   _min_delta;

};

#endif /* ARDUINO_CLOUD_PROPERTY_INT_H_ */
