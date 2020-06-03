/*
   Copyright (c) 2019 Arduino.  All rights reserved.
*/

/**************************************************************************************
   INCLUDE
 **************************************************************************************/

#include <catch.hpp>

#include <TestUtil.h>
#include <ArduinoCloudThing.h>

/**************************************************************************************
   TEST CODE
 **************************************************************************************/

SCENARIO("A Arduino cloud property is published periodically", "[ArduinoCloudThing::publishEvery]") {
  /************************************************************************************/

  GIVEN("CloudProtocol::V2") {
    ArduinoCloudThing thing;
    thing.begin();

    CloudBool test = true;
    unsigned long const PUBLISH_INTERVAL_SEC = 1 * SECONDS;

    thing.addPropertyReal(test, "test", Permission::ReadWrite).publishEvery(PUBLISH_INTERVAL_SEC);

    WHEN("t = 0 ms, publish interval = 1000 ms, 1st call to 'encode'") {
      set_millis(0);
      THEN("'encode' should encode the property") {
        REQUIRE(encode(thing).size() != 0);
        WHEN("t = 999 ms") {
          set_millis(999);
          THEN("'encode' should not encode the property") {
            REQUIRE(encode(thing).size() == 0);
            WHEN("t = 1000 ms") {
              set_millis(1000);
              THEN("'encode' should encode the property") {
                REQUIRE(encode(thing).size() != 0);
                WHEN("t = 1999 ms") {
                  set_millis(1999);
                  THEN("'encode' should not encode the property") {
                    REQUIRE(encode(thing).size() == 0);
                    WHEN("t = 2000 ms") {
                      set_millis(2000);
                      THEN("'encode' should encode the property") {
                        REQUIRE(encode(thing).size() != 0);
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  /************************************************************************************/
}
