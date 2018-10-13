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
        virtual String& getName() = 0;
        virtual int getTag() = 0;
        virtual propertyType getType() = 0;
        virtual permissionType getPermission() = 0;
        virtual bool newData() = 0;
        virtual bool shouldBeUpdated() = 0;
        virtual void updateShadow() = 0;
        virtual bool canRead() = 0;
        virtual void printinfo(Stream& stream) = 0;
        void(*callback)(void) = NULL;
};

#endif /* ARDUINO_CLOUD_PROPERTY_GENERIC_H_ */
