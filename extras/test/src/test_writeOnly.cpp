/*
   Copyright (c) 2019 Arduino.  All rights reserved.
*/

/**************************************************************************************
   INCLUDE
 **************************************************************************************/

#include <catch.hpp>

#include <util/CBORTestUtil.h>

#include <CBORDecoder.h>
#include <PropertyContainer.h>

/**************************************************************************************
   TEST CODE
 **************************************************************************************/

SCENARIO("An Arduino cloud property is marked 'write only'", "[ArduinoCloudThing::encode]")
{
  PropertyContainer property_container;
  
  CloudInt test = 0;
  addPropertyToContainer(property_container, test, "test", Permission::Write);

  REQUIRE(cbor::encode(property_container).size() == 0); /* Since 'test' is 'write only' it should not be encoded */
}
