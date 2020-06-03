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

SCENARIO("A Arduino cloud property is marked 'read only'", "[ArduinoCloudThing::decode]") {
  /************************************************************************************/

  GIVEN("CloudProtocol::V2") {
    ArduinoCloudThing thing;
    thing.begin();

    CloudInt test = 0;
    thing.addPropertyReal(test, "test", Permission::Read);

    /* [{0: "test", 2: 7}] = 81 A2 00 64 74 65 73 74 02 07 */
    uint8_t const payload[] = {0x81, 0xA2, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x02, 0x07};
    int const payload_length = sizeof(payload) / sizeof(uint8_t);
    thing.decode(payload, payload_length);

    REQUIRE(test == 0);
  }

  /************************************************************************************/
}
