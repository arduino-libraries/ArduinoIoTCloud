/*
   Copyright (c) 2019 Arduino.  All rights reserved.
*/

/**************************************************************************************
   INCLUDE
 **************************************************************************************/

#include <catch.hpp>

#include <util/CBORTestUtil.h>
#include <ArduinoCloudThing.h>

/**************************************************************************************
   TEST CODE
 **************************************************************************************/

SCENARIO("A Arduino cloud property is marked 'write only'", "[ArduinoCloudThing::encode]") {
  /************************************************************************************/

  PropertyContainer property_container;
  ArduinoCloudThing thing;
  thing.begin(&property_container);

  CloudInt test = 0;
  property_container.addPropertyReal(test, "test", Permission::Write);

  REQUIRE(cbor::encode(thing).size() == 0); /* Since 'test' is 'write only' it should not be encoded */

  /************************************************************************************/
}
