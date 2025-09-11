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
#include <CBORDecoder.h>
#include <property/types/CloudWrapperFloat.h>

/******************************************************************************
  TEST CODE
 ******************************************************************************/

SCENARIO("Arduino Cloud Properties ", "[ArduinoCloudThing::CloudWrapperFloat]")
{
  WHEN("NAN value from cloud is received")
  {
    PropertyContainer property_container;
    float value = 2.0f;
    CloudWrapperFloat prop(value);
    prop.writeOnDemand();
    addPropertyToContainer(property_container, prop, "test", Permission::ReadWrite);

    /* [{0: "test", 2: NaN}] = 81A200647465737402F97E00 */
    uint8_t const payload[] = { 0x81, 0xA2, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x02, 0xF9, 0x7E, 0x00 };
    int const payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORDecoder::decode(property_container, payload, payload_length);

    REQUIRE(prop.isDifferentFromCloud() == true);
  }

  WHEN("Local value is NAN")
  {
    PropertyContainer property_container;
    float value = NAN;
    CloudWrapperFloat prop(value);
    prop.writeOnDemand();
    addPropertyToContainer(property_container, prop, "test", Permission::ReadWrite);

    /* [{0: "test", 2: 1.5}] = 81A200647465737402F93E00 */
    uint8_t const payload[] = { 0x81, 0xA2, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x02, 0xF9, 0x3E, 0x00 };
    int const payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORDecoder::decode(property_container, payload, payload_length);

    REQUIRE(prop.isDifferentFromCloud() == true);
  }

  WHEN("both, the local and the cloud values are NaN")
  {
    PropertyContainer property_container;
    float value = NAN;
    CloudWrapperFloat prop(value);
    prop.writeOnDemand();
    addPropertyToContainer(property_container, prop, "test", Permission::ReadWrite);

    /* [{0: "test", 2: NaN}] = 81A200647465737402F97E00 */
    uint8_t const payload[] = { 0x81, 0xA2, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x02, 0xF9, 0x7E, 0x00 };
    int const payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORDecoder::decode(property_container, payload, payload_length);

    REQUIRE(prop.isDifferentFromCloud() == false);
  }

  WHEN("the local and cloud values are the same")
  {
    PropertyContainer property_container;
    float value = 1.5f;
    CloudWrapperFloat prop(value);
    prop.writeOnDemand();
    addPropertyToContainer(property_container, prop, "test", Permission::ReadWrite);

    /* [{0: "test", 2: 1.5}] = 81A200647465737402F93E00 */
    uint8_t const payload[] = { 0x81, 0xA2, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x02, 0xF9, 0x3E, 0x00 };
    int const payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORDecoder::decode(property_container, payload, payload_length);

    REQUIRE(prop.isDifferentFromCloud() == false);
  }

  WHEN("the local and the cloud values differ")
  {
    PropertyContainer property_container;
    float value = 1.0f;
    CloudWrapperFloat prop(value);
    prop.writeOnDemand();
    addPropertyToContainer(property_container, prop, "test", Permission::ReadWrite);

    /* [{0: "test", 2: 1.5}] = 81A200647465737402F97E00 */
    uint8_t const payload[] = { 0x81, 0xA2, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x02, 0xF9, 0x7E, 0x00 };
    int const payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORDecoder::decode(property_container, payload, payload_length);

    REQUIRE(prop.isDifferentFromCloud() == true);
  }

  WHEN("the local value differs by 0.25 from the cloud value")
  {
    PropertyContainer property_container;
    float value = 1.0f;
    CloudWrapperFloat prop(value);
    prop.writeOnDemand();
    addPropertyToContainer(property_container, prop, "test", Permission::ReadWrite);

    /* [{0: "test", 2: 1.5}] = 81A200647465737402F97E00 */
    uint8_t const payload[] = { 0x81, 0xA2, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x02, 0xF9, 0x7E, 0x00 };
    int const payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORDecoder::decode(property_container, payload, payload_length);
    prop.publishOnChange(0.25f, 0); // 0.25 min delta

    REQUIRE(prop.isDifferentFromCloud() == true);
  }
}
