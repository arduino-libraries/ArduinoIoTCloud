#ifndef ARDUINO_CLOUD_THING_H_
#define ARDUINO_CLOUD_THING_H_

#include <Client.h>
#include <Stream.h>

#include "ArduinoCloudProperty.hpp"
#include "ArduinoCloudPropertyGeneric.h"

#include "lib/tinycbor/cbor-lib.h"
#include "lib/LinkedList/LinkedList.h"

enum boolStatus {
    ON = true,
    OFF = false,
};

enum times {
    SECONDS = 1,
    MINUTES = 60,
    HOURS = 3600,
    DAYS = 86400,
};

class ArduinoCloudThing {
    public:
        ArduinoCloudThing();
        void begin();
        // overload, with default arguments, of different type of properties
        ArduinoCloudPropertyGeneric& addPropertyReal(int& property, String name, permissionType _permission = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, int minDelta = 0);
        ArduinoCloudPropertyGeneric& addPropertyReal(bool& property, String name, permissionType permission = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, bool minDelta = false);
        ArduinoCloudPropertyGeneric& addPropertyReal(float& property, String name, permissionType _permission = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f);
        ArduinoCloudPropertyGeneric& addPropertyReal(String& property, String name, permissionType permission = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, String minDelta = "");
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

#endif /* ARDUINO_CLOUD_THING_H_ */
