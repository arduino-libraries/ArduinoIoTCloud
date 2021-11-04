/*
   Copyright (c) 2019 Arduino.  All rights reserved.
*/

/**************************************************************************************
   INCLUDE
 **************************************************************************************/

#include <catch.hpp>

#include <memory>

#include <util/CBORTestUtil.h>

#include <CBORDecoder.h>
#include "types/CloudWrapperBool.h"
#include "types/CloudWrapperFloat.h"
#include "types/CloudWrapperInt.h"
#include "types/CloudWrapperString.h"
#include "types/automation/CloudColoredLight.h"
#include "types/automation/CloudContactSensor.h"
#include "types/automation/CloudDimmedLight.h"
#include "types/automation/CloudLight.h"
#include "types/automation/CloudMotionSensor.h"
#include "types/automation/CloudSmartPlug.h"
#include "types/automation/CloudSwitch.h"
#include "types/automation/CloudTemperatureSensor.h"
#include "types/automation/CloudTelevision.h"

/**************************************************************************************
   TEST CODE
 **************************************************************************************/

SCENARIO("Arduino Cloud Properties are decoded", "[ArduinoCloudThing::decode]")
{

  WHEN("A boolean property is changed via CBOR message")
  {
    PropertyContainer property_container;
        
    CloudBool test = true;
    addPropertyToContainer(property_container, test, "test", Permission::ReadWrite);

    /* [{0: "test", 4: false}] = 81 A2 00 64 74 65 73 74 04 F4 */
    uint8_t const payload[] = {0x81, 0xA2, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x04, 0xF4};
    int const payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORDecoder::decode(property_container, payload, payload_length);

    REQUIRE(test == false);
  }

  /************************************************************************************/

  WHEN("A boolean property is changed via CBOR message - light payload")
  {
    /*An integer identifier has been encoded instead of the name of the property in order to have a shorter payload*/
    PropertyContainer property_container;
        
    CloudBool test = true;
    /*The property is added with identifier 1 that will be used instead of the string "test" as property identifier*/
    addPropertyToContainer(property_container, test, "test", Permission::ReadWrite, 1);

    /* [{0: 1, 4: false}] = 81 A2 00 01 04 F4 */
    uint8_t const payload[] = {0x81, 0xA2, 0x00, 0x01, 0x04, 0xF4};
    int const payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORDecoder::decode(property_container, payload, payload_length);

    REQUIRE(test == false);
  }

  /************************************************************************************/

  WHEN("A positive int property is changed via CBOR message")
  {
    PropertyContainer property_container;
        
    CloudInt test = 0;
    addPropertyToContainer(property_container, test, "test", Permission::ReadWrite);

    /* [{0: "test", 2: 7}] = 81 A2 00 64 74 65 73 74 02 07 */
    uint8_t const payload[] = {0x81, 0xA2, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x02, 0x07};
    int const payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORDecoder::decode(property_container, payload, payload_length);

    REQUIRE(test == 7);
  }

  WHEN("A negative int property is changed via CBOR message")
  {
    PropertyContainer property_container;
        
    CloudInt test = 0;
    addPropertyToContainer(property_container, test, "test", Permission::ReadWrite);

    /* [{0: "test", 2: -7}] = 81 A2 00 64 74 65 73 74 02 26 */
    uint8_t const payload[] = {0x81, 0xA2, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x02, 0x26};
    int const payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORDecoder::decode(property_container, payload, payload_length);

    REQUIRE(test == -7);
  }

  /************************************************************************************/

  WHEN("A float property is changed via CBOR message")
  {
    PropertyContainer property_container;
        
    CloudFloat test = 0.0f;
    addPropertyToContainer(property_container, test, "test", Permission::ReadWrite);

    /* [{0: "test", 2: 3.1459}] = 81 A2 00 64 74 65 73 74 02 FB 40 09 2A CD 9E 83 E4 26 */
    uint8_t const payload[] = {0x81, 0xA2, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x02, 0xFB, 0x40, 0x09, 0x2A, 0xCD, 0x9E, 0x83, 0xE4, 0x26};
    int const payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORDecoder::decode(property_container, payload, payload_length);

    REQUIRE(test == Approx(3.1459).epsilon(0.01));
  }

  /************************************************************************************/

  WHEN("A String property is changed via CBOR message")
  {
    PropertyContainer property_container;
        
    CloudString str_test;
    str_test = "test";
    addPropertyToContainer(property_container, str_test, "test", Permission::ReadWrite);

    /* [{0: "test", 3: "testtt"}] = 81 A2 00 64 74 65 73 74 03 66 74 65 73 74 74 74 */
    uint8_t const payload[] = {0x81, 0xA2, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x03, 0x66, 0x74, 0x65, 0x73, 0x74, 0x74, 0x74};
    CBORDecoder::decode(property_container, payload, sizeof(payload) / sizeof(uint8_t));

    REQUIRE(str_test == "testtt");
  }

  /************************************************************************************/

  WHEN("A Location property is changed via CBOR message")
  {
    PropertyContainer property_container;
        
    CloudLocation location_test = CloudLocation(0, 1);
    addPropertyToContainer(property_container, location_test, "test", Permission::ReadWrite);

    /* [{0: "test:lat", 2: 2},{0: "test:lon", 2: 3}] = 82 A2 00 68 74 65 73 74 3A 6C 61 74 02 02 A2 00 68 74 65 73 74 3A 6C 6F 6E 02 03*/
    uint8_t const payload[] = { 0x82, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x6C, 0x61, 0x74, 0x02, 0x02, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x6C, 0x6F, 0x6E, 0x02, 0x03 };
    CBORDecoder::decode(property_container, payload, sizeof(payload) / sizeof(uint8_t));
    Location location_compare = Location(2, 3);
    Location value_location_test = location_test.getValue();
    REQUIRE(value_location_test.lat == location_compare.lat);
    REQUIRE(value_location_test.lon == location_compare.lon);
  }

  /************************************************************************************/

  WHEN("A Color property is changed via CBOR message")
  {
    PropertyContainer property_container;
        
    CloudColor color_test = CloudColor(0.0, 0.0, 0.0);

    addPropertyToContainer(property_container, color_test, "test", Permission::ReadWrite);

    /* [{0: "test:hue", 2: 2.0},{0: "test:sat", 2: 2.0},{0: "test:bri", 2: 2.0}] = 83 A2 00 68 74 65 73 74 3A 68 75 65 02 FA 40 00 00 00 A2 00 68 74 65 73 74 3A 73 61 74 02 FA 40 00 00 00 A2 00 68 74 65 73 74 3A 62 72 69 02 FA 40 00 00 00 */
    uint8_t const payload[] = {0x83, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x68, 0x75, 0x65, 0x02, 0xFA, 0x40, 0x00, 0x00, 0x00, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x73, 0x61, 0x74, 0x02, 0xFA, 0x40, 0x00, 0x00, 0x00, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x62, 0x72, 0x69, 0x02, 0xFA, 0x40, 0x00, 0x00, 0x00 };
    CBORDecoder::decode(property_container, payload, sizeof(payload) / sizeof(uint8_t));

    Color color_compare = Color(2.0, 2.0, 2.0);
    Color value_color_test = color_test.getValue();
    bool verify = (value_color_test == color_compare);
    REQUIRE(verify);
    REQUIRE(value_color_test.hue == color_compare.hue);
    REQUIRE(value_color_test.sat == color_compare.sat);
    REQUIRE(value_color_test.bri == color_compare.bri);
  }

  /************************************************************************************/

  WHEN("A Color property is changed via CBOR message - light payload")
  {
    /*An integer identifier has been encoded instead of the name of the property in order to have a shorter payload*/
    PropertyContainer property_container;
        
    CloudColor color_test = CloudColor(0.0, 0.0, 0.0);

    /*The property is added with identifier 1 that will be used instead of the string "test" as property identifier*/
    addPropertyToContainer(property_container, color_test, "test", Permission::ReadWrite, 1);

    /* [{0: 257, 2: 2.0},{0: 513, 2: 2.0},{0: 769, 2: 2.0}] = 83 A2 00 19 01 01 02 FA 40 00 00 00 A2 00 19 02 01 02 FA 40 00 00 00 A2 00 19 03 01 02 FA 40 00 00 00 */
    uint8_t const payload[] = {0x83, 0xA2, 0x00, 0x19, 0x01, 0x01, 0x02, 0xFA, 0x40, 0x00, 0x00, 0x00, 0xA2, 0x00, 0x19, 0x02, 0x01, 0x02, 0xFA, 0x40, 0x00, 0x00, 0x00, 0xA2, 0x00, 0x19, 0x03, 0x01, 0x02, 0xFA, 0x40, 0x00, 0x00, 0x00 };
    CBORDecoder::decode(property_container, payload, sizeof(payload) / sizeof(uint8_t));

    Color color_compare = Color(2.0, 2.0, 2.0);
    Color value_color_test = color_test.getValue();
    bool verify = (value_color_test == color_compare);
    REQUIRE(verify);
    REQUIRE(value_color_test.hue == color_compare.hue);
    REQUIRE(value_color_test.sat == color_compare.sat);
    REQUIRE(value_color_test.bri == color_compare.bri);
  }

  /************************************************************************************/

  WHEN("A ColoredLight property is changed via CBOR message")
  {
    PropertyContainer property_container;
      
    CloudColoredLight color_test = CloudColoredLight(false, 0.0, 0.0, 0.0);

    addPropertyToContainer(property_container, color_test, "test", Permission::ReadWrite);

    /* [{0: "test:swi", 4: true},{0: "test:hue", 2: 2.0},{0: "test:sat", 2: 2.0},{0: "test:bri", 2: 2.0}] = 83 A2 00 68 74 65 73 74 3A 73 77 69 04 F5 //A2 00 68 74 65 73 74 3A 68 75 65 02 FA 40 00 00 00 A2 00 68 74 65 73 74 3A 73 61 74 02 FA 40 00 00 00 A2 00 68 74 65 73 74 3A 62 72 69 02 FA 40 00 00 00 */
    uint8_t const payload[] = {0x84, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x73, 0x77, 0x69, 0x04, 0xF5, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x68, 0x75, 0x65, 0x02, 0xFA, 0x40, 0x00, 0x00, 0x00, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x73, 0x61, 0x74, 0x02, 0xFA, 0x40, 0x00, 0x00, 0x00, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x62, 0x72, 0x69, 0x02, 0xFA, 0x40, 0x00, 0x00, 0x00 };
    CBORDecoder::decode(property_container, payload, sizeof(payload) / sizeof(uint8_t));

    ColoredLight color_compare = ColoredLight(true, 2.0, 2.0, 2.0);
    ColoredLight value_color_test = color_test.getValue();
    bool verify = (value_color_test == color_compare);
    REQUIRE(verify);
    REQUIRE(value_color_test.swi == color_compare.swi);
    REQUIRE(value_color_test.hue == color_compare.hue);
    REQUIRE(value_color_test.sat == color_compare.sat);
    REQUIRE(value_color_test.bri == color_compare.bri);
  }

  /************************************************************************************/

  WHEN("A Television property is changed via CBOR message")
  {
    PropertyContainer property_container;
        
    CloudTelevision tv_test = CloudTelevision(false, 0, false, PlaybackCommands::Stop, InputValue::AUX1, 0);

    addPropertyToContainer(property_container, tv_test, "test", Permission::ReadWrite);

    /* [{0: "test:swi", 4: true},{0: "test:vol", 2: 50},{0: "test:mut", 2: false},{0: "test:pbc", 2: 3},{0: "test:inp", 2: 55},{0: "test:cha", 2: 7}] = 9F A2 00 68 74 65 73 74 3A 73 77 69 04 F5 A2 00 68 74 65 73 74 3A 76 6F 6C 02 18 32 A2 00 68 74 65 73 74 3A 6D 75 74 04 F4 A2 00 68 74 65 73 74 3A 70 62 63 02 03 A2 00 68 74 65 73 74 3A 69 6E 70 02 18 37 A2 00 68 74 65 73 74 3A 63 68 61 02 07 FF */
    uint8_t const payload[] = {0x9F, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x73, 0x77, 0x69, 0x04, 0xF5, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x76, 0x6F, 0x6C, 0x02, 0x18, 0x32, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x6D, 0x75, 0x74, 0x04, 0xF4, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x70, 0x62, 0x63, 0x02, 0x03, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x69, 0x6E, 0x70, 0x02, 0x18, 0x37, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x63, 0x68, 0x61, 0x02, 0x07, 0xFF};
    CBORDecoder::decode(property_container, payload, sizeof(payload) / sizeof(uint8_t));

    Television tv_compare = Television(true, 50, false, PlaybackCommands::Play, InputValue::TV, 7);
    Television value_tv_test = tv_test.getValue();
    bool verify = (value_tv_test == tv_compare);
    REQUIRE(verify);
    REQUIRE(value_tv_test.swi == tv_compare.swi);
    REQUIRE(value_tv_test.vol == tv_compare.vol);
    REQUIRE(value_tv_test.mut == tv_compare.mut);
    REQUIRE(value_tv_test.pbc == tv_compare.pbc);
    REQUIRE(value_tv_test.inp == tv_compare.inp);
    REQUIRE(value_tv_test.cha == tv_compare.cha);
  }

  /************************************************************************************/

  WHEN("A DimmedLight property is changed via CBOR message")
  {
    PropertyContainer property_container;
        
    CloudDimmedLight light_test = CloudDimmedLight(false, 0.0);

    addPropertyToContainer(property_container, light_test, "test", Permission::ReadWrite);

    /* [{0: "test:swi", 4: true},{0: "test:bri", 2: 2.0}] = 83 A2 00 68 74 65 73 74 3A 73 77 69 04 F5 //A2 00 68 74 65 73 74 3A 62 72 69 02 FA 40 00 00 00  */
    uint8_t const payload[] = {0x82, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x73, 0x77, 0x69, 0x04, 0xF5, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x62, 0x72, 0x69, 0x02, 0xFA, 0x40, 0x00, 0x00, 0x00};
    CBORDecoder::decode(property_container, payload, sizeof(payload) / sizeof(uint8_t));

    DimmedLight light_compare = DimmedLight(true, 2.0);
    DimmedLight value_light_test = light_test.getValue();
    bool verify = (value_light_test == light_compare);
    REQUIRE(verify);
    REQUIRE(value_light_test.swi == light_compare.swi);
    REQUIRE(value_light_test.bri == light_compare.bri);
  }

  /************************************************************************************/

  WHEN("A Light property is changed via CBOR message")
  {
    PropertyContainer property_container;
        
    CloudLight light_test;
    light_test = false;

    addPropertyToContainer(property_container, light_test, "test", Permission::ReadWrite);

    /* [{0: "test", 4: true}] = 81 A2 00 64 74 65 73 74 04 F5  */
    uint8_t const payload[] = {0x81, 0xA2, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x04, 0xF5};
    CBORDecoder::decode(property_container, payload, sizeof(payload) / sizeof(uint8_t));

    REQUIRE(light_test == true);
  }

  /************************************************************************************/

  WHEN("A ContactSensor property is changed via CBOR message")
  {
    PropertyContainer property_container;
        
    CloudContactSensor contact_test;
    contact_test = false;

    addPropertyToContainer(property_container, contact_test, "test", Permission::ReadWrite);

    /* [{0: "test", 4: true}] = 81 A2 00 64 74 65 73 74 04 F5  */
    uint8_t const payload[] = {0x81, 0xA2, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x04, 0xF5};
    CBORDecoder::decode(property_container, payload, sizeof(payload) / sizeof(uint8_t));

    REQUIRE(contact_test == true);
  }

  /************************************************************************************/

  WHEN("A MotionSensor property is changed via CBOR message")
  {
    PropertyContainer property_container;
        
    CloudMotionSensor motion_test;
    motion_test = false;

    addPropertyToContainer(property_container, motion_test, "test", Permission::ReadWrite);

    /* [{0: "test", 4: true}] = 81 A2 00 64 74 65 73 74 04 F5  */
    uint8_t const payload[] = {0x81, 0xA2, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x04, 0xF5};
    CBORDecoder::decode(property_container, payload, sizeof(payload) / sizeof(uint8_t));

    REQUIRE(motion_test == true);
  }

  /************************************************************************************/

  WHEN("A SmartPlug property is changed via CBOR message")
  {
    PropertyContainer property_container;
        
    CloudSmartPlug plug_test;
    plug_test = false;

    addPropertyToContainer(property_container, plug_test, "test", Permission::ReadWrite);

    /* [{0: "test", 4: true}] = 81 A2 00 64 74 65 73 74 04 F5  */
    uint8_t const payload[] = {0x81, 0xA2, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x04, 0xF5};
    CBORDecoder::decode(property_container, payload, sizeof(payload) / sizeof(uint8_t));

    REQUIRE(plug_test == true);
  }

  /************************************************************************************/

  WHEN("A Switch property is changed via CBOR message")
  {
    PropertyContainer property_container;
        
    CloudSwitch switch_test;
    switch_test = false;

    addPropertyToContainer(property_container, switch_test, "test", Permission::ReadWrite);

    /* [{0: "test", 4: true}] = 81 A2 00 64 74 65 73 74 04 F5  */
    uint8_t const payload[] = {0x81, 0xA2, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x04, 0xF5};
    CBORDecoder::decode(property_container, payload, sizeof(payload) / sizeof(uint8_t));

    REQUIRE(switch_test == true);
  }

  /************************************************************************************/

  WHEN("A Temperature property is changed via CBOR message")
  {
    PropertyContainer property_container;
        
    CloudTemperatureSensor test;
    test = 0.0f;
    addPropertyToContainer(property_container, test, "test", Permission::ReadWrite);

    /* [{0: "test", 2: 3.1459}] = 81 A2 00 64 74 65 73 74 02 FB 40 09 2A CD 9E 83 E4 26 */
    uint8_t const payload[] = {0x81, 0xA2, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x02, 0xFB, 0x40, 0x09, 0x2A, 0xCD, 0x9E, 0x83, 0xE4, 0x26};
    int const payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORDecoder::decode(property_container, payload, payload_length);

    REQUIRE(test == Approx(3.1459).epsilon(0.01));
  }

  /************************************************************************************/

  WHEN("A Time property is changed via CBOR message")
  {
    PropertyContainer property_container;

    CloudTime test;
    test = 0;
    addPropertyToContainer(property_container, test, "test", Permission::ReadWrite);

    /* [{0: "test", 2: 4294967295}] = 81 A2 00 64 74 65 73 74 02 1A FF FF FF FF */
    uint8_t const payload[] = {0x81, 0xA2, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x02, 0x1A, 0xFF, 0xFF, 0xFF, 0xFF};
    int const payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORDecoder::decode(property_container, payload, payload_length);

    REQUIRE(test == 4294967295);
  }

  /************************************************************************************/

  WHEN("A Schedule property is changed via CBOR message")
  {
    PropertyContainer property_container;

    CloudSchedule schedule_test = CloudSchedule(0, 0, 0, 0);
    addPropertyToContainer(property_container, schedule_test, "test", Permission::ReadWrite);

    /* [{0: "test:frm", 2: 1633305600}, {0: "test:to", 2: 1633651200}, {0: "test:len", 2: 600}, {0: "test:msk", 2: 1140850708}]
       = 84 A2 00 68 74 65 73 74 3A 66 72 6D 02 1A 61 5A 44 00 A2 00 67 74 65 73 74 3A 74 6F 02 1A 61 5F 8A 00 A2 00 68 74 65 73 74 3A 6C 65 6E 02 19 02 58 A2 00 68 74 65 73 74 3A 6D 73 6B 02 1A 44 00 00 14
    */
    uint8_t const payload[] = {0x84, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x66, 0x72, 0x6D, 0x02, 0x1A, 0x61, 0x5A, 0x44, 0x00, 0xA2, 0x00, 0x67, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x74, 0x6F, 0x02, 0x1A, 0x61, 0x5F, 0x8A, 0x00, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x6C, 0x65, 0x6E, 0x02, 0x19, 0x02, 0x58, 0xA2, 0x00, 0x68, 0x74, 0x65, 0x73, 0x74, 0x3A, 0x6D, 0x73, 0x6B, 0x02, 0x1A, 0x44, 0x00, 0x00, 0x14};
    CBORDecoder::decode(property_container, payload, sizeof(payload) / sizeof(uint8_t));

    Schedule schedule_compare = Schedule(1633305600, 1633651200, 600, 1140850708);
    Schedule value_schedule_test = schedule_test.getValue(); 
    
    bool verify = (value_schedule_test == schedule_compare);
    REQUIRE(verify);
    REQUIRE(value_schedule_test.frm == schedule_compare.frm);
    REQUIRE(value_schedule_test.to  == schedule_compare.to);
    REQUIRE(value_schedule_test.len == schedule_compare.len);
    REQUIRE(value_schedule_test.msk == schedule_compare.msk);
  }

  /************************************************************************************/

  WHEN("Multiple properties is changed via CBOR message")
  {
    WHEN("Multiple properties of different type are changed via CBOR message")
    {
      PropertyContainer property_container;
          
      CloudBool   bool_test = false;
      CloudInt    int_test = 1;
      CloudFloat  float_test = 2.0f;
      CloudString str_test;
      str_test = ("str_test");

      addPropertyToContainer(property_container, bool_test,  "bool_test",  Permission::ReadWrite);
      addPropertyToContainer(property_container, int_test,   "int_test",   Permission::ReadWrite);
      addPropertyToContainer(property_container, float_test, "float_test", Permission::ReadWrite);
      addPropertyToContainer(property_container, str_test,   "str_test",   Permission::ReadWrite);

      /* [{0: "bool_test", 4: true}, {0: "int_test", 2: 10}, {0: "float_test", 2: 20.0}, {0: "str_test", 3: "hello arduino"}]
         = 84 A2 00 69 62 6F 6F 6C 5F 74 65 73 74 04 F5 A2 00 68 69 6E 74 5F 74 65 73 74 02 0A A2 00 6A 66 6C 6F 61 74 5F 74 65 73 74 02 F9 4D 00 A2 00 68 73 74 72 5F 74 65 73 74 03 6D 68 65 6C 6C 6F 20 61 72 64 75 69 6E 6F
      */
      uint8_t const payload[] = {0x84, 0xA2, 0x00, 0x69, 0x62, 0x6F, 0x6F, 0x6C, 0x5F, 0x74, 0x65, 0x73, 0x74, 0x04, 0xF5, 0xA2, 0x00, 0x68, 0x69, 0x6E, 0x74, 0x5F, 0x74, 0x65, 0x73, 0x74, 0x02, 0x0A, 0xA2, 0x00, 0x6A, 0x66, 0x6C, 0x6F, 0x61, 0x74, 0x5F, 0x74, 0x65, 0x73, 0x74, 0x02, 0xF9, 0x4D, 0x00, 0xA2, 0x00, 0x68, 0x73, 0x74, 0x72, 0x5F, 0x74, 0x65, 0x73, 0x74, 0x03, 0x6D, 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x61, 0x72, 0x64, 0x75, 0x69, 0x6E, 0x6F};
      CBORDecoder::decode(property_container, payload, sizeof(payload) / sizeof(uint8_t));

      REQUIRE(bool_test  == true);
      REQUIRE(int_test   == 10);
      REQUIRE(float_test == Approx(20.0).epsilon(0.01));
      REQUIRE(str_test   == "hello arduino");
    }

    /********************************************************************************/

    WHEN("Multiple properties of different type are synchronized via CBOR message. FORCE_CLOUD_SYNC is passed as synchronization function and as a consequence values contained in the incoming message are stored in the properties")
    {
      PropertyContainer property_container;
          
      CloudBool   bool_test = false;
      CloudInt    int_test = 1;
      CloudFloat  float_test = 2.0f;
      CloudString str_test;
      str_test = ("str_test");

      addPropertyToContainer(property_container, bool_test,  "bool_test",  Permission::ReadWrite).onSync(CLOUD_WINS);
      addPropertyToContainer(property_container, int_test,   "int_test",   Permission::ReadWrite).onSync(CLOUD_WINS);
      addPropertyToContainer(property_container, float_test, "float_test", Permission::ReadWrite).onSync(CLOUD_WINS);
      addPropertyToContainer(property_container, str_test,   "str_test",   Permission::ReadWrite).onSync(CLOUD_WINS);

      /* [{0: "bool_test", 4: true}, {0: "int_test", 2: 10}, {0: "float_test", 2: 20.0}, {0: "str_test", 3: "hello arduino"}]
         = 84 A2 00 69 62 6F 6F 6C 5F 74 65 73 74 04 F5 A2 00 68 69 6E 74 5F 74 65 73 74 02 0A A2 00 6A 66 6C 6F 61 74 5F 74 65 73 74 02 F9 4D 00 A2 00 68 73 74 72 5F 74 65 73 74 03 6D 68 65 6C 6C 6F 20 61 72 64 75 69 6E 6F
      */
      uint8_t const payload[] = {0x84, 0xA2, 0x00, 0x69, 0x62, 0x6F, 0x6F, 0x6C, 0x5F, 0x74, 0x65, 0x73, 0x74, 0x04, 0xF5, 0xA2, 0x00, 0x68, 0x69, 0x6E, 0x74, 0x5F, 0x74, 0x65, 0x73, 0x74, 0x02, 0x0A, 0xA2, 0x00, 0x6A, 0x66, 0x6C, 0x6F, 0x61, 0x74, 0x5F, 0x74, 0x65, 0x73, 0x74, 0x02, 0xF9, 0x4D, 0x00, 0xA2, 0x00, 0x68, 0x73, 0x74, 0x72, 0x5F, 0x74, 0x65, 0x73, 0x74, 0x03, 0x6D, 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x61, 0x72, 0x64, 0x75, 0x69, 0x6E, 0x6F};
      CBORDecoder::decode(property_container, payload, sizeof(payload) / sizeof(uint8_t), true);

      REQUIRE(bool_test  == true);
      REQUIRE(int_test   == 10);
      REQUIRE(float_test == Approx(20.0).epsilon(0.01));
      REQUIRE(str_test   == "hello arduino");
    }

    /********************************************************************************/

    WHEN("Multiple primitive properties of different type are synchronized via CBOR message. FORCE_CLOUD_SYNC is passed as synchronization function and as a consequence values contained in the incoming message are stored in the properties")
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

      addPropertyToContainer(property_container, *b, "bool_test",  Permission::ReadWrite).onSync(CLOUD_WINS);
      addPropertyToContainer(property_container, *i, "int_test",   Permission::ReadWrite).onSync(CLOUD_WINS);
      addPropertyToContainer(property_container, *f, "float_test", Permission::ReadWrite).onSync(CLOUD_WINS);
      addPropertyToContainer(property_container, *s, "str_test",   Permission::ReadWrite).onSync(CLOUD_WINS);

      updateTimestampOnLocallyChangedProperties(property_container);

      /* [{0: "bool_test", 4: true}, {0: "int_test", 2: 10}, {0: "float_test", 2: 20.0}, {0: "str_test", 3: "hello arduino"}]
         = 84 A2 00 69 62 6F 6F 6C 5F 74 65 73 74 04 F5 A2 00 68 69 6E 74 5F 74 65 73 74 02 0A A2 00 6A 66 6C 6F 61 74 5F 74 65 73 74 02 F9 4D 00 A2 00 68 73 74 72 5F 74 65 73 74 03 6D 68 65 6C 6C 6F 20 61 72 64 75 69 6E 6F
      */
      uint8_t const payload[] = {0x84, 0xA2, 0x00, 0x69, 0x62, 0x6F, 0x6F, 0x6C, 0x5F, 0x74, 0x65, 0x73, 0x74, 0x04, 0xF5, 0xA2, 0x00, 0x68, 0x69, 0x6E, 0x74, 0x5F, 0x74, 0x65, 0x73, 0x74, 0x02, 0x0A, 0xA2, 0x00, 0x6A, 0x66, 0x6C, 0x6F, 0x61, 0x74, 0x5F, 0x74, 0x65, 0x73, 0x74, 0x02, 0xF9, 0x4D, 0x00, 0xA2, 0x00, 0x68, 0x73, 0x74, 0x72, 0x5F, 0x74, 0x65, 0x73, 0x74, 0x03, 0x6D, 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x61, 0x72, 0x64, 0x75, 0x69, 0x6E, 0x6F};
      CBORDecoder::decode(property_container, payload, sizeof(payload) / sizeof(uint8_t), true);

      REQUIRE(bool_test  == true);
      REQUIRE(int_test   == 10);
      REQUIRE(float_test == Approx(20.0).epsilon(0.01));
      REQUIRE(str_test   == "hello arduino");
    }

    /********************************************************************************/

    WHEN("Multiple String properties are changed via CBOR message")
    {
      PropertyContainer property_container;
          
      CloudString str_1("hello"),
                  str_2("arduino"),
                  str_3("cloud"),
                  str_4("test");

      addPropertyToContainer(property_container, str_1, "str_1", Permission::ReadWrite);
      addPropertyToContainer(property_container, str_2, "str_2", Permission::ReadWrite);
      addPropertyToContainer(property_container, str_3, "str_3", Permission::ReadWrite);
      addPropertyToContainer(property_container, str_4, "str_4", Permission::ReadWrite);

      /* [{0: "str_1", 3: "I'd like"}, {0: "str_2", 3: "a"}, {0: "str_3", 3: "cup"}, {0: "str_4", 3: "of coffee"}]
         = 84 A2 00 65 73 74 72 5F 31 03 68 49 27 64 20 6C 69 6B 65 A2 00 65 73 74 72 5F 32 03 61 61 A2 00 65 73 74 72 5F 33 03 63 63 75 70 A2 00 65 73 74 72 5F 34 03 69 6F 66 20 63 6F 66 66 65 65
      */
      uint8_t const payload[] = {0x84, 0xA2, 0x00, 0x65, 0x73, 0x74, 0x72, 0x5F, 0x31, 0x03, 0x68, 0x49, 0x27, 0x64, 0x20, 0x6C, 0x69, 0x6B, 0x65, 0xA2, 0x00, 0x65, 0x73, 0x74, 0x72, 0x5F, 0x32, 0x03, 0x61, 0x61, 0xA2, 0x00, 0x65, 0x73, 0x74, 0x72, 0x5F, 0x33, 0x03, 0x63, 0x63, 0x75, 0x70, 0xA2, 0x00, 0x65, 0x73, 0x74, 0x72, 0x5F, 0x34, 0x03, 0x69, 0x6F, 0x66, 0x20, 0x63, 0x6F, 0x66, 0x66, 0x65, 0x65};
      CBORDecoder::decode(property_container, payload, sizeof(payload) / sizeof(uint8_t));

      REQUIRE(str_1 == "I'd like");
      REQUIRE(str_2 == "a");
      REQUIRE(str_3 == "cup");
      REQUIRE(str_4 == "of coffee");
    }
  }

  /************************************************************************************/

  WHEN("A payload containing a CBOR base name is parsed")
  {
    PropertyContainer property_container;
        
    CloudString str = "hello";
    addPropertyToContainer(property_container, str, "test", Permission::ReadWrite);

    /* [{-2: "some-test-base-name", 0: "test", 3: "test"}] = 81 A3 21 73 73 6F 6D 65 2D 74 65 73 74 2D 62 61 73 65 2D 6E 61 6D 65 00 64 74 65 73 74 03 64 74 65 73 74 */
    uint8_t const payload[] = {0x81, 0xA3, 0x21, 0x73, 0x73, 0x6F, 0x6D, 0x65, 0x2D, 0x74, 0x65, 0x73, 0x74, 0x2D, 0x62, 0x61, 0x73, 0x65, 0x2D, 0x6E, 0x61, 0x6D, 0x65, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x03, 0x64, 0x74, 0x65, 0x73, 0x74};
    CBORDecoder::decode(property_container, payload, sizeof(payload) / sizeof(uint8_t));

    REQUIRE(str == "test");
  }

  /************************************************************************************/

  WHEN("A payload containing a CBOR base time is parsed")
  {
    PropertyContainer property_container;
       
    CloudInt test = 0;
    addPropertyToContainer(property_container, test, "test", Permission::ReadWrite);

    /* [{-3: 123.456, 0: "test", 2: 1}] = 81 A3 22 FB 40 5E DD 2F 1A 9F BE 77 00 64 74 65 73 74 02 01 */
    uint8_t const payload[] = {0x81, 0xA3, 0x22, 0xFB, 0x40, 0x5E, 0xDD, 0x2F, 0x1A, 0x9F, 0xBE, 0x77, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x02, 0x01};
    CBORDecoder::decode(property_container, payload, sizeof(payload) / sizeof(uint8_t));

    REQUIRE(test == 1);
  }

  /************************************************************************************/

  WHEN("A payload containing a CBOR time is parsed")
  {
    PropertyContainer property_container;
        
    CloudInt test = 0;
    addPropertyToContainer(property_container, test, "test", Permission::ReadWrite);

    /* [{6: 123.456, 0: "test", 2: 1}] = 81 A3 06 FB 40 5E DD 2F 1A 9F BE 77 00 64 74 65 73 74 02 01 */
    uint8_t const payload[] = {0x81, 0xA3, 0x06, 0xFB, 0x40, 0x5E, 0xDD, 0x2F, 0x1A, 0x9F, 0xBE, 0x77, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x02, 0x01};
    CBORDecoder::decode(property_container, payload, sizeof(payload) / sizeof(uint8_t));

    REQUIRE(test == 1);
  }

  /************************************************************************************/

  WHEN("A payload containing a CBOR BaseVersion is parsed")
  {
    PropertyContainer property_container;
        
    CloudInt test = 0;
    addPropertyToContainer(property_container, test, "test", Permission::ReadWrite);

    /* [{-1: 1, 0: "test", 2: 1}] = 81 A3 20 01 00 64 74 65 73 74 02 01 */
    uint8_t const payload[] = {0x81, 0xA3, 0x20, 0x01, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x02, 0x01};
    CBORDecoder::decode(property_container, payload, sizeof(payload) / sizeof(uint8_t));

    REQUIRE(test == 1);
  }

  /************************************************************************************/

  WHEN("A payload containing a CBOR BaseName, BaseTime and Time is parsed")
  {
    PropertyContainer property_container;
        
    CloudInt test = 0;
    addPropertyToContainer(property_container, test, "test", Permission::ReadWrite);

    /* [{-2: "base-name", -3: 654.321, 6: 123.456, 0: "test", 2: 1}] =
       81 A5 21 69 62 61 73 65 2D 6E 61 6D 65 22 FB 40 84 72 91 68 72 B0 21 06 FB 40 5E DD 2F 1A 9F BE 77 00 64 74 65 73 74 02 01
    */
    uint8_t const payload[] = {0x81, 0xA5, 0x21, 0x69, 0x62, 0x61, 0x73, 0x65, 0x2D, 0x6E, 0x61, 0x6D, 0x65, 0x22, 0xFB, 0x40, 0x84, 0x72, 0x91, 0x68, 0x72, 0xB0, 0x21, 0x06, 0xFB, 0x40, 0x5E, 0xDD, 0x2F, 0x1A, 0x9F, 0xBE, 0x77, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x02, 0x01};
    CBORDecoder::decode(property_container, payload, sizeof(payload) / sizeof(uint8_t));

    REQUIRE(test == 1);
  }

  /************************************************************************************/

  WHEN("A payload containing a invalid CBOR key is parsed")
  {
    PropertyContainer property_container;
        
    CloudInt test = 0;
    addPropertyToContainer(property_container, test, "test", Permission::ReadWrite);

    /* [{123: 123, 0: "test", 2: 1}] = 81 A3 18 7B 18 7B 00 64 74 65 73 74 02 01 */
    uint8_t const payload[] = {0x81, 0xA3, 0x18, 0x7B, 0x18, 0x7B, 0x00, 0x64, 0x74, 0x65, 0x73, 0x74, 0x02, 0x01};
    CBORDecoder::decode(property_container, payload, sizeof(payload) / sizeof(uint8_t));

    REQUIRE(test == 1);
  }

  /************************************************************************************/
}
