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

    /* [{0: "test", 2: 123}] = 9F A2 00 64 74 65 73 74 02 18 7B FF */
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

    /* [{0: "test:lat", 2: 2},{0: "test:lon", 2: 3}] = 9F A2 00 68 74 65 73 74 3A 6C 61 74 02 FA 40 00 00 00 A2 00 68 74 65 73 74 3A 6C 6F 6E 02 FA 40 40 00 00 FF*/
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

    /* [{0: "test:swi", 4: true},{0: "test:hue", 2: 2.0},{0: "test:sat", 2: 2.0},{0: "test:bri", 2: 2.0}] = 9F A2 00 68 74 65 73 74 3A 73 77 69 04 F5 A2 00 68 74 65 73 74 3A 68 75 65 02 FA 40 00 00 00 A2 00 68 74 65 73 74 3A 73 61 74 02 FA 40 00 00 00 A2 00 68 74 65 73 74 3A 62 72 69 02 FA 40 00 00 00 FF*/
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

    /* [{0: "test:swi", 4: true},{0: "test:vol", 2: 50},{0: "test:mut", 4: false},{0: "test:pbc", 2: 3},{0: "test:inp", 2: 55},{0: "test:cha", 2: 7}] = 9F A2 00 68 74 65 73 74 3A 73 77 69 04 F5 A2 00 68 74 65 73 74 3A 76 6F 6C 02 18 32 A2 00 68 74 65 73 74 3A 6D 75 74 04 F4 A2 00 68 74 65 73 74 3A 70 62 63 02 03 A2 00 68 74 65 73 74 3A 69 6E 70 02 18 37 A2 00 68 74 65 73 74 3A 63 68 61 02 07 FF */
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

    /* [{0: "test:swi", 4: true},{0: "test:bri", 2: 2.0}] = 9F A2 00 68 74 65 73 74 3A 73 77 69 04 F5 A2 00 68 74 65 73 74 3A 62 72 69 02 FA 40 00 00 00 FF*/
    std::vector<uint8_t> const expected = {0x9F, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x73, 0x77, 0x69, 0x04, 0xF5, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x62, 0x72, 0x69, 0x02, 0xFA, 0x40, 0x00, 0x00, 0x00, 0xFF };
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

  WHEN("A time property is added")
  {
    PropertyContainer property_container;
    cbor::encode(property_container);

    CloudTime test = 1633342784;
    addPropertyToContainer(property_container, test, "test", Permission::ReadWrite);

    /* [{0: "test", 2: 1633342784}] = 9F A2 00 64 74 65 73 74 02 1A 61 5A D5 40 FF*/
    std::vector<uint8_t> const expected = {0x9F, 0xA2, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x02, 0x1A, 0x61, 0x5A, 0xD5, 0x40, 0xFF};
    std::vector<uint8_t> const actual = cbor::encode(property_container);
    REQUIRE(actual == expected);
  }

  /************************************************************************************/

  WHEN("A 'Schedule' property is added")
  {
    PropertyContainer property_container;
    cbor::encode(property_container);

    CloudSchedule schedule_test = CloudSchedule(1633305600, 1633651200, 600, 1140850708);
    addPropertyToContainer(property_container, schedule_test, "test", Permission::ReadWrite);

    /* [{0: "test:frm", 2: 1633305600}, {0: "test:to", 2: 1633651200}, {0: "test:len", 2: 600}, {0: "test:msk", 2: 1140850708}]
       = 9F A2 00 68 74 65 73 74 3A 66 72 6D 02 1A 61 5A 44 00 A2 00 67 74 65 73 74 3A 74 6F 02 1A 61 5F 8A 00 A2 00 68 74 65 73 74 3A 6C 65 6E 02 19 02 58 A2 00 68 74 65 73 74 3A 6D 73 6B 02 1A 44 00 00 14 FF
    */
    std::vector<uint8_t> const expected = {0x9F, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x66, 0x72, 0x6D, 0x02, 0x1A, 0x61, 0x5A, 0x44, 0x00, 0xA2, 0x00, 0x67, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x74, 0x6F, 0x02, 0x1A, 0x61, 0x5F, 0x8A, 0x00, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x6C, 0x65, 0x6E, 0x02, 0x19, 0x02, 0x58, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x6D, 0x73, 0x6B, 0x02, 0x1A, 0x44, 0x00, 0x00, 0x14, 0xFF };
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

    /* [{0: "str_0", 3: "This string is 30 bytes long."}, {0: "str_1", 3: "This string is 30 bytes long."}, {0: "str_2", 3: "This string is 30 bytes long."}, {0: "str_3", 3: "This string is 30 bytes long."}, {0: "str_4", 3: "This string is 30 bytes long."}, {0: "str_5", 3: "This string is 30 bytes long."}]
     * = 9F A2 00 65 73 74 72 5F 30 03 78 1D 54 68 69 73 20 73 74 72 69 6E 67 20 69 73 20 33 30 20 62 79 74 65 73 20 6C 6F 6E 67 2E A2 00 65 73 74 72 5F 31 03 78 1D 54 68 69 73 20 73 74 72 69 6E 67 20 69 73 20 33 30 20 62 79 74 65 73 20 6C 6F 6E 67 2E A2 00 65 73 74 72 5F 32 03 78 1D 54 68 69 73 20 73 74 72 69 6E 67 20 69 73 20 33 30 20 62 79 74 65 73 20 6C 6F 6E 67 2E A2 00 65 73 74 72 5F 33 03 78 1D 54 68 69 73 20 73 74 72 69 6E 67 20 69 73 20 33 30 20 62 79 74 65 73 20 6C 6F 6E 67 2E A2 00 65 73 74 72 5F 34 03 78 1D 54 68 69 73 20 73 74 72 69 6E 67 20 69 73 20 33 30 20 62 79 74 65 73 20 6C 6F 6E 67 2E A2 00 65 73 74 72 5F 35 03 78 1D 54 68 69 73 20 73 74 72 69 6E 67 20 69 73 20 33 30 20 62 79 74 65 73 20 6C 6F 6E 67 2E FF
     */
    std::vector<uint8_t> const expected_1 = {0x9F, 0xA2, 0x00, 0x65, 0x73, 0x74, 0x72, 0x5F, 0x30, 0x03, 0x78, 0x1D, 0x54, 0x68, 0x69, 0x73, 0x20, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x69, 0x73, 0x20, 0x33, 0x30, 0x20, 0x62, 0x79, 0x74, 0x65, 0x73, 0x20, 0x6C, 0x6F, 0x6E, 0x67, 0x2E, 0xA2, 0x00, 0x65, 0x73, 0x74, 0x72, 0x5F, 0x31, 0x03, 0x78, 0x1D, 0x54, 0x68, 0x69, 0x73, 0x20, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x69, 0x73, 0x20, 0x33, 0x30, 0x20, 0x62, 0x79, 0x74, 0x65, 0x73, 0x20, 0x6C, 0x6F, 0x6E, 0x67, 0x2E, 0xA2, 0x00, 0x65, 0x73, 0x74, 0x72, 0x5F, 0x32, 0x03, 0x78, 0x1D, 0x54, 0x68, 0x69, 0x73, 0x20, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x69, 0x73, 0x20, 0x33, 0x30, 0x20, 0x62, 0x79, 0x74, 0x65, 0x73, 0x20, 0x6C, 0x6F, 0x6E, 0x67, 0x2E, 0xA2, 0x00, 0x65, 0x73, 0x74, 0x72, 0x5F, 0x33, 0x03, 0x78, 0x1D, 0x54, 0x68, 0x69, 0x73, 0x20, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x69, 0x73, 0x20, 0x33, 0x30, 0x20, 0x62, 0x79, 0x74, 0x65, 0x73, 0x20, 0x6C, 0x6F, 0x6E, 0x67, 0x2E, 0xA2, 0x00, 0x65, 0x73, 0x74, 0x72, 0x5F, 0x34, 0x03, 0x78, 0x1D, 0x54, 0x68, 0x69, 0x73, 0x20, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x69, 0x73, 0x20, 0x33, 0x30, 0x20, 0x62, 0x79, 0x74, 0x65, 0x73, 0x20, 0x6C, 0x6F, 0x6E, 0x67, 0x2E, 0xA2, 0x00, 0x65, 0x73, 0x74, 0x72, 0x5F, 0x35, 0x03, 0x78, 0x1D, 0x54, 0x68, 0x69, 0x73, 0x20, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x69, 0x73, 0x20, 0x33, 0x30, 0x20, 0x62, 0x79, 0x74, 0x65, 0x73, 0x20, 0x6C, 0x6F, 0x6E, 0x67, 0x2E, 0xFF};
    std::vector<uint8_t> const actual_1 = cbor::encode(property_container);
    REQUIRE(actual_1 == expected_1);

    /* [{0: "str_6", 3: "This string is 30 bytes long."}, {0: "str_7", 3: "This string is 30 bytes long."}, {0: "str_8", 3: "This string is 30 bytes long."}, {0: "str_9", 3: "This string is 30 bytes long."}]
     * = 9F A2 00 65 73 74 72 5F 36 03 78 1D 54 68 69 73 20 73 74 72 69 6E 67 20 69 73 20 33 30 20 62 79 74 65 73 20 6C 6F 6E 67 2E A2 00 65 73 74 72 5F 37 03 78 1D 54 68 69 73 20 73 74 72 69 6E 67 20 69 73 20 33 30 20 62 79 74 65 73 20 6C 6F 6E 67 2E A2 00 65 73 74 72 5F 38 03 78 1D 54 68 69 73 20 73 74 72 69 6E 67 20 69 73 20 33 30 20 62 79 74 65 73 20 6C 6F 6E 67 2E A2 00 65 73 74 72 5F 39 03 78 1D 54 68 69 73 20 73 74 72 69 6E 67 20 69 73 20 33 30 20 62 79 74 65 73 20 6C 6F 6E 67 2E FF
     */
    std::vector<uint8_t> const expected_2 = {0x9F, 0xA2, 0x00, 0x65, 0x73, 0x74, 0x72, 0x5F, 0x36, 0x03, 0x78, 0x1D, 0x54, 0x68, 0x69, 0x73, 0x20, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x69, 0x73, 0x20, 0x33, 0x30, 0x20, 0x62, 0x79, 0x74, 0x65, 0x73, 0x20, 0x6C, 0x6F, 0x6E, 0x67, 0x2E, 0xA2, 0x00, 0x65, 0x73, 0x74, 0x72, 0x5F, 0x37, 0x03, 0x78, 0x1D, 0x54, 0x68, 0x69, 0x73, 0x20, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x69, 0x73, 0x20, 0x33, 0x30, 0x20, 0x62, 0x79, 0x74, 0x65, 0x73, 0x20, 0x6C, 0x6F, 0x6E, 0x67, 0x2E, 0xA2, 0x00, 0x65, 0x73, 0x74, 0x72, 0x5F, 0x38, 0x03, 0x78, 0x1D, 0x54, 0x68, 0x69, 0x73, 0x20, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x69, 0x73, 0x20, 0x33, 0x30, 0x20, 0x62, 0x79, 0x74, 0x65, 0x73, 0x20, 0x6C, 0x6F, 0x6E, 0x67, 0x2E, 0xA2, 0x00, 0x65, 0x73, 0x74, 0x72, 0x5F, 0x39, 0x03, 0x78, 0x1D, 0x54, 0x68, 0x69, 0x73, 0x20, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x69, 0x73, 0x20, 0x33, 0x30, 0x20, 0x62, 0x79, 0x74, 0x65, 0x73, 0x20, 0x6C, 0x6F, 0x6E, 0x67, 0x2E, 0xFF};
    std::vector<uint8_t> const actual_2 = cbor::encode(property_container);
    REQUIRE(actual_2 == expected_2);
  }

  /************************************************************************************/

  WHEN("The size of the encoded properties is exceeding the CBOR buffer size and includes a multivalue property")
  {
    PropertyContainer property_container;

    CloudInt int_0; int_0 = 1000000;
    CloudInt int_1; int_1 = 1000001;
    CloudInt int_2; int_2 = 1000002;
    CloudInt int_3; int_3 = 1000003;
    CloudInt int_4; int_4 = 1000004;
    CloudInt int_5; int_5 = 1000005;
    CloudInt int_6; int_6 = 1000006;
    CloudInt int_7; int_7 = 1000007;
    CloudInt int_8; int_8 = 1000008;
    CloudInt int_9; int_9 = 1000009;
    CloudInt int_A; int_A = 1000010;
    CloudSchedule schedule = CloudSchedule(1633305600, 1633651200, 600, 1140850708);

    addPropertyToContainer(property_container, int_0, "int_value_0", Permission::ReadWrite);
    addPropertyToContainer(property_container, int_1, "int_value_1", Permission::ReadWrite);
    addPropertyToContainer(property_container, int_2, "int_value_2", Permission::ReadWrite);
    addPropertyToContainer(property_container, int_3, "int_value_3", Permission::ReadWrite);
    addPropertyToContainer(property_container, int_4, "int_value_4", Permission::ReadWrite);
    addPropertyToContainer(property_container, int_5, "int_value_5", Permission::ReadWrite);
    addPropertyToContainer(property_container, int_6, "int_value_6", Permission::ReadWrite);
    addPropertyToContainer(property_container, int_7, "int_value_7", Permission::ReadWrite);
    addPropertyToContainer(property_container, int_8, "int_value_8", Permission::ReadWrite);
    addPropertyToContainer(property_container, int_9, "int_value_9", Permission::ReadWrite);
    addPropertyToContainer(property_container, int_A, "int_value_A", Permission::ReadWrite);
    addPropertyToContainer(property_container, schedule, "schedule", Permission::ReadWrite);

    /* Due to the size if the encoded properties exceeding 256 bytes if encoded all at
     * once they are encoded in subsequent calls to CBOREncoder::encode.
     */

    /* [{0: "int_value_0", 2: 1000000}, {0: "int_value_1", 2: 1000001}, {0: "int_value_2", 2: 1000002}, {0: "int_value_3", 2: 1000003}, {0: "int_value_4", 2: 1000004}, {0: "int_value_5", 2: 1000005}, {0: "int_value_6", 2: 1000006}, {0: "int_value_7", 2: 1000007}, {0: "int_value_8", 2: 1000008}, {0: "int_value_9", 2: 1000009}, {0: "int_value_A", 2: 1000010}]
     * = 9F A2 00 6B 69 6E 74 5F 76 61 6C 75 65 5F 30 02 1A 00 0F 42 40 A2 00 6B 69 6E 74 5F 76 61 6C 75 65 5F 31 02 1A 00 0F 42 41 A2 00 6B 69 6E 74 5F 76 61 6C 75 65 5F 32 02 1A 00 0F 42 42 A2 00 6B 69 6E 74 5F 76 61 6C 75 65 5F 33 02 1A 00 0F 42 43 A2 00 6B 69 6E 74 5F 76 61 6C 75 65 5F 34 02 1A 00 0F 42 44 A2 00 6B 69 6E 74 5F 76 61 6C 75 65 5F 35 02 1A 00 0F 42 45 A2 00 6B 69 6E 74 5F 76 61 6C 75 65 5F 36 02 1A 00 0F 42 46 A2 00 6B 69 6E 74 5F 76 61 6C 75 65 5F 37 02 1A 00 0F 42 47 A2 00 6B 69 6E 74 5F 76 61 6C 75 65 5F 38 02 1A 00 0F 42 48 A2 00 6B 69 6E 74 5F 76 61 6C 75 65 5F 39 02 1A 00 0F 42 49 A2 00 6B 69 6E 74 5F 76 61 6C 75 65 5F 41 02 1A 00 0F 42 4A FF
     */
    std::vector<uint8_t> const expected_1 = {0x9F, 0xA2, 0x00, 0x6B, 0x69, 0x6E, 0x74, 0x5F, 0x76, 0x61, 0x6C, 0x75, 0x65, 0x5F, 0x30, 0x02, 0x1A, 0x00, 0x0F, 0x42, 0x40, 0xA2, 0x00, 0x6B, 0x69, 0x6E, 0x74, 0x5F, 0x76, 0x61, 0x6C, 0x75, 0x65, 0x5F, 0x31, 0x02, 0x1A, 0x00, 0x0F, 0x42, 0x41, 0xA2, 0x00, 0x6B, 0x69, 0x6E, 0x74, 0x5F, 0x76, 0x61, 0x6C, 0x75, 0x65, 0x5F, 0x32, 0x02, 0x1A, 0x00, 0x0F, 0x42, 0x42, 0xA2, 0x00, 0x6B, 0x69, 0x6E, 0x74, 0x5F, 0x76, 0x61, 0x6C, 0x75, 0x65, 0x5F, 0x33, 0x02, 0x1A, 0x00, 0x0F, 0x42, 0x43, 0xA2, 0x00, 0x6B, 0x69, 0x6E, 0x74, 0x5F, 0x76, 0x61, 0x6C, 0x75, 0x65, 0x5F, 0x34, 0x02, 0x1A, 0x00, 0x0F, 0x42, 0x44, 0xA2, 0x00, 0x6B, 0x69, 0x6E, 0x74, 0x5F, 0x76, 0x61, 0x6C, 0x75, 0x65, 0x5F, 0x35, 0x02, 0x1A, 0x00, 0x0F, 0x42, 0x45, 0xA2, 0x00, 0x6B, 0x69, 0x6E, 0x74, 0x5F, 0x76, 0x61, 0x6C, 0x75, 0x65, 0x5F, 0x36, 0x02, 0x1A, 0x00, 0x0F, 0x42, 0x46, 0xA2, 0x00, 0x6B, 0x69, 0x6E, 0x74, 0x5F, 0x76, 0x61, 0x6C, 0x75, 0x65, 0x5F, 0x37, 0x02, 0x1A, 0x00, 0x0F, 0x42, 0x47, 0xA2, 0x00, 0x6B, 0x69, 0x6E, 0x74, 0x5F, 0x76, 0x61, 0x6C, 0x75, 0x65, 0x5F, 0x38, 0x02, 0x1A, 0x00, 0x0F, 0x42, 0x48, 0xA2, 0x00, 0x6B, 0x69, 0x6E, 0x74, 0x5F, 0x76, 0x61, 0x6C, 0x75, 0x65, 0x5F, 0x39, 0x02, 0x1A, 0x00, 0x0F, 0x42, 0x49, 0xA2, 0x00, 0x6B, 0x69, 0x6E, 0x74, 0x5F, 0x76, 0x61, 0x6C, 0x75, 0x65, 0x5F, 0x41, 0x02, 0x1A, 0x00, 0x0F, 0x42, 0x4A, 0xFF};
    std::vector<uint8_t> const actual_1 = cbor::encode(property_container);
    REQUIRE(actual_1 == expected_1);

    /* [{0: "schedule:frm", 2: 1633305600}, {0: "schedule:to", 2: 1633651200}, {0: "schedule:len", 2: 600}, {0: "schedule:msk", 2: 1140850708}]
     * = 9F A2 00 6C 73 63 68 65 64 75 6C 65 3A 66 72 6D 02 1A 61 5A 44 00 A2 00 6B 73 63 68 65 64 75 6C 65 3A 74 6F 02 1A 61 5F 8A 00 A2 00 6C 73 63 68 65 64 75 6C 65 3A 6C 65 6E 02 19 02 58 A2 00 6C 73 63 68 65 64 75 6C 65 3A 6D 73 6B 02 1A 44 00 00 14 FF
     */
    std::vector<uint8_t> const expected_2 = {0x9F, 0xA2, 0x00, 0x6C, 0x73, 0x63, 0x68, 0x65, 0x64, 0x75, 0x6C, 0x65, 0x3A, 0x66, 0x72, 0x6D, 0x02, 0x1A, 0x61, 0x5A, 0x44, 0x00, 0xA2, 0x00, 0x6B, 0x73, 0x63, 0x68, 0x65, 0x64, 0x75, 0x6C, 0x65, 0x3A, 0x74, 0x6F, 0x02, 0x1A, 0x61, 0x5F, 0x8A, 0x00, 0xA2, 0x00, 0x6C, 0x73, 0x63, 0x68, 0x65, 0x64, 0x75, 0x6C, 0x65, 0x3A, 0x6C, 0x65, 0x6E, 0x02, 0x19, 0x02, 0x58, 0xA2, 0x00, 0x6C, 0x73, 0x63, 0x68, 0x65, 0x64, 0x75, 0x6C, 0x65, 0x3A, 0x6D, 0x73, 0x6B, 0x02, 0x1A, 0x44, 0x00, 0x00, 0x14, 0xFF};
    std::vector<uint8_t> const actual_2 = cbor::encode(property_container);
    REQUIRE(actual_2 == expected_2);
  }


  /************************************************************************************/

  WHEN("The size of a single encoded properties is exceeding the CBOR buffer size")
  {
    PropertyContainer property_container;

    CloudString str_0; str_0 = "I am trying to create a single property in such a way it does not fit the CBOR encoder message buffer size (that is 256 bytes) in order to test what happens trying to encode it. It looks easy, but in reality 256 bytes are a lot of characters to fill. Done!";

    addPropertyToContainer(property_container, str_0, "str_0", Permission::ReadWrite);

    /* Due to the size if the encoded properties exceeding 256 bytes if encoded all at
     * once they are encoded in subsequent calls to CBOREncoder::encode.
     */

    /* [{0: "str_0", 3: "I am trying to create a single property in such a way it does not fit the CBOR encoder message buffer size (that is 256 bytes) in order to test what happens trying to encode it. It looks easy, but in reality 256 bytes are a lot of characters to fill. Done!"}]
     * = 9F A2 00 65 73 74 72 5F 30 03 78 FF 49 20 61 6D 20 74 72 79 69 6E 67 20 74 6F 20 63 72 65 61 74 65 20 61 20 73 69 6E 67 6C 65 20 70 72 6F 70 65 72 74 79 20 69 6E 20 73 75 63 68 20 61 20 77 61 79 20 69 74 20 64 6F 65 73 20 6E 6F 74 20 66 69 74 20 74 68 65 20 43 42 4F 52 20 65 6E 63 6F 64 65 72 20 6D 65 73 73 61 67 65 20 62 75 66 66 65 72 20 73 69 7A 65 20 28 74 68 61 74 20 69 73 20 32 35 36 20 62 79 74 65 73 29 20 69 6E 20 6F 72 64 65 72 20 74 6F 20 74 65 73 74 20 77 68 61 74 20 68 61 70 70 65 6E 73 20 74 72 79 69 6E 67 20 74 6F 20 65 6E 63 6F 64 65 20 69 74 2E 20 49 74 20 6C 6F 6F 6B 73 20 65 61 73 79 2C 20 62 75 74 20 69 6E 20 72 65 61 6C 69 74 79 20 32 35 36 20 62 79 74 65 73 20 61 72 65 20 61 20 6C 6F 74 20 6F 66 20 63 68 61 72 61 63 74 65 72 73 20 74 6F 20 66 69 6C 6C 2E 20 44 6F 6E 65 21 FF
     */
    std::vector<uint8_t> const expected_1 = {};
    std::vector<uint8_t> const actual_1 = cbor::encode(property_container);
    REQUIRE(actual_1 == expected_1);
  }

}
