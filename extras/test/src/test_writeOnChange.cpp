/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2019 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/******************************************************************************
  INCLUDE
 ******************************************************************************/

#include <catch2/catch_test_macros.hpp>

#include <util/CBORTestUtil.h>

#include <CBORDecoder.h>
#include <PropertyContainer.h>

/******************************************************************************
  TEST CODE
 ******************************************************************************/

SCENARIO("An Arduino cloud property is marked 'write on change'", "[ArduinoCloudThing::decode]")
{
  PropertyContainer property_container;
  CloudInt test = 0;
  addPropertyToContainer(property_container, test, "test", Permission::ReadWrite).writeOnChange();

  /* [{0: "test", 2: 7}] = 81 A2 00 64 74 65 73 74 02 07 */
  uint8_t const payload[] = {0x81, 0xA2, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x02, 0x07};
  int const payload_length = sizeof(payload) / sizeof(uint8_t);
  CBORDecoder::decode(property_container, payload, payload_length);

  REQUIRE(test == 7);
}
