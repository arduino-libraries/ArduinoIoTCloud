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

        void printinfo(Stream& stream);

        inline void updateShadow   () { shadow_property = property;           }
               bool shouldBeUpdated();
        inline bool newData        () { return (property != shadow_property); }


        inline bool canRead() { return (permission & READ); }

        inline String&        getName      () { return name;       }
        inline int            getTag       () { return tag;        }
        inline permissionType getPermission() { return permission; }
               propertyType   getType      ();

        void append     (CborEncoder* encoder);
        void appendValue(CborEncoder* mapEncoder);

        inline bool operator == (const ArduinoCloudProperty& rhs) { return (getName() == rhs.getName()); }

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
