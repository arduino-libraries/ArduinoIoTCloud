#ifndef ARDUINO_CLOUD_PROPERTY_H_
#define ARDUINO_CLOUD_PROPERTY_H_

#include "ArduinoCloudPropertyGeneric.h"

template <typename T>
class ArduinoCloudProperty : public ArduinoCloudPropertyGeneric {
    public:

        ArduinoCloudProperty(T& _property, T const _shadow_property, String const & _name, propertyType const property_type, T const _minDelta, permissionType const _permission, long const _updatePolicy, void(*fn)(void));

        bool write(T value);
        T    read ();

        virtual void printinfo(Stream& stream) override;

        virtual void updateShadow   () override       { shadow_property = property; }
        virtual bool newData        () const override { return (property != shadow_property); };
        virtual void appendValue    (CborEncoder* mapEncoder) override;

      private:

        T& property;
        T shadow_property;
        T minDelta;
};

#include "ArduinoCloudProperty.ipp"

#endif /* ARDUINO_CLOUD_PROPERTY_H_ */
