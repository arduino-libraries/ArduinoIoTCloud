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
#include <AIoTC_Const.h>

/******************************************************************************
  TEST CODE
 ******************************************************************************/

SCENARIO("A Arduino cloud property is published periodically", "[ArduinoCloudThing::publishEvery]")
{
  PropertyContainer property_container;
  CloudBool test = true;
  unsigned long const PUBLISH_INTERVAL_SEC = 1 * SECONDS;

  addPropertyToContainer(property_container, test, "test", Permission::ReadWrite).publishEvery(PUBLISH_INTERVAL_SEC);

  WHEN("t = 0 ms, publish interval = 1000 ms, 1st call to 'encode'") {
    set_millis(0);
    THEN("'encode' should encode the property") {
      REQUIRE(cbor::encode(property_container).size() != 0);
      WHEN("t = 999 ms") {
        set_millis(999);
        THEN("'encode' should not encode the property") {
          REQUIRE(cbor::encode(property_container).size() == 0);
          WHEN("t = 1000 ms") {
            set_millis(1000);
            THEN("'encode' should encode the property") {
              REQUIRE(cbor::encode(property_container).size() != 0);
              WHEN("t = 1999 ms") {
                set_millis(1999);
                THEN("'encode' should not encode the property") {
                  REQUIRE(cbor::encode(property_container).size() == 0);
                  WHEN("t = 2000 ms") {
                    set_millis(2000);
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
