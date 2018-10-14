#ifndef ARDUINO_CLOUD_PROPERTY_BOOL_H_
#define ARDUINO_CLOUD_PROPERTY_BOOL_H_

#include "ArduinoCloudPropertyGeneric.h"

class ArduinoCloudPropertyBool : public ArduinoCloudPropertyGeneric {

public:
  ArduinoCloudPropertyBool(bool & property,
                           String const & name,
                           permissionType const permission,
                           long const update_policy,
                          void(*fn)(void)) :
  ArduinoCloudPropertyGeneric(name, BOOL, permission, update_policy, fn),
  _property(property),
  _shadow_property(!property)
  {
  }

  virtual bool newData() const override {
    return (_property != _shadow_property);
  }

  virtual void updateShadow() override {
    _shadow_property = _property;
  }

  virtual void printinfo(Stream& stream) override {
    stream.println("name: " + getName() + " value: " + String(_property) + " shadow: " + String(_shadow_property) + " permission: " + String(getPermission()));
  }

  virtual void appendValue(CborEncoder* mapEncoder) override {
      cbor_encode_text_stringz(mapEncoder, "vb");
      cbor_encode_boolean(mapEncoder, _property);
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

  bool & _property;
  bool   _shadow_property;

};

#endif /* ARDUINO_CLOUD_PROPERTY_BOOL_H_ */
