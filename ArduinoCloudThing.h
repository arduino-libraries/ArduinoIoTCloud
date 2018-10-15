#ifndef ARDUINO_CLOUD_THING_H_
#define ARDUINO_CLOUD_THING_H_

#include <Client.h>
#include <Stream.h>

#include "ArduinoCloudProperty.hpp"
#include "ArduinoCloudPropertyContainer.hpp"

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

        ArduinoCloudProperty<bool>   & addProperty(bool   & property, String const & name, Permission const permission);
        ArduinoCloudProperty<int>    & addProperty(int    & property, String const & name, Permission const permission);
        ArduinoCloudProperty<float>  & addProperty(float  & property, String const & name, Permission const permission);
        ArduinoCloudProperty<String> & addProperty(String & property, String const & name, Permission const permission);

        // poll should return > 0 if something has changed
        int poll(uint8_t* data, size_t size);
        // decode a CBOR payload received from the Cloud.
        void decode(uint8_t * payload, size_t length);

    private:
        void update();

        bool status = OFF;
        char uuid[33];

        ArduinoCloudPropertyContainer<bool>   _bool_property_list;
        ArduinoCloudPropertyContainer<int>    _int_property_list;
        ArduinoCloudPropertyContainer<float>  _float_property_list;
        ArduinoCloudPropertyContainer<String> _string_property_list;

};

#endif /* ARDUINO_CLOUD_THING_H_ */
