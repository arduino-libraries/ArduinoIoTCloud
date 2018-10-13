#ifndef ARDUINO_CLOUD_PROPERTY_H_
#define ARDUINO_CLOUD_PROPERTY_H_

#include "ArduinoCloudPropertyGeneric.h"

class ArduinoCloudThing; /* Forward declaration to satisfy compiler for this line "friend ArduinoCloudThing" */

// definition of the default property update policy
static const int ON_CHANGE = -1;

template <typename T>
class ArduinoCloudProperty : public ArduinoCloudPropertyGeneric {
    public:

        ArduinoCloudProperty(T& _property,  String _name);

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

    protected:
        T& property;
        T shadow_property;
        String name;
        int tag = -1;
        long lastUpdated = 0;
        long updatePolicy = ON_CHANGE;
        T minDelta;
        permissionType permission = READWRITE;
        static int tagIndex;

        // In this way it is possible to set shadow_property(protected), from ArduinoCloudThing,
        // when a new property is added.
        friend ArduinoCloudThing;
};

#include "ArduinoCloudProperty.ipp"

#endif /* ARDUINO_CLOUD_PROPERTY_H_ */
