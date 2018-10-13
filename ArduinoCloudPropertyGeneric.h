#ifndef ARDUINO_CLOUD_PROPERTY_GENERIC_H_
#define ARDUINO_CLOUD_PROPERTY_GENERIC_H_

#include "lib/tinycbor/cbor-lib.h"

typedef enum {
    READ    = 0b01,
    WRITE   = 0b10,
    READWRITE = READ|WRITE,
} permissionType;

typedef enum {
    INT,
    FLOAT,
    BOOL,
    STRING
} propertyType;

class ArduinoCloudPropertyGeneric {
    public:
        virtual void append(CborEncoder* encoder) = 0;
        virtual String const & getName() const = 0;
        virtual int getTag() const = 0;
        virtual propertyType getType() const = 0;
        virtual permissionType getPermission() const = 0;
        virtual bool newData() const = 0;
        virtual bool shouldBeUpdated() const = 0;
        virtual void updateShadow() = 0;
        virtual bool canRead() const = 0;
        virtual void printinfo(Stream& stream) = 0;
        void(*callback)(void) = NULL;
};

#endif /* ARDUINO_CLOUD_PROPERTY_GENERIC_H_ */
