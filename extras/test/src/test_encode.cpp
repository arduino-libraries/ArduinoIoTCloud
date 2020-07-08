/*
   Copyright (c) 2019 Arduino.  All rights reserved.
*/

/**************************************************************************************
   INCLUDE
 **************************************************************************************/

#include <catch.hpp>

#include <memory>

#include <util/CBORTestUtil.h>
#include "types/CloudWrapperBool.h"
#include "types/CloudWrapperFloat.h"
#include "types/CloudWrapperInt.h"
#include "types/CloudWrapperString.h"

/**************************************************************************************
   TEST CODE
 **************************************************************************************/

SCENARIO("Arduino Cloud Properties are encoded", "[ArduinoCloudThing::encode-1]") {
  /************************************************************************************/

  WHEN("A 'bool' property is added")
  {
    PropertyContainer property_container;
    cbor::encode(property_container);

    CloudBool test = true;
    addPropertyToContainer(property_container, test, "test", Permission::ReadWrite);

    /* [{0: "test", 4: true}] = 9F A2 00 64 74 65 73 74 04 F5 FF*/
    std::vector<uint8_t> const expected = {0x9F, 0xA2, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x04, 0xF5, 0xFF};
    std::vector<uint8_t> const actual = cbor::encode(property_container);
    REQUIRE(actual == expected);
  }

  /************************************************************************************/

  WHEN("A 'bool' property is added - light payload")
  {
    /*An integer identifier must be instead of the name of the property in order to have a shorter payload*/
    PropertyContainer property_container;
    cbor::encode(property_container);

    CloudBool test = true;
    /*The property is added with identifier 1 that will be used instead of the string "test" as property identifier*/
    addPropertyToContainer(property_container, test, "test", Permission::ReadWrite, 1);

    /* [{0: 1, 4: true}] = 9F A2 00 01 04 F5 FF*/
    std::vector<uint8_t> const expected = {0x9F, 0xA2, 0x00, 0x01, 0x04, 0xF5, 0xFF};
    std::vector<uint8_t> const actual = cbor::encode(property_container, true);
    REQUIRE(actual == expected);
  }

  /************************************************************************************/

  WHEN("A 'int' property is added")
  {
    PropertyContainer property_container;
    cbor::encode(property_container);

    CloudInt int_test = 123;
    addPropertyToContainer(property_container, int_test, "test", Permission::ReadWrite);

    /* [{0: "test", 3: 123}] = 9F A2 00 64 74 65 73 74 02 18 7B FF */
    std::vector<uint8_t> const expected = {0x9F, 0xA2, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x02, 0x18, 0x7B, 0xFF};
    std::vector<uint8_t> const actual = cbor::encode(property_container);
    REQUIRE(actual == expected);
  }

  /************************************************************************************/

  WHEN("A 'float' property is added")
  {
    PropertyContainer property_container;
    cbor::encode(property_container);

    CloudFloat float_test = 3.14159;
    addPropertyToContainer(property_container, float_test, "test", Permission::ReadWrite);

    /* [{0: "test", 2: 3.141590118408203}] = 9F A2 00 64 74 65 73 74 02 FA 40 49 0F D0 FF */
    std::vector<uint8_t> const expected = {0x9F, 0xA2, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x02, 0xFA, 0x40, 0x49, 0x0F, 0xD0, 0xFF};
    std::vector<uint8_t> const actual = cbor::encode(property_container);
    REQUIRE(actual == expected);
  }

  /************************************************************************************/

  WHEN("A 'String' property is added")
  {
    PropertyContainer property_container;
    cbor::encode(property_container);

    CloudString string_test;
    string_test = "test";
    addPropertyToContainer(property_container, string_test, "test", Permission::ReadWrite);

    /* [{0: "test", 3: "test"}] = 9F A2 00 64 74 65 73 74 03 64 74 65 73 74 FF*/
    std::vector<uint8_t> const expected = {0x9F, 0xA2, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x03, 0x64, 0x74, 0x65, 0x73, 0x74, 0xFF};
    std::vector<uint8_t> const actual = cbor::encode(property_container);
    REQUIRE(actual == expected);
  }

  /************************************************************************************/

  WHEN("A 'Location' property is added")
  {
    PropertyContainer property_container;
    cbor::encode(property_container);

    CloudLocation location_test = CloudLocation(2.0f, 3.0f);
    addPropertyToContainer(property_container, location_test, "test", Permission::ReadWrite);

    /* [{0: "test:lat", 3: 2},{0: "test:lon", 3: 3}] = 9F A2 00 68 74 65 73 74 3A 6C 61 74 02 FA 40 00 00 00 A2 00 68 74 65 73 74 3A 6C 6F 6E 02 FA 40 40 00 00 FF*/
    std::vector<uint8_t> const expected = { 0x9F, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x6C, 0x61, 0x74, 0x02, 0xFA, 0x40, 0x00, 0x00, 0x00, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x6C, 0x6F, 0x6E, 0x02, 0xFA, 0x40, 0x40, 0x00, 0x00, 0xFF };
    std::vector<uint8_t> const actual = cbor::encode(property_container);
    REQUIRE(actual == expected);
  }

  /************************************************************************************/

  WHEN("A 'Color' property is added")
  {
    PropertyContainer property_container;
    cbor::encode(property_container);

    CloudColor color_test = CloudColor(2.0, 2.0, 2.0);
    addPropertyToContainer(property_container, color_test, "test", Permission::ReadWrite);

    /* [{0: "test:hue", 2: 2.0},{0: "test:sat", 2: 2.0},{0: "test:bri", 2: 2.0}] = 9F A2 00 68 74 65 73 74 3A 68 75 65 02 FA 40 00 00 00 A2 00 68 74 65 73 74 3A 73 61 74 02 FA 40 00 00 00 A2 00 68 74 65 73 74 3A 62 72 69 02 FA 40 00 00 00 FF*/
    std::vector<uint8_t> const expected = {0x9F, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x68, 0x75, 0x65, 0x02, 0xFA, 0x40, 0x00, 0x00, 0x00, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x73, 0x61, 0x74, 0x02, 0xFA, 0x40, 0x00, 0x00, 0x00, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x62, 0x72, 0x69, 0x02, 0xFA, 0x40, 0x00, 0x00, 0x00, 0xFF };
    std::vector<uint8_t> const actual = cbor::encode(property_container);
    REQUIRE(actual == expected);
  }

  /************************************************************************************/

  WHEN("A 'Color' property is added - light payload")
  {
    /*An integer identifier must be encoded instead of the name of the property in order to have a shorter payload*/
    PropertyContainer property_container;
    cbor::encode(property_container);

    CloudColor color_test = CloudColor(2.0, 2.0, 2.0);
    /*The property is added with identifier 1 that will be used instead of the string "name" as property identifier */
    addPropertyToContainer(property_container, color_test, "test", Permission::ReadWrite, 1);

    /* [{0: 257, 2: 2.0},{0: 513, 2: 2.0},{0: 769, 2: 2.0}] = 9F A2 00 19 01 01 02 FA 40 00 00 00 A2 00 19 02 01 02 FA 40 00 00 00 A2 00 19 03 01 02 FA 40 00 00 00 FF*/
    std::vector<uint8_t> const expected = {0x9F, 0xA2, 0x00, 0x19, 0x01, 0x01, 0x02, 0xFA, 0x40, 0x00, 0x00, 0x00, 0xA2, 0x00, 0x19, 0x02, 0x01, 0x02, 0xFA, 0x40, 0x00, 0x00, 0x00, 0xA2, 0x00, 0x19, 0x03, 0x01, 0x02, 0xFA, 0x40, 0x00, 0x00, 0x00, 0xFF };
    std::vector<uint8_t> const actual = cbor::encode(property_container, true);
    REQUIRE(actual == expected);
  }

  /************************************************************************************/

  WHEN("A 'ColoredLight' property is added")
  {
    PropertyContainer property_container;
    cbor::encode(property_container);

    CloudColoredLight color_test = CloudColoredLight(true, 2.0, 2.0, 2.0);
    addPropertyToContainer(property_container, color_test, "test", Permission::ReadWrite);

    /* [{0: "test:swi", 4: true},{0: "test:hue", 2: 2.0},{0: "test:sat", 2: 2.0},{0: "test:bri", 2: 2.0}] = 83 A2 00 68 74 65 73 74 3A 73 77 69 04 F5 //A2 00 68 74 65 73 74 3A 68 75 65 02 FA 40 00 00 00 A2 00 68 74 65 73 74 3A 73 61 74 02 FA 40 00 00 00 A2 00 68 74 65 73 74 3A 62 72 69 02 FA 40 00 00 00 FF*/
    std::vector<uint8_t> const expected = {0x9F, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x73, 0x77, 0x69, 0x04, 0xF5, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x68, 0x75, 0x65, 0x02, 0xFA, 0x40, 0x00, 0x00, 0x00, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x73, 0x61, 0x74, 0x02, 0xFA, 0x40, 0x00, 0x00, 0x00, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x62, 0x72, 0x69, 0x02, 0xFA, 0x40, 0x00, 0x00, 0x00, 0xFF };
    std::vector<uint8_t> const actual = cbor::encode(property_container);
    REQUIRE(actual == expected);
  }

  /************************************************************************************/

  WHEN("A 'Television' property is added")
  {
    PropertyContainer property_container;
    cbor::encode(property_container);

    CloudTelevision tv_test = CloudTelevision(true, 50, false, PlaybackCommands::Play, InputValue::TV, 7);
    addPropertyToContainer(property_container, tv_test, "test", Permission::ReadWrite);

    /* [{0: "test:swi", 4: true},{0: "test:vol", 2: 50},{0: "test:mut", 2: false},{0: "test:pbc", 2: 3},{0: "test:inp", 2: 55},{0: "test:cha", 2: 7}] = 9F A2 00 68 74 65 73 74 3A 73 77 69 04 F5 A2 00 68 74 65 73 74 3A 76 6F 6C 02 18 32 A2 00 68 74 65 73 74 3A 6D 75 74 04 F4 A2 00 68 74 65 73 74 3A 70 62 63 02 03 A2 00 68 74 65 73 74 3A 69 6E 70 02 18 37 A2 00 68 74 65 73 74 3A 63 68 61 02 07 FF */
    std::vector<uint8_t> const expected = {0x9F, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x73, 0x77, 0x69, 0x04, 0xF5, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x76, 0x6F, 0x6C, 0x02, 0x18, 0x32, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x6D, 0x75, 0x74, 0x04, 0xF4, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x70, 0x62, 0x63, 0x02, 0x03, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x69, 0x6E, 0x70, 0x02, 0x18, 0x37, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x63, 0x68, 0x61, 0x02, 0x07, 0xFF};
    std::vector<uint8_t> const actual = cbor::encode(property_container);
    REQUIRE(actual == expected);
  }

  /************************************************************************************/

  WHEN("A 'DimmedLight' property is added")
  {
    PropertyContainer property_container;
    cbor::encode(property_container);

    CloudDimmedLight color_test = CloudDimmedLight(true, 2.0);
    addPropertyToContainer(property_container, color_test, "test", Permission::ReadWrite);

    /* [{0: "test:swi", 4: true},{0: "test:hue", 2: 0.0},{0: "test:sat", 2: 0.0},{0: "test:bri", 2: 2.0}] = 83 A2 00 68 74 65 73 74 3A 73 77 69 04 F5 //A2 00 68 74 65 73 74 3A 68 75 65 02 FA 00 00 00 00 A2 00 68 74 65 73 74 3A 73 61 74 02 FA 00 00 00 00 A2 00 68 74 65 73 74 3A 62 72 69 02 FA 40 00 00 00 FF*/
    std::vector<uint8_t> const expected = {0x9F, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x73, 0x77, 0x69, 0x04, 0xF5, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x68, 0x75, 0x65, 0x02, 0xFA, 0x00, 0x00, 0x00, 0x00, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x73, 0x61, 0x74, 0x02, 0xFA, 0x00, 0x00, 0x00, 0x00, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x62, 0x72, 0x69, 0x02, 0xFA, 0x40, 0x00, 0x00, 0x00, 0xFF };
    std::vector<uint8_t> const actual = cbor::encode(property_container);
    REQUIRE(actual == expected);
  }

  /************************************************************************************/

  WHEN("A light property is added")
  {
    PropertyContainer property_container;
    cbor::encode(property_container);

    CloudLight test;
    test = true;
    addPropertyToContainer(property_container, test, "test", Permission::ReadWrite);

    /* [{0: "test", 4: true}] = 9F A2 00 64 74 65 73 74 04 F5 FF*/
    std::vector<uint8_t> const expected = {0x9F, 0xA2, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x04, 0xF5, 0xFF};
    std::vector<uint8_t> const actual = cbor::encode(property_container);
    REQUIRE(actual == expected);
  }

  /************************************************************************************/

  WHEN("A contact sensor property is added")
  {
    PropertyContainer property_container;
    cbor::encode(property_container);

    CloudContactSensor test;
    test = true;
    addPropertyToContainer(property_container, test, "test", Permission::ReadWrite);

    /* [{0: "test", 4: true}] = 9F A2 00 64 74 65 73 74 04 F5 FF*/
    std::vector<uint8_t> const expected = {0x9F, 0xA2, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x04, 0xF5, 0xFF};
    std::vector<uint8_t> const actual = cbor::encode(property_container);
    REQUIRE(actual == expected);
  }

  /************************************************************************************/

  WHEN("A motion sensor property is added")
  {
    PropertyContainer property_container;
    cbor::encode(property_container);

    CloudMotionSensor test;
    test = true;
    addPropertyToContainer(property_container, test, "test", Permission::ReadWrite);

    /* [{0: "test", 4: true}] = 9F A2 00 64 74 65 73 74 04 F5 FF*/
    std::vector<uint8_t> const expected = {0x9F, 0xA2, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x04, 0xF5, 0xFF};
    std::vector<uint8_t> const actual = cbor::encode(property_container);
    REQUIRE(actual == expected);
  }

  /************************************************************************************/

  WHEN("A smart plug property is added")
  {
    PropertyContainer property_container;
    cbor::encode(property_container);

    CloudSmartPlug test;
    test = true;
    addPropertyToContainer(property_container, test, "test", Permission::ReadWrite);

    /* [{0: "test", 4: true}] = 9F A2 00 64 74 65 73 74 04 F5 FF*/
    std::vector<uint8_t> const expected = {0x9F, 0xA2, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x04, 0xF5, 0xFF};
    std::vector<uint8_t> const actual = cbor::encode(property_container);
    REQUIRE(actual == expected);
  }

  /************************************************************************************/

  WHEN("A Temperature property is added")
  {
    PropertyContainer property_container;
    cbor::encode(property_container);

    CloudTemperature float_test;
    float_test = 3.14159;
    addPropertyToContainer(property_container, float_test, "test", Permission::ReadWrite);

    /* [{0: "test", 2: 3.141590118408203}] = 9F A2 00 64 74 65 73 74 02 FA 40 49 0F D0 FF */
    std::vector<uint8_t> const expected = {0x9F, 0xA2, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x02, 0xFA, 0x40, 0x49, 0x0F, 0xD0, 0xFF};
    std::vector<uint8_t> const actual = cbor::encode(property_container);
    REQUIRE(actual == expected);
  }

  /************************************************************************************/

  WHEN("A switch property is added")
  {
    PropertyContainer property_container;
    cbor::encode(property_container);

    CloudSwitch test;
    test = true;
    addPropertyToContainer(property_container, test, "test", Permission::ReadWrite);

    /* [{0: "test", 4: true}] = 9F A2 00 64 74 65 73 74 04 F5 FF*/
    std::vector<uint8_t> const expected = {0x9F, 0xA2, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x04, 0xF5, 0xFF};
    std::vector<uint8_t> const actual = cbor::encode(property_container);
    REQUIRE(actual == expected);
  }

  /************************************************************************************/

  WHEN("Multiple properties are added")
  {
    PropertyContainer property_container;
        
    CloudInt    int_test = 1;
    CloudBool   bool_test = false;
    CloudFloat  float_test = 2.0f;
    CloudString str_test;
    str_test = "str_test";

    addPropertyToContainer(property_container, int_test,   "int_test",   Permission::ReadWrite);
    addPropertyToContainer(property_container, bool_test,  "bool_test",  Permission::ReadWrite);
    addPropertyToContainer(property_container, float_test, "float_test", Permission::ReadWrite);
    addPropertyToContainer(property_container, str_test,   "str_test",   Permission::ReadWrite);

    /* [{0: "int_test", 2: 1}, {0: "bool_test", 4: false},  {0: "float_test", 2: 2.0}, {0: "str_test", 3: "str_test"}]
       = 9F A2 00 68 69 6E 74 5F 74 65 73 74 02 01 A2 00 6A 66 6C 6F 61 74 5F 74 65 73 74 02 FA A2 00 6A 66 6C 6F 61 74 5F 74 65 73 74 02 FA 40 00 00 00 A2 00 68 73 74 72 5F 74 65 73 74 03 68 73 74 72 5F 74 65 73 74 FF
    */
    std::vector<uint8_t> const expected = {0x9F, 0xA2, 0x00, 0x68, 0x69, 0x6E, 0x74, 0x5F, 0x74, 0x65, 0x73, 0x74, 0x02, 0x01, 0xA2, 0x00, 0x69, 0x62, 0x6F, 0x6F, 0x6C, 0x5F, 0x74, 0x65, 0x73, 0x74, 0x04, 0xF4, 0xA2, 0x00, 0x6A, 0x66, 0x6C, 0x6F, 0x61, 0x74, 0x5F, 0x74, 0x65, 0x73, 0x74, 0x02, 0xFA, 0x40, 0x00, 0x00, 0x00, 0xA2, 0x00, 0x68, 0x73, 0x74, 0x72, 0x5F, 0x74, 0x65, 0x73, 0x74, 0x03, 0x68, 0x73, 0x74, 0x72, 0x5F, 0x74, 0x65, 0x73, 0x74, 0xFF};
    std::vector<uint8_t> const actual = cbor::encode(property_container);
    REQUIRE(actual == expected);
  }

  /************************************************************************************/

  WHEN("Multiple primitive properties are added")
  {
    PropertyContainer property_container;
        
    int    int_test = 1;
    bool   bool_test = false;
    float  float_test = 2.0f;
    String str_test;
    str_test = "str_test";

    std::unique_ptr<Property> i(new CloudWrapperInt(int_test));
    std::unique_ptr<Property> b(new CloudWrapperBool(bool_test));
    std::unique_ptr<Property> f(new CloudWrapperFloat(float_test));
    std::unique_ptr<Property> s(new CloudWrapperString(str_test));

    addPropertyToContainer(property_container, *i, "int_test",   Permission::ReadWrite);
    addPropertyToContainer(property_container, *b, "bool_test",  Permission::ReadWrite);
    addPropertyToContainer(property_container, *f, "float_test", Permission::ReadWrite);
    addPropertyToContainer(property_container, *s, "str_test",   Permission::ReadWrite);

    updateTimestampOnLocallyChangedProperties(property_container);

    /* [{0: "int_test", 2: 1}, {0: "bool_test", 4: false},  {0: "float_test", 2: 2.0}, {0: "str_test", 3: "str_test"}]
       = 9F A2 00 68 69 6E 74 5F 74 65 73 74 02 01 A2 00 6A 66 6C 6F 61 74 5F 74 65 73 74 02 FA A2 00 6A 66 6C 6F 61 74 5F 74 65 73 74 02 FA 40 00 00 00 A2 00 68 73 74 72 5F 74 65 73 74 03 68 73 74 72 5F 74 65 73 74 FF
    */
    std::vector<uint8_t> const expected = {0x9F, 0xA2, 0x00, 0x68, 0x69, 0x6E, 0x74, 0x5F, 0x74, 0x65, 0x73, 0x74, 0x02, 0x01, 0xA2, 0x00, 0x69, 0x62, 0x6F, 0x6F, 0x6C, 0x5F, 0x74, 0x65, 0x73, 0x74, 0x04, 0xF4, 0xA2, 0x00, 0x6A, 0x66, 0x6C, 0x6F, 0x61, 0x74, 0x5F, 0x74, 0x65, 0x73, 0x74, 0x02, 0xFA, 0x40, 0x00, 0x00, 0x00, 0xA2, 0x00, 0x68, 0x73, 0x74, 0x72, 0x5F, 0x74, 0x65, 0x73, 0x74, 0x03, 0x68, 0x73, 0x74, 0x72, 0x5F, 0x74, 0x65, 0x73, 0x74, 0xFF};
    std::vector<uint8_t> const actual = cbor::encode(property_container);
    REQUIRE(actual == expected);
  }

  /************************************************************************************/

  WHEN("The size of the encoded properties is exceeding the CBOR buffer size")
  {
    PropertyContainer property_container;

    CloudString str_0; str_0 = "This string is 30 bytes long.";
    CloudString str_1; str_1 = "This string is 30 bytes long.";
    CloudString str_2; str_2 = "This string is 30 bytes long.";
    CloudString str_3; str_3 = "This string is 30 bytes long.";
    CloudString str_4; str_4 = "This string is 30 bytes long.";
    CloudString str_5; str_5 = "This string is 30 bytes long.";
    CloudString str_6; str_6 = "This string is 30 bytes long.";
    CloudString str_7; str_7 = "This string is 30 bytes long.";
    CloudString str_8; str_8 = "This string is 30 bytes long.";
    CloudString str_9; str_9 = "This string is 30 bytes long.";

    addPropertyToContainer(property_container, str_0, "str_0", Permission::ReadWrite);
    addPropertyToContainer(property_container, str_1, "str_1", Permission::ReadWrite);
    addPropertyToContainer(property_container, str_2, "str_2", Permission::ReadWrite);
    addPropertyToContainer(property_container, str_3, "str_3", Permission::ReadWrite);
    addPropertyToContainer(property_container, str_4, "str_4", Permission::ReadWrite);
    addPropertyToContainer(property_container, str_5, "str_5", Permission::ReadWrite);
    addPropertyToContainer(property_container, str_6, "str_6", Permission::ReadWrite);
    addPropertyToContainer(property_container, str_7, "str_7", Permission::ReadWrite);
    addPropertyToContainer(property_container, str_8, "str_8", Permission::ReadWrite);
    addPropertyToContainer(property_container, str_9, "str_9", Permission::ReadWrite);

    /* Due to the size if the encoded properties exceeding 256 bytes if encoded all at
     * once they are encoded in subsequent calls to CBOREncoder::encode.
     */

    /* [{0: "str_0", 3: "This string is 30 bytes long."}, {0: "str_1", 3: "This string is 30 bytes long."}, {0: "str_2", 3: "This string is 30 bytes long."}, {0: "str_3", 3: "This string is 30 bytes long."}]
     * = 9F A2 00 65 73 74 72 5F 30 03 78 1D 54 68 69 73 20 73 74 72 69 6E 67 20 69 73 20 33 30 20 62 79 74 65 73 20 6C 6F 6E 67 2E A2 00 65 73 74 72 5F 31 03 78 1D 54 68 69 73 20 73 74 72 69 6E 67 20 69 73 20 33 30 20 62 79 74 65 73 20 6C 6F 6E 67 2E A2 00 65 73 74 72 5F 32 03 78 1D 54 68 69 73 20 73 74 72 69 6E 67 20 69 73 20 33 30 20 62 79 74 65 73 20 6C 6F 6E 67 2E A2 00 65 73 74 72 5F 33 03 78 1D 54 68 69 73 20 73 74 72 69 6E 67 20 69 73 20 33 30 20 62 79 74 65 73 20 6C 6F 6E 67 2E FF
     */
    std::vector<uint8_t> const expected_1 = {0x9F, 0xA2, 0x00, 0x65, 0x73, 0x74, 0x72, 0x5F, 0x30, 0x03, 0x78, 0x1D, 0x54, 0x68, 0x69, 0x73, 0x20, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x69, 0x73, 0x20, 0x33, 0x30, 0x20, 0x62, 0x79, 0x74, 0x65, 0x73, 0x20, 0x6C, 0x6F, 0x6E, 0x67, 0x2E, 0xA2, 0x00, 0x65, 0x73, 0x74, 0x72, 0x5F, 0x31, 0x03, 0x78, 0x1D, 0x54, 0x68, 0x69, 0x73, 0x20, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x69, 0x73, 0x20, 0x33, 0x30, 0x20, 0x62, 0x79, 0x74, 0x65, 0x73, 0x20, 0x6C, 0x6F, 0x6E, 0x67, 0x2E, 0xA2, 0x00, 0x65, 0x73, 0x74, 0x72, 0x5F, 0x32, 0x03, 0x78, 0x1D, 0x54, 0x68, 0x69, 0x73, 0x20, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x69, 0x73, 0x20, 0x33, 0x30, 0x20, 0x62, 0x79, 0x74, 0x65, 0x73, 0x20, 0x6C, 0x6F, 0x6E, 0x67, 0x2E, 0xA2, 0x00, 0x65, 0x73, 0x74, 0x72, 0x5F, 0x33, 0x03, 0x78, 0x1D, 0x54, 0x68, 0x69, 0x73, 0x20, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x69, 0x73, 0x20, 0x33, 0x30, 0x20, 0x62, 0x79, 0x74, 0x65, 0x73, 0x20, 0x6C, 0x6F, 0x6E, 0x67, 0x2E, 0xFF};
    std::vector<uint8_t> const actual_1 = cbor::encode(property_container);
    REQUIRE(actual_1 == expected_1);

    /* [{0: "str_4", 3: "This string is 30 bytes long."}, {0: "str_5", 3: "This string is 30 bytes long."}, {0: "str_6", 3: "This string is 30 bytes long."}, {0: "str_7", 3: "This string is 30 bytes long."}]
     * = 9F A2 00 65 73 74 72 5F 34 03 78 1D 54 68 69 73 20 73 74 72 69 6E 67 20 69 73 20 33 30 20 62 79 74 65 73 20 6C 6F 6E 67 2E A2 00 65 73 74 72 5F 35 03 78 1D 54 68 69 73 20 73 74 72 69 6E 67 20 69 73 20 33 30 20 62 79 74 65 73 20 6C 6F 6E 67 2E A2 00 65 73 74 72 5F 36 03 78 1D 54 68 69 73 20 73 74 72 69 6E 67 20 69 73 20 33 30 20 62 79 74 65 73 20 6C 6F 6E 67 2E A2 00 65 73 74 72 5F 37 03 78 1D 54 68 69 73 20 73 74 72 69 6E 67 20 69 73 20 33 30 20 62 79 74 65 73 20 6C 6F 6E 67 2E FF
     */
    std::vector<uint8_t> const expected_2 = {0x9F, 0xA2, 0x00, 0x65, 0x73, 0x74, 0x72, 0x5F, 0x34, 0x03, 0x78, 0x1D, 0x54, 0x68, 0x69, 0x73, 0x20, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x69, 0x73, 0x20, 0x33, 0x30, 0x20, 0x62, 0x79, 0x74, 0x65, 0x73, 0x20, 0x6C, 0x6F, 0x6E, 0x67, 0x2E, 0xA2, 0x00, 0x65, 0x73, 0x74, 0x72, 0x5F, 0x35, 0x03, 0x78, 0x1D, 0x54, 0x68, 0x69, 0x73, 0x20, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x69, 0x73, 0x20, 0x33, 0x30, 0x20, 0x62, 0x79, 0x74, 0x65, 0x73, 0x20, 0x6C, 0x6F, 0x6E, 0x67, 0x2E, 0xA2, 0x00, 0x65, 0x73, 0x74, 0x72, 0x5F, 0x36, 0x03, 0x78, 0x1D, 0x54, 0x68, 0x69, 0x73, 0x20, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x69, 0x73, 0x20, 0x33, 0x30, 0x20, 0x62, 0x79, 0x74, 0x65, 0x73, 0x20, 0x6C, 0x6F, 0x6E, 0x67, 0x2E, 0xA2, 0x00, 0x65, 0x73, 0x74, 0x72, 0x5F, 0x37, 0x03, 0x78, 0x1D, 0x54, 0x68, 0x69, 0x73, 0x20, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x69, 0x73, 0x20, 0x33, 0x30, 0x20, 0x62, 0x79, 0x74, 0x65, 0x73, 0x20, 0x6C, 0x6F, 0x6E, 0x67, 0x2E, 0xFF};
    std::vector<uint8_t> const actual_2 = cbor::encode(property_container);
    REQUIRE(actual_2 == expected_2);

    /* [{0: "str_8", 3: "This string is 30 bytes long."}, {0: "str_9", 3: "This string is 30 bytes long."}]
     * = 9F A2 00 65 73 74 72 5F 38 03 78 1D 54 68 69 73 20 73 74 72 69 6E 67 20 69 73 20 33 30 20 62 79 74 65 73 20 6C 6F 6E 67 2E A2 00 65 73 74 72 5F 39 03 78 1D 54 68 69 73 20 73 74 72 69 6E 67 20 69 73 20 33 30 20 62 79 74 65 73 20 6C 6F 6E 67 2E FF
     */
    std::vector<uint8_t> const expected_3 = {0x9F, 0xA2, 0x00, 0x65, 0x73, 0x74, 0x72, 0x5F, 0x38, 0x03, 0x78, 0x1D, 0x54, 0x68, 0x69, 0x73, 0x20, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x69, 0x73, 0x20, 0x33, 0x30, 0x20, 0x62, 0x79, 0x74, 0x65, 0x73, 0x20, 0x6C, 0x6F, 0x6E, 0x67, 0x2E, 0xA2, 0x00, 0x65, 0x73, 0x74, 0x72, 0x5F, 0x39, 0x03, 0x78, 0x1D, 0x54, 0x68, 0x69, 0x73, 0x20, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x69, 0x73, 0x20, 0x33, 0x30, 0x20, 0x62, 0x79, 0x74, 0x65, 0x73, 0x20, 0x6C, 0x6F, 0x6E, 0x67, 0x2E, 0xFF};
    std::vector<uint8_t> const actual_3 = cbor::encode(property_container);
    REQUIRE(actual_3 == expected_3);
  }

  /************************************************************************************/
}
