#ifndef ARDUINO_CLOUD_PROPERTY_H_
#define ARDUINO_CLOUD_PROPERTY_H_

#include "ArduinoCloudPropertyGeneric.h"

// definition of the default property update policy
static const int ON_CHANGE = -1;

template <typename T>
class ArduinoCloudProperty : public ArduinoCloudPropertyGeneric {
    public:

        ArduinoCloudProperty(T& _property, T const _shadow_property, String const & _name, T const _minDelta, permissionType const _permission, long const _updatePolicy, void(*fn)(void));

        bool write(T value);
        T    read ();

        virtual void printinfo(Stream& stream) override;

        virtual void updateShadow   () override { shadow_property = property; }
        virtual bool shouldBeUpdated() const override;
        virtual bool newData        () const override { return (property != shadow_property); };

        inline  bool canWrite() const          { return (permission & WRITE); }
        virtual bool canRead () const override { return (permission & READ);  }

        virtual String const & getName      () const override { return name;       }
        virtual int            getTag       () const override { return tag;        }
        virtual permissionType getPermission() const override { return permission; }
        virtual propertyType   getType      () const override;

        virtual void append     (CborEncoder* encoder) override;
                void appendValue(CborEncoder* mapEncoder);

        inline bool operator == (ArduinoCloudProperty const & rhs) const { return (getName() == rhs.getName()); }

      private:

        T& property;
        T shadow_property;
        String name;
        T minDelta;
        permissionType permission = READWRITE;
        long updatePolicy = ON_CHANGE;
        int tag = -1;
        long lastUpdated = 0;

};

#include "ArduinoCloudProperty.ipp"

#endif /* ARDUINO_CLOUD_PROPERTY_H_ */
