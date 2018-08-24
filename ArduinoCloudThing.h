#ifndef ArduinoCloudThing_h
#define ArduinoCloudThing_h

#include <Client.h>
#include <Stream.h>
#include "lib/LinkedList/LinkedList.h"
#include "lib/tinycbor/cbor.h"

enum permissionType {
    READ    = 0b01,
    WRITE   = 0b10,
    READWRITE = READ|WRITE,
};

enum boolStatus {
    ON,
    OFF,
};

#define ON_CHANGE   -1

enum times {
    SECONDS = 1,
    MINUTES = 60,
    HOURS = 3600,
    DAYS = 86400,
};

class ArduinoCloudPropertyGeneric
{
public:
    virtual void append(CborEncoder* encoder) = 0;
    virtual String& getName() = 0;
    virtual void setName(String _name) = 0;
    virtual ArduinoCloudPropertyGeneric& setTag(int _tag) = 0;
    virtual ArduinoCloudPropertyGeneric& readOnly() = 0;
    virtual ArduinoCloudPropertyGeneric& writeOnly() = 0;
    virtual ArduinoCloudPropertyGeneric& minimumDelta(void* delta) = 0;
    virtual int getTag() = 0;
    virtual ArduinoCloudPropertyGeneric& setPermission(permissionType _permission) = 0;
    virtual permissionType getPermission() = 0;
    virtual bool newData() = 0;
    virtual bool shouldBeUpdated() = 0;
    virtual void updateShadow() = 0;
    virtual bool canRead() = 0;
    virtual void printinfo(Stream& stream) = 0;
    virtual ArduinoCloudPropertyGeneric& onUpdate(void(*fn)(void)) = 0;
    virtual ArduinoCloudPropertyGeneric& publishEvery(long seconds) = 0;
    void(*callback)(void) = NULL;
};

class ArduinoCloudThing {
public:
    ArduinoCloudThing();
    void begin();
    ArduinoCloudPropertyGeneric& addPropertyReal(int& property, String name);
    ArduinoCloudPropertyGeneric& addPropertyReal(bool& property, String name);
    ArduinoCloudPropertyGeneric& addPropertyReal(float& property, String name);
    ArduinoCloudPropertyGeneric& addPropertyReal(void* property, String name);
    ArduinoCloudPropertyGeneric& addPropertyReal(String& property, String name);
    // poll should return > 0 if something has changed
    int poll(uint8_t* data, size_t size);
    void decode(uint8_t * payload, size_t length);

private:
    void update();
    int checkNewData();
    int findPropertyByName(String &name);

    ArduinoCloudPropertyGeneric* exists(String &name);

    bool status = OFF;
    char uuid[33];

    LinkedList<ArduinoCloudPropertyGeneric*> list;
    int currentListIndex = -1;
};

template <typename T>
class ArduinoCloudProperty : public ArduinoCloudPropertyGeneric
{
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

    void setName(String _name) {
        name = _name;
    }

    ArduinoCloudPropertyGeneric& setTag(int _tag) {
        tag = _tag;
        return *(reinterpret_cast<ArduinoCloudPropertyGeneric*>(this));
    }

    int getTag() {
        return tag;
    }

    ArduinoCloudPropertyGeneric& setPermission(permissionType _permission) {
        permission = _permission;
        return *(reinterpret_cast<ArduinoCloudPropertyGeneric*>(this));
    }

    ArduinoCloudPropertyGeneric& readOnly() {
        permission = READ;
        return *(reinterpret_cast<ArduinoCloudPropertyGeneric*>(this));
    }

    ArduinoCloudPropertyGeneric& writeOnly() {
        permission = WRITE;
        return *(reinterpret_cast<ArduinoCloudPropertyGeneric*>(this));
    }

    ArduinoCloudPropertyGeneric& minimumDelta(void* delta) {
        minDelta = *(T*)delta;
        return *(reinterpret_cast<ArduinoCloudPropertyGeneric*>(this));
    }

    permissionType getPermission() {
        return permission;
    }

    ArduinoCloudPropertyGeneric& onUpdate(void(*fn)(void)) {
        callback = fn;
        return *(reinterpret_cast<ArduinoCloudPropertyGeneric*>(this));
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

    ArduinoCloudPropertyGeneric& publishEvery(long seconds) {
        updatePolicy = seconds;
        return *(reinterpret_cast<ArduinoCloudPropertyGeneric*>(this));
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
        return (strcmp(getName(), rhs.getName()) == 0);
    }


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

template <>
inline void ArduinoCloudProperty<int>::appendValue(CborEncoder* mapEncoder) {
    cbor_encode_text_stringz(mapEncoder, "v");
    cbor_encode_int(mapEncoder, property);
};

template <>
inline void ArduinoCloudProperty<bool>::appendValue(CborEncoder* mapEncoder) {
    cbor_encode_text_stringz(mapEncoder, "v");
    cbor_encode_boolean(mapEncoder, property);
};

template <>
inline void ArduinoCloudProperty<float>::appendValue(CborEncoder* mapEncoder) {
    cbor_encode_text_stringz(mapEncoder, "v");
    cbor_encode_float(mapEncoder, property);
};

template <>
inline void ArduinoCloudProperty<String>::appendValue(CborEncoder* mapEncoder) {
    cbor_encode_text_stringz(mapEncoder, "v");
    cbor_encode_text_stringz(mapEncoder, property.c_str());
};

template <>
inline void ArduinoCloudProperty<String*>::appendValue(CborEncoder* mapEncoder) {
    cbor_encode_text_stringz(mapEncoder, "v");
    cbor_encode_text_stringz(mapEncoder, property->c_str());
};

template <>
inline void ArduinoCloudProperty<char*>::appendValue(CborEncoder* mapEncoder) {
    cbor_encode_text_stringz(mapEncoder, "v");
    cbor_encode_text_stringz(mapEncoder, property);
};

#endif
