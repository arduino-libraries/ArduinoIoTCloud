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

SCENARIO("An Arduino cloud property is marked 'write only'", "[ArduinoCloudThing::encode]")
{
  PropertyContainer property_container;
  CloudInt test = 0;
  addPropertyToContainer(property_container, test, "test", Permission::Write);

  REQUIRE(cbor::encode(property_container).size() == 0); /* Since 'test' is 'write only' it should not be encoded */
}
