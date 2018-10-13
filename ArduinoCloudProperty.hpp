#ifndef ARDUINO_CLOUD_PROPERTY_H_
#define ARDUINO_CLOUD_PROPERTY_H_

#include "ArduinoCloudPropertyGeneric.h"

class ArduinoCloudThing; /* Forward declaration to satisfy compiler for this line "friend ArduinoCloudThing" */

// definition of the default property update policy
static const int ON_CHANGE = -1;

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

#endif /* ARDUINO_CLOUD_PROPERTY_H_ */
