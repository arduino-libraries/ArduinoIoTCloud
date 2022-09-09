/*
   Copyright (c) 2019 Arduino.  All rights reserved.
*/

/**************************************************************************************
   INCLUDE
 **************************************************************************************/

#include <catch.hpp>

#include <util/CBORTestUtil.h>

/**************************************************************************************
   TEST CODE
 **************************************************************************************/

SCENARIO("An Arduino cloud property is published on value change but the update rate is limited", "[ArduinoCloudThing::publishOnChange]")
{
  PropertyContainer property_container;
    
  CloudInt test = 0;
  int const MIN_DELTA = 0;
  unsigned long const MIN_TIME_BETWEEN_UPDATES_ms = 500; /* No updates faster than 500 ms */

  addPropertyToContainer(property_container, test, "test", Permission::ReadWrite).publishOnChange(MIN_DELTA, MIN_TIME_BETWEEN_UPDATES_ms);

  WHEN("t = 0 ms, min time between updates = 500 ms, property not modified, 1st call to 'encode'") {
    set_millis(0);
    THEN("'encode' should encode the property") {
      REQUIRE(cbor::encode(property_container).size() != 0);
      WHEN("t = 499 ms, property modified") {
        test++;
        set_millis(499);
        THEN("'encode' should not encode any property") {
          REQUIRE(cbor::encode(property_container).size() == 0);
          WHEN("t = 500 ms, property modified") {
            test++;
            set_millis(500);
            THEN("'encode' should encode the property") {
              REQUIRE(cbor::encode(property_container).size() != 0);
              WHEN("t = 999 ms, property modified") {
                test++;
                set_millis(999);
                THEN("'encode' should not encode any property") {
                  REQUIRE(cbor::encode(property_container).size() == 0);
                  WHEN("t = 1000 ms, property modified") {
                    test++;
                    set_millis(1000);
                    THEN("'encode' should encode the property") {
                      REQUIRE(cbor::encode(property_container).size() != 0);
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
