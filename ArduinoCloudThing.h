#ifndef ArduinoCloudThing_h
#define ArduinoCloudThing_h

#include <Client.h>
#include <Stream.h>
#include "lib/LinkedList/LinkedList.h"
#include "lib/tinycbor/cbor-lib.h"

// definition of the default property update policy
static const int ON_CHANGE = -1;

enum permissionType {
    READ    = 0b01,
    WRITE   = 0b10,
    READWRITE = READ|WRITE,
};

enum boolStatus {
    ON,
    OFF,
};

// allowed property types
enum propertyType {
    INT,
    FLOAT,
    BOOL,
    STRING
};

enum times {
    SECONDS = 1,
    MINUTES = 60,
    HOURS = 3600,
    DAYS = 86400,
};

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

class ArduinoCloudThing {
    public:
        ArduinoCloudThing();
        void begin();
        // overload, with default arguments, of different type of properties
        ArduinoCloudPropertyGeneric& addPropertyReal(int& property, String name, permissionType _permission = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, int minDelta = 0);
        ArduinoCloudPropertyGeneric& addPropertyReal(bool& property, String name, permissionType _permission = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, bool minDelta = false);
        ArduinoCloudPropertyGeneric& addPropertyReal(float& property, String name, permissionType _permission = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f);
        ArduinoCloudPropertyGeneric& addPropertyReal(String& property, String name, permissionType _permission = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, String minDelta = "");
        // poll should return > 0 if something has changed
        int poll(uint8_t* data, size_t size);
        // decode a CBOR payload received from the Cloud.
        void decode(uint8_t * payload, size_t length);

    private:
        void update();
        int checkNewData();
        // return the index of that property in the linked list, -1 if it does not exist.
        int findPropertyByName(String &name);
        // return the pointer to the desired property, NULL if it does not exist.
        ArduinoCloudPropertyGeneric* exists(String &name);

        bool status = OFF;
        char uuid[33];
        int currentListIndex = -1;
        LinkedList<ArduinoCloudPropertyGeneric*> list;

};

// ArduinoCloudProperty declaration and definition. It inherits from ArduinoCloudPropertyGeneric interface.
template <typename T>
class ArduinoCloudProperty : public ArduinoCloudPropertyGeneric {
    public:
        ArduinoCloudProperty(T& _property,  String _name) :
            property(_property), name(_name)
            {
            }

        bool write(T value) {
            /* permissions are intended as seen from cloud */
            if (permission & WRITE) {
                property = value;
                return true;
            }
            return false;
        }

        void printinfo(Stream& stream) {
            stream.println("name: " + name + " value: " + String(property) + " shadow: " + String(shadow_property) + " permission: " + String(permission));
        }

        void updateShadow() {
            shadow_property = property;
        }

        T read() {
            /* permissions are intended as seen from cloud */
            if (permission & READ) {
                return property;
            }
        }

        bool canRead() {
            return (permission & READ);
        }

        String& getName() {
            return name;
        }

        int getTag() {
            return tag;
        }

        permissionType getPermission() {
            return permission;
        }

        void appendValue(CborEncoder* mapEncoder);

        void append(CborEncoder* encoder) {
            if (!canRead()) {
                return;
            }
            CborEncoder mapEncoder;
            cbor_encoder_create_map(encoder, &mapEncoder, CborIndefiniteLength);
            if (tag != -1) {
                cbor_encode_text_stringz(&mapEncoder, "t");
                cbor_encode_int(&mapEncoder, tag);
            } else {
                cbor_encode_text_stringz(&mapEncoder, "n");
                cbor_encode_text_stringz(&mapEncoder, name.c_str());
            }
            appendValue(&mapEncoder);
            cbor_encoder_close_container(encoder, &mapEncoder);
            lastUpdated = millis();
        }

        bool newData() {
            return (property != shadow_property);
        }

        bool shouldBeUpdated() {
            if (updatePolicy == ON_CHANGE) {
                return newData();
            }
            return ((millis() - lastUpdated) > (updatePolicy * 1000)) ;
        }

        inline bool operator==(const ArduinoCloudProperty& rhs){
            return (getName() == rhs.getName());
        }

        propertyType getType();

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

#endif
