#ifndef ArduinoCloudThing_h
#define ArduinoCloudThing_h

#include <Client.h>
#include <Stream.h>
#include "lib/LinkedList/LinkedList.h"
#include "lib/ArduinoCbor/src/ArduinoCbor.h"

//#define TESTING_PROTOCOL
#define DEBUG_MEMORY
#define USE_ARDUINO_CLOUD

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
    virtual void append(CborObject& object) = 0;
    virtual String& getName() = 0;
    virtual void setName(String _name) = 0;
    virtual ArduinoCloudPropertyGeneric& setTag(int _tag) = 0;
    virtual ArduinoCloudPropertyGeneric& readOnly() = 0;
    virtual ArduinoCloudPropertyGeneric& writeOnly() = 0;
    virtual int getTag() = 0;
    virtual ArduinoCloudPropertyGeneric& setPermission(permissionType _permission) = 0;
    virtual permissionType getPermission() = 0;
    virtual bool newData() = 0;
    virtual bool shouldBeUpdated() = 0;
    virtual void updateShadow() = 0;
    virtual bool canRead() = 0;
    virtual void printinfo() = 0;
    virtual ArduinoCloudPropertyGeneric& onUpdate(void(*fn)(void)) = 0;
    virtual ArduinoCloudPropertyGeneric& publishEvery(long seconds) = 0;
    void(*callback)(void) = NULL;
};

template <typename T>
class ArduinoCloudProperty : public ArduinoCloudPropertyGeneric
{
public:
    ArduinoCloudProperty(T& _property, String _name) :
        property(_property), name(_name) {}

    bool write(T value) {
        /* permissions are intended as seen from cloud */
        if (permission & WRITE) {
            property = value;
            return true;
        }
        return false;
    }

    void printinfo() {
        Serial.println("name: " + name + " value: " + String(property) + " shadow: " + String(shadow_property) + " permission: " + String(permission));
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

    permissionType getPermission() {
        return permission;
    }

    ArduinoCloudPropertyGeneric& onUpdate(void(*fn)(void)) {
        callback = fn;
        return *(reinterpret_cast<ArduinoCloudPropertyGeneric*>(this));
    }

    void appendValue(CborObject &cbor);

    void append(CborObject &cbor) {
        if (!canRead()) {
            return;
        }
        if (tag != -1) {
            cbor.set("t", tag);
        } else {
            cbor.set("n", name.c_str());
        }
        appendValue(cbor);
        cbor.set("p", permission);
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
        return (millis() - lastUpdated > updatePolicy * 1000) ;
    }

    inline bool operator==(const ArduinoCloudProperty& rhs){
        return (strcmp(getName(), rhs.getName) == 0);
    }

protected:
    T& property;
    T shadow_property;
    String name;
    int tag = -1;
    long lastUpdated = 0;
    long updatePolicy = ON_CHANGE;
    permissionType permission = READWRITE;
    static int tagIndex;
};

template <>
inline void ArduinoCloudProperty<int>::appendValue(CborObject &cbor) {
    cbor.set("i", property);
};

template <>
inline void ArduinoCloudProperty<bool>::appendValue(CborObject &cbor) {
    cbor.set("b", property);
};

template <>
inline void ArduinoCloudProperty<float>::appendValue(CborObject &cbor) {
    cbor.set("f", property);
};

template <>
inline void ArduinoCloudProperty<String>::appendValue(CborObject &cbor) {
    cbor.set("s", property.c_str());
};

template <>
inline void ArduinoCloudProperty<char*>::appendValue(CborObject &cbor) {
    cbor.set("s", property);
};

class ArduinoCloudThing {
public:
    ArduinoCloudThing();
    void begin();
    ArduinoCloudPropertyGeneric& addPropertyReal(int& property, String name);
    ArduinoCloudPropertyGeneric& addPropertyReal(bool& property, String name);
    ArduinoCloudPropertyGeneric& addPropertyReal(float& property, String name);
    ArduinoCloudPropertyGeneric& addPropertyReal(void* property, String name);
    ArduinoCloudPropertyGeneric& addPropertyReal(String property, String name);
    // poll should return > 0 if something has changed
    int poll(uint8_t* data);
    void decode(uint8_t * payload, size_t length);

private:
    int publish(CborArray& object, uint8_t* data);

    void update();
    int checkNewData();
    void compress(CborArray& object, CborBuffer& buffer);

    ArduinoCloudPropertyGeneric* exists(String &name);

    bool status = OFF;
    char uuid[33];

    LinkedList<ArduinoCloudPropertyGeneric*> list;
    int currentListIndex = -1;
};

#endif
