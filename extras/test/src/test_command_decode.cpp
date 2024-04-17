/*
   Copyright (c) 2019 Arduino.  All rights reserved.
*/

/**************************************************************************************
   INCLUDE
 **************************************************************************************/

#include <catch.hpp>
#include <string.h>

#include <memory>

#include <util/CBORTestUtil.h>
#include <MessageDecoder.h>

/**************************************************************************************
   TEST CODE
 **************************************************************************************/

SCENARIO("Test the decoding of command messages") {
  /************************************************************************************/

  // FIXME
  // WHEN("Decode the ThingBeginCmd message")
  // {
  //   CommandDown command;

  //   /*

  //     DA 00010300                             # tag(66560)
  //       81                                    # array(1)
  //           78 24                             # text(36)
  //             65343439346435352D383732612D346664322D393634362D393266383739343933393463 # "e4494d55-872a-4fd2-9646-92f87949394c"

  //   */

  //   uint8_t const payload[] = {0xDA, 0x00, 0x01, 0x03, 0x00, 0x81, 0x78, 0x24, 0x65, 0x34, 0x34, 0x39, 0x34, 0x64, 0x35, 0x35, 0x2D, 0x38, 0x37, 0x32, 0x61, 0x2D, 0x34, 0x66, 0x64, 0x32, 0x2D, 0x39, 0x36, 0x34, 0x36, 0x2D, 0x39, 0x32, 0x66, 0x38, 0x37, 0x39, 0x34, 0x39, 0x33, 0x39, 0x34, 0x63};

  //   size_t payload_length = sizeof(payload) / sizeof(uint8_t);

  //   CBORMessageDecoder decoder;
  //   Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

  //   const char *thingIdToMatch = "e4494d55-872a-4fd2-9646-92f87949394c";

  //   THEN("The decode is successful") {
  //     REQUIRE(err == Decoder::Status::Complete);
  //     REQUIRE(strcmp(command.thingBeginCmd.params.thing_id, thingIdToMatch) == 0);
  //     REQUIRE(command.c.id == ThingBeginCmdId);
  //   }
  // }

  /************************************************************************************/

  WHEN("Decode the SetTimezoneCommand message")
  {
    CommandDown command;

    /*
      DA 00010764       # tag(67840)
        82             # array(2)
            1A 65DCB821 # unsigned(1708963873)
            1A 78ACA191 # unsigned(2024579473)
    */

    uint8_t const payload[] = {0xDA, 0x00, 0x01, 0x09, 0x00, 0x82, 0x1A, 0x65, 0xDC, 0xB8, 0x21, 0x1A, 0x78, 0xAC, 0xA1, 0x91};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is successful") {
      REQUIRE(err == Decoder::Status::Complete);
      REQUIRE(command.timezoneCommandDown.params.offset == (uint32_t)1708963873);
      REQUIRE(command.timezoneCommandDown.params.until == (uint32_t)2024579473);
      REQUIRE(command.c.id == TimezoneCommandDownId);
    }
  }

  /************************************************************************************/

  WHEN("Decode the LastValuesUpdateCmd message")
  {
    CommandDown command;

    /*
      DA 00010600                         # tag(67072)
        81                               # array(1)
            4D                            # bytes(13)
              00010203040506070809101112 # "\u0000\u0001\u0002\u0003\u0004\u0005\u0006\u0007\b\t\u0010\u0011\u0012"

    */

    uint8_t const payload[] = {0xDA, 0x00, 0x01, 0x06, 0x00, 0x81, 0x4D, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is successful") {
      REQUIRE(err == Decoder::Status::Complete);
      REQUIRE(command.lastValuesUpdateCmd.params.length == 13);
      REQUIRE(command.lastValuesUpdateCmd.params.last_values[0] == (uint8_t)0x00);
      REQUIRE(command.lastValuesUpdateCmd.params.last_values[1] == (uint8_t)0x01);
      REQUIRE(command.lastValuesUpdateCmd.params.last_values[2] == (uint8_t)0x02);
      REQUIRE(command.lastValuesUpdateCmd.params.last_values[3] == (uint8_t)0x03);
      REQUIRE(command.lastValuesUpdateCmd.params.last_values[4] == (uint8_t)0x04);
      REQUIRE(command.lastValuesUpdateCmd.params.last_values[5] == (uint8_t)0x05);
      REQUIRE(command.lastValuesUpdateCmd.params.last_values[6] == (uint8_t)0x06);
      REQUIRE(command.lastValuesUpdateCmd.params.last_values[7] == (uint8_t)0x07);
      REQUIRE(command.lastValuesUpdateCmd.params.last_values[8] == (uint8_t)0x08);
      REQUIRE(command.lastValuesUpdateCmd.params.last_values[9] == (uint8_t)0x09);
      REQUIRE(command.lastValuesUpdateCmd.params.last_values[10] == (uint8_t)0x10);
      REQUIRE(command.lastValuesUpdateCmd.params.last_values[11] == (uint8_t)0x11);
      REQUIRE(command.lastValuesUpdateCmd.params.last_values[12] == (uint8_t)0x12);
      REQUIRE(command.c.id == LastValuesUpdateCmdId);
    }
    free(command.lastValuesUpdateCmd.params.last_values);
  }

  /************************************************************************************/

  // FIXME
  // WHEN("Decode the OtaUpdateCmdDown message")
  // {
  //   CommandDown command;

  //   /*
  //     DA 00010100                             # tag(65792)
  //       84                                   # array(4)
  //           50                                # text(12)
  //             000102030405060708090A0B0C0D0E0F
  //           75                                # text(21)
  //             383736313233383736313233383736313233313233 # "876123876123876123123"
  //           53                                # bytes(19)
  //             33303034616162323735313164623332313264 # "3004aab27511db3212d"
  //           50                                # bytes(16)
  //             6A6B6173646B6A686173646B6A687868 # "jkasdkjhasdkjhxh"

  //   */

  //   uint8_t const payload[] = {0xDA, 0x00, 0x01, 0x01, 0x00, 0x84, 0x6C, 0x6F, 0x74, 0x61, 0x2D, 0x69, 0x64, 0x2D, 0x31, 0x32, 0x33, 0x34, 0x35, 0x75, 0x38, 0x37, 0x36, 0x31, 0x32, 0x33, 0x38, 0x37, 0x36, 0x31, 0x32, 0x33, 0x38, 0x37, 0x36, 0x31, 0x32, 0x33, 0x31, 0x32, 0x33, 0x53, 0x33, 0x30, 0x30, 0x34, 0x61, 0x61, 0x62, 0x32, 0x37, 0x35, 0x31, 0x31, 0x64, 0x62, 0x33, 0x32, 0x31, 0x32, 0x64, 0x50, 0x6A, 0x6B, 0x61, 0x73, 0x64, 0x6B, 0x6A, 0x68, 0x61, 0x73, 0x64, 0x6B, 0x6A, 0x68, 0x78, 0x68};

  //   size_t payload_length = sizeof(payload) / sizeof(uint8_t);
  //   CBORMessageDecoder decoder;
  //   Decoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

  //   uint8_t otaIdToMatch[ID_SIZE] = {};
  //   const char *urlToMatch   = "876123876123876123123";

  //   THEN("The decode is successful") {
  //     REQUIRE(err == Decoder::Status::Complete);
  //     REQUIRE(memcmp(command.otaUpdateCmdDown.params.id, otaIdToMatch, ID_SIZE) == 0);
  //     REQUIRE(strcmp(command.otaUpdateCmdDown.params.url, urlToMatch) == 0);
  //     // Initial SHA256 check
  //     REQUIRE(command.otaUpdateCmdDown.params.initialSha256[0] == (uint8_t)0x33);
  //     REQUIRE(command.otaUpdateCmdDown.params.initialSha256[1] == (uint8_t)0x30);
  //     REQUIRE(command.otaUpdateCmdDown.params.initialSha256[2] == (uint8_t)0x30);
  //     REQUIRE(command.otaUpdateCmdDown.params.initialSha256[3] == (uint8_t)0x34);
  //     REQUIRE(command.otaUpdateCmdDown.params.initialSha256[4] == (uint8_t)0x61);
  //     REQUIRE(command.otaUpdateCmdDown.params.initialSha256[5] == (uint8_t)0x61);
  //     REQUIRE(command.otaUpdateCmdDown.params.initialSha256[6] == (uint8_t)0x62);
  //     REQUIRE(command.otaUpdateCmdDown.params.initialSha256[7] == (uint8_t)0x32);

  //     // Final SHA256 check
  //     REQUIRE(command.otaUpdateCmdDown.params.finalSha256[0] == (uint8_t)0x6A);
  //     REQUIRE(command.otaUpdateCmdDown.params.finalSha256[1] == (uint8_t)0x6B);
  //     REQUIRE(command.otaUpdateCmdDown.params.finalSha256[2] == (uint8_t)0x61);
  //     REQUIRE(command.otaUpdateCmdDown.params.finalSha256[3] == (uint8_t)0x73);
  //     REQUIRE(command.otaUpdateCmdDown.params.finalSha256[4] == (uint8_t)0x64);
  //     REQUIRE(command.otaUpdateCmdDown.params.finalSha256[5] == (uint8_t)0x6B);

  //     REQUIRE(command.c.id == OtaUpdateCmdDownId);
  //   }
  // }
}
