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
#include <catch2/matchers/catch_matchers_vector.hpp>
#include <string.h>
#include <memory>
#include <util/CBORTestUtil.h>
#include <IoTCloudMessageEncoder.h>
#include <MessageDecoder.h>

/******************************************************************************
  TEST CODE
 ******************************************************************************/

SCENARIO("Test the decoding of command messages") {

  WHEN("Decode the ThingUpdateCmdId message")
  {
    CommandDown command;
    /*
      DA 00010400                             # tag(66560)
          81                                  # array(1)
              78 24                           # text(36)
                65343439346435352D383732612D346664322D393634362D393266383739343933393463 # "e4494d55-872a-4fd2-9646-92f87949394c"
    */
    uint8_t const payload[] = {0xDA, 0x00, 0x01, 0x04, 0x00, 0x81, 0x78, 0x24,
                               0x65, 0x34, 0x34, 0x39, 0x34, 0x64, 0x35, 0x35,
                               0x2D, 0x38, 0x37, 0x32, 0x61, 0x2D, 0x34, 0x66,
                               0x64, 0x32, 0x2D, 0x39, 0x36, 0x34, 0x36, 0x2D,
                               0x39, 0x32, 0x66, 0x38, 0x37, 0x39, 0x34, 0x39,
                               0x33, 0x39, 0x34, 0x63};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    MessageDecoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);
    const char *thingIdToMatch = "e4494d55-872a-4fd2-9646-92f87949394c";

    THEN("The decode is successful") {
      REQUIRE(err == MessageDecoder::Status::Complete);
      REQUIRE(strcmp(command.thingUpdateCmd.params.thing_id, thingIdToMatch) == 0);
      REQUIRE(command.c.id == ThingUpdateCmdId);
    }
  }

  WHEN("Decode the ThingDetachCmd message")
  {
    CommandDown command;
    /*
      DA 00011000                             # tag(69632)
        81                                    # array(1)
            78 24                             # text(36)
              65343439346435352D383732612D346664322D393634362D393266383739343933393463 # "e4494d55-872a-4fd2-9646-92f87949394c"
    */
    uint8_t const payload[] = {0xDA, 0x00, 0x01, 0x10, 0x00, 0x81, 0x78, 0x24,
                               0x65, 0x34, 0x34, 0x39, 0x34, 0x64, 0x35, 0x35,
                               0x2D, 0x38, 0x37, 0x32, 0x61, 0x2D, 0x34, 0x66,
                               0x64, 0x32, 0x2D, 0x39, 0x36, 0x34, 0x36, 0x2D,
                               0x39, 0x32, 0x66, 0x38, 0x37, 0x39, 0x34, 0x39,
                               0x33, 0x39, 0x34, 0x63};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    MessageDecoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);
    const char *thingIdToMatch = "e4494d55-872a-4fd2-9646-92f87949394c";

    THEN("The decode is successful") {
      REQUIRE(err == MessageDecoder::Status::Complete);
      REQUIRE(strcmp(command.thingDetachCmd.params.thing_id, thingIdToMatch) == 0);
      REQUIRE(command.c.id == ThingDetachCmdId);
    }
  }

  WHEN("Decode the ThingUpdateCmdId message containing a number instead of a string")
  {
    CommandDown command;
    /*
      DA 00010400                             # tag(66560)
          81                                  # array(1)
              1A 65DCB821                     # unsigned(1708963873)
    */
    uint8_t const payload[] = {0xDA, 0x00, 0x01, 0x04, 0x00, 0x81, 0x1A, 0x65,
                               0xDC, 0xB8, 0x21};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    MessageDecoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is unsuccessful") {
      REQUIRE(err == MessageDecoder::Status::Error);
    }
  }

  WHEN("Decode the ThingDetachCmd message containing a number instead of a string")
  {
    CommandDown command;
    /*
      DA 00011000                             # tag(69632)
        81                                    # array(1)
              1A 65DCB821                     # unsigned(1708963873)
    */
    uint8_t const payload[] = {0xDA, 0x00, 0x01, 0x10, 0x00, 0x81, 0x1A, 0x65,
                               0xDC, 0xB8, 0x21};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    MessageDecoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is successful") {
      REQUIRE(err == MessageDecoder::Status::Error);
    }
  }

  WHEN("Decode the TimezoneCommandDown message")
  {
    CommandDown command;

    /*
      DA 00010764       # tag(67840)
        82              # array(2)
            1A 65DCB821 # unsigned(1708963873)
            1A 78ACA191 # unsigned(2024579473)
    */

    uint8_t const payload[] = {0xDA, 0x00, 0x01, 0x09, 0x00, 0x82, 0x1A, 0x65,
                               0xDC, 0xB8, 0x21, 0x1A, 0x78, 0xAC, 0xA1, 0x91};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    MessageDecoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is successful") {
      REQUIRE(err == MessageDecoder::Status::Complete);
      REQUIRE(command.timezoneCommandDown.params.offset == (uint32_t)1708963873);
      REQUIRE(command.timezoneCommandDown.params.until == (uint32_t)2024579473);
      REQUIRE(command.c.id == TimezoneCommandDownId);
    }
  }

  WHEN("Decode the TimezoneCommandDown message, but until is not present")
  {
    CommandDown command;

    /*
      DA 00010764       # tag(67840)
        81              # array(1)
            1A 65DCB821 # unsigned(1708963873)
    */

    uint8_t const payload[] = {0xDA, 0x00, 0x01, 0x09, 0x00, 0x81, 0x1A, 0x65,
                               0xDC, 0xB8, 0x21};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    MessageDecoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is unsuccessful") {
      REQUIRE(err == MessageDecoder::Status::Error);
    }
  }

  WHEN("Decode the TimezoneCommandDown message, but offset is incorrectly encoded")
  {
    CommandDown command;

    /*
      DA 00010764       # tag(67840)
        81              # array(1)
            1A 65DC     # unsigned(26076)
    */

    uint8_t const payload[] = {0xDA, 0x00, 0x01, 0x09, 0x00, 0x81, 0x1A,};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    MessageDecoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is unsuccessful") {
      REQUIRE(err == MessageDecoder::Status::Error);
    }
  }

  WHEN("Decode the TimezoneCommandDown message, but offset is a byte array instead of an integer")
  {
    CommandDown command;

    /*
      DA 00010764                        # tag(67840)
        81                               # array(2)
            4D                           # bytes(13)
              00010203040506070809101112 # "\u0000\u0001\u0002\u0003\u0004\u0005\u0006\u0007\b\t\u0010\u0011\u0012"
            1A 65DCB821                  # unsigned(1708963873)
    */

    uint8_t const payload[] = {0xDA, 0x00, 0x01, 0x09, 0x00, 0x81, 0x21, 0x4D,
                               0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                               0x08, 0x09, 0x10, 0x11, 0x12, 0x1A, 0x65, 0xDC,
                               0xB8,};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    MessageDecoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is unsuccessful") {
      REQUIRE(err == MessageDecoder::Status::Error);
    }
  }

  WHEN("Decode the TimezoneCommandDown message, but until is a byte array instead of an integer")
  {
    CommandDown command;

    /*
      DA 00010764                        # tag(67840)
        82                               # array(2)
            1A 65DCB821                  # unsigned(1708963873)
            4D                           # bytes(13)
              00010203040506070809101112 # "\u0000\u0001\u0002\u0003\u0004\u0005\u0006\u0007\b\t\u0010\u0011\u0012"
    */

    uint8_t const payload[] = {0xDA, 0x00, 0x01, 0x09, 0x00, 0x82, 0x1A, 0x65,
                               0xDC, 0xB8, 0x21, 0x4D, 0x00, 0x01, 0x02, 0x03,
                               0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11,
                               0x12};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    MessageDecoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is unsuccessful") {
      REQUIRE(err == MessageDecoder::Status::Error);
    }
  }

  WHEN("Decode the TimezoneCommandDown message, but until is incorrectly encoded")
  {
    CommandDown command;

    /*
      DA 00010764       # tag(67840)
        82              # array(2)
            1A 65DCB821 # unsigned(1708963873)
            1A 78AC     # unsigned(30892)
    */

    uint8_t const payload[] = {0xDA, 0x00, 0x01, 0x09, 0x00, 0x82, 0x1A, 0x65,
                               0xDC, 0xB8, 0x21, 0x1A, 0x78, 0xAC};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    MessageDecoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is unsuccessful") {
      REQUIRE(err == MessageDecoder::Status::Error);
    }
  }

  WHEN("Decode the LastValuesUpdateCmd message")
  {
    CommandDown command;

    /*
      DA 00010600                        # tag(67072)
        81                               # array(1)
            4D                           # bytes(13)
              00010203040506070809101112 # "\u0000\u0001\u0002\u0003\u0004\u0005\u0006\u0007\b\t\u0010\u0011\u0012"

    */

    uint8_t const payload[] = {0xDA, 0x00, 0x01, 0x06, 0x00, 0x81, 0x4D, 0x00,
                               0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                               0x09, 0x10, 0x11, 0x12};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    MessageDecoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is successful") {
      REQUIRE(err == MessageDecoder::Status::Complete);

      std::vector<int> last_values(command.lastValuesUpdateCmd.params.last_values,
        command.lastValuesUpdateCmd.params.last_values+13);

      REQUIRE_THAT(last_values,
        Catch::Matchers::Equals(std::vector<int>{
          0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12,
      }));

      REQUIRE(command.c.id == LastValuesUpdateCmdId);
    }
    free(command.lastValuesUpdateCmd.params.last_values);
  }

  WHEN("Decode the LastValuesUpdateCmd message, but lastvalues is an integer")
  {
    CommandDown command;

    /*
      DA 00010600                        # tag(67072)
        81                               # array(1)
            1A 65DCB821                  # unsigned(1708963873)

    */

    uint8_t const payload[] = {0xDA, 0x00, 0x01, 0x06, 0x00, 0x81, 0x1A, 0x65,
                               0xDC, 0xB8, 0x21};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    MessageDecoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is unsuccessful") {
      REQUIRE(err == MessageDecoder::Status::Error);
    }
  }

  WHEN("Decode the OtaUpdateCmdDown message")
  {
    CommandDown command;

    /*
      DA 00010100                             # tag(65792)
        84                                    # array(4)
            50                                # bytes(16)
              C73CB045F9C2434585AFFA36A307BFE7"\xC7<\xB0E\xF9\xC2CE\x85\xAF\xFA6\xA3\a\xBF\xE7"
            78 72                             # text(141)
              68747470733A2F2F626F617264732D69
              6E742E6F6E69756472612E63632F7374
              6F726167652F6669726D776172652F76
              312F6466316561633963376264363334
              37336666666231313766393837333730
              33653465633935353933316532363766
              32363236326230393439626331366463
              3439                            # "https://boards-int.oniudra.cc/storage/firmware/v1/df1eac9c7bd63473fffb117f9873703e4ec955931e267f26262b0949bc16dc49"
            58 20                             # bytes(32)
              00000000000000000000000000000000
              00000000000000000000000000000000# "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000"
            58 20                             # bytes(32)
              DF1EAC9C7BD63473FFFB117F9873703E
              4EC955931E267F26262B0949BC16DC49# "\xDF\u001E\xAC\x9C{\xD64s\xFF\xFB\u0011\u007F\x98sp>N\xC9U\x93\u001E&\u007F&&+\tI\xBC\u0016\xDCI"

    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x01, 0x00, 0x84, 0x50, 0xc7,
                               0x3c, 0xb0, 0x45, 0xf9, 0xc2, 0x43, 0x45, 0x85,
                               0xaf, 0xfa, 0x36, 0xa3, 0x07, 0xbf, 0xe7, 0x78,
                               0x72, 0x68, 0x74, 0x74, 0x70, 0x73, 0x3a, 0x2f,
                               0x2f, 0x62, 0x6f, 0x61, 0x72, 0x64, 0x73, 0x2d,
                               0x69, 0x6e, 0x74, 0x2e, 0x6f, 0x6e, 0x69, 0x75,
                               0x64, 0x72, 0x61, 0x2e, 0x63, 0x63, 0x2f, 0x73,
                               0x74, 0x6f, 0x72, 0x61, 0x67, 0x65, 0x2f, 0x66,
                               0x69, 0x72, 0x6d, 0x77, 0x61, 0x72, 0x65, 0x2f,
                               0x76, 0x31, 0x2f, 0x64, 0x66, 0x31, 0x65, 0x61,
                               0x63, 0x39, 0x63, 0x37, 0x62, 0x64, 0x36, 0x33,
                               0x34, 0x37, 0x33, 0x66, 0x66, 0x66, 0x62, 0x31,
                               0x31, 0x37, 0x66, 0x39, 0x38, 0x37, 0x33, 0x37,
                               0x30, 0x33, 0x65, 0x34, 0x65, 0x63, 0x39, 0x35,
                               0x35, 0x39, 0x33, 0x31, 0x65, 0x32, 0x36, 0x37,
                               0x66, 0x32, 0x36, 0x32, 0x36, 0x32, 0x62, 0x30,
                               0x39, 0x34, 0x39, 0x62, 0x63, 0x31, 0x36, 0x64,
                               0x63, 0x34, 0x39, 0x58, 0x20, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x58, 0x20, 0xdf,
                               0x1e, 0xac, 0x9c, 0x7b, 0xd6, 0x34, 0x73, 0xff,
                               0xfb, 0x11, 0x7f, 0x98, 0x73, 0x70, 0x3e, 0x4e,
                               0xc9, 0x55, 0x93, 0x1e, 0x26, 0x7f, 0x26, 0x26,
                               0x2b, 0x09, 0x49, 0xbc, 0x16, 0xdc, 0x49};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    MessageDecoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    uint8_t otaIdToMatch[ID_SIZE] = { 0xC7, 0x3C, 0xB0, 0x45, 0xF9, 0xC2, 0x43, 0x45,
                                      0x85, 0xAF, 0xFA, 0x36, 0xA3, 0x07, 0xBF, 0xE7};
    const char *urlToMatch   = "https://boards-int.oniudra.cc/storage/firmware/v1/df1eac9c7bd63473fffb117f9873703e4ec955931e267f26262b0949bc16dc49";

    THEN("The decode is successful") {
      REQUIRE(err == MessageDecoder::Status::Complete);
      REQUIRE(memcmp(command.otaUpdateCmdDown.params.id, otaIdToMatch, ID_SIZE) == 0);
      REQUIRE(strcmp(command.otaUpdateCmdDown.params.url, urlToMatch) == 0);

      std::vector<int> initialSha256(command.otaUpdateCmdDown.params.initialSha256,
        command.otaUpdateCmdDown.params.initialSha256+32);

      REQUIRE_THAT(initialSha256,
        Catch::Matchers::Equals(std::vector<int>{
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
      }));

      std::vector<int> finalSha256(command.otaUpdateCmdDown.params.finalSha256,
        command.otaUpdateCmdDown.params.finalSha256+32);

      REQUIRE_THAT(finalSha256,
        Catch::Matchers::Equals(std::vector<int>{
          0xdf, 0x1e, 0xac, 0x9c, 0x7b, 0xd6, 0x34, 0x73, 0xff, 0xfb, 0x11, 0x7f, 0x98, 0x73, 0x70, 0x3e,
          0x4e, 0xc9, 0x55, 0x93, 0x1e, 0x26, 0x7f, 0x26, 0x26, 0x2b, 0x09, 0x49, 0xbc, 0x16, 0xdc, 0x49
      }));

      REQUIRE(command.c.id == OtaUpdateCmdDownId);
    }
  }

  WHEN("Decode the OtaUpdateCmdDown message with out of order fields 1")
  {
    CommandDown command;

    /*
      DA 00010100                             # tag(65792)
        84                                    # array(4)
            78 72                             # text(141)
              68747470733A2F2F626F617264732D69
              6E742E6F6E69756472612E63632F7374
              6F726167652F6669726D776172652F76
              312F6466316561633963376264363334
              37336666666231313766393837333730
              33653465633935353933316532363766
              32363236326230393439626331366463
              3439                            # "https://boards-int.oniudra.cc/storage/firmware/v1/df1eac9c7bd63473fffb117f9873703e4ec955931e267f26262b0949bc16dc49"
            50                                # bytes(16)
              C73CB045F9C2434585AFFA36A307BFE7"\xC7<\xB0E\xF9\xC2CE\x85\xAF\xFA6\xA3\a\xBF\xE7"
            58 20                             # bytes(32)
              00000000000000000000000000000000
              00000000000000000000000000000000# "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000"
            58 20                             # bytes(32)
              DF1EAC9C7BD63473FFFB117F9873703E
              4EC955931E267F26262B0949BC16DC49# "\xDF\u001E\xAC\x9C{\xD64s\xFF\xFB\u0011\u007F\x98sp>N\xC9U\x93\u001E&\u007F&&+\tI\xBC\u0016\xDCI"

    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x01, 0x00, 0x84, 0x78, 0x72,
                               0x68, 0x74, 0x74, 0x70, 0x73, 0x3a, 0x2f, 0x2f,
                               0x62, 0x6f, 0x61, 0x72, 0x64, 0x73, 0x2d, 0x69,
                               0x6e, 0x74, 0x2e, 0x6f, 0x6e, 0x69, 0x75, 0x64,
                               0x72, 0x61, 0x2e, 0x63, 0x63, 0x2f, 0x73, 0x74,
                               0x6f, 0x72, 0x61, 0x67, 0x65, 0x2f, 0x66, 0x69,
                               0x72, 0x6d, 0x77, 0x61, 0x72, 0x65, 0x2f, 0x76,
                               0x31, 0x2f, 0x64, 0x66, 0x31, 0x65, 0x61, 0x63,
                               0x39, 0x63, 0x37, 0x62, 0x64, 0x36, 0x33, 0x34,
                               0x37, 0x33, 0x66, 0x66, 0x66, 0x62, 0x31, 0x31,
                               0x37, 0x66, 0x39, 0x38, 0x37, 0x33, 0x37, 0x30,
                               0x33, 0x65, 0x34, 0x65, 0x63, 0x39, 0x35, 0x35,
                               0x39, 0x33, 0x31, 0x65, 0x32, 0x36, 0x37, 0x66,
                               0x32, 0x36, 0x32, 0x36, 0x32, 0x62, 0x30, 0x39,
                               0x34, 0x39, 0x62, 0x63, 0x31, 0x36, 0x64, 0x63,
                               0x34, 0x39, 0x50, 0xc7, 0x3c, 0xb0, 0x45, 0xf9,
                               0xc2, 0x43, 0x45, 0x85, 0xaf, 0xfa, 0x36, 0xa3,
                               0x07, 0xbf, 0xe7, 0x58, 0x20, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x58, 0x20, 0xdf,
                               0x1e, 0xac, 0x9c, 0x7b, 0xd6, 0x34, 0x73, 0xff,
                               0xfb, 0x11, 0x7f, 0x98, 0x73, 0x70, 0x3e, 0x4e,
                               0xc9, 0x55, 0x93, 0x1e, 0x26, 0x7f, 0x26, 0x26,
                               0x2b, 0x09, 0x49, 0xbc, 0x16, 0xdc, 0x49};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    MessageDecoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is successful") {
      REQUIRE(err == MessageDecoder::Status::Error);
    }
  }

  WHEN("Decode the OtaUpdateCmdDown message with out of order fields 2")
  {
    CommandDown command;

    /*
      DA 00010100                             # tag(65792)
        84                                    # array(4)
            50                                # bytes(16)
              C73CB045F9C2434585AFFA36A307BFE7"\xC7<\xB0E\xF9\xC2CE\x85\xAF\xFA6\xA3\a\xBF\xE7"
            58 20                             # bytes(32)
              00000000000000000000000000000000
              00000000000000000000000000000000# "\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000"
            78 72                             # text(141)
              68747470733A2F2F626F617264732D69
              6E742E6F6E69756472612E63632F7374
              6F726167652F6669726D776172652F76
              312F6466316561633963376264363334
              37336666666231313766393837333730
              33653465633935353933316532363766
              32363236326230393439626331366463
              3439                            # "https://boards-int.oniudra.cc/storage/firmware/v1/df1eac9c7bd63473fffb117f9873703e4ec955931e267f26262b0949bc16dc49"
            58 20                             # bytes(32)
              DF1EAC9C7BD63473FFFB117F9873703E
              4EC955931E267F26262B0949BC16DC49# "\xDF\u001E\xAC\x9C{\xD64s\xFF\xFB\u0011\u007F\x98sp>N\xC9U\x93\u001E&\u007F&&+\tI\xBC\u0016\xDCI"

    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x01, 0x00, 0x84, 0x50, 0xc7,
                               0x3c, 0xb0, 0x45, 0xf9, 0xc2, 0x43, 0x45, 0x85,
                               0xaf, 0xfa, 0x36, 0xa3, 0x07, 0xbf, 0xe7, 0x58,
                               0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x78, 0x72, 0x68, 0x74, 0x74, 0x70, 0x73,
                               0x3a, 0x2f, 0x2f, 0x62, 0x6f, 0x61, 0x72, 0x64,
                               0x73, 0x2d, 0x69, 0x6e, 0x74, 0x2e, 0x6f, 0x6e,
                               0x69, 0x75, 0x64, 0x72, 0x61, 0x2e, 0x63, 0x63,
                               0x2f, 0x73, 0x74, 0x6f, 0x72, 0x61, 0x67, 0x65,
                               0x2f, 0x66, 0x69, 0x72, 0x6d, 0x77, 0x61, 0x72,
                               0x65, 0x2f, 0x76, 0x31, 0x2f, 0x64, 0x66, 0x31,
                               0x65, 0x61, 0x63, 0x39, 0x63, 0x37, 0x62, 0x64,
                               0x36, 0x33, 0x34, 0x37, 0x33, 0x66, 0x66, 0x66,
                               0x62, 0x31, 0x31, 0x37, 0x66, 0x39, 0x38, 0x37,
                               0x33, 0x37, 0x30, 0x33, 0x65, 0x34, 0x65, 0x63,
                               0x39, 0x35, 0x35, 0x39, 0x33, 0x31, 0x65, 0x32,
                               0x36, 0x37, 0x66, 0x32, 0x36, 0x32, 0x36, 0x32,
                               0x62, 0x30, 0x39, 0x34, 0x39, 0x62, 0x63, 0x31,
                               0x36, 0x64, 0x63, 0x34, 0x39, 0x58, 0x20, 0xdf,
                               0x1e, 0xac, 0x9c, 0x7b, 0xd6, 0x34, 0x73, 0xff,
                               0xfb, 0x11, 0x7f, 0x98, 0x73, 0x70, 0x3e, 0x4e,
                               0xc9, 0x55, 0x93, 0x1e, 0x26, 0x7f, 0x26, 0x26,
                               0x2b, 0x09, 0x49, 0xbc, 0x16, 0xdc, 0x49};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    MessageDecoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is successful") {
      REQUIRE(err == MessageDecoder::Status::Error);
    }
  }

  WHEN("Decode the OtaUpdateCmdDown message with corrupted fields 1")
  {
    CommandDown command;

    /*
      DA 00010100                             # tag(65792)
        84                                    # array(4)
            50                                # bytes(16)
              C73CB045F9C2434585AFFA36A307BFE7"\xC7<\xB0E\xF9\xC2CE\x85\xAF\xFA6\xA3\a\xBF\xE7"
            78 72                             # text(141)
              68747470733A2F2F626F617264732D69
              6E742E6F6E69756472612E63632F7374
              6F726167652F6669726D776172652F76
              312F6466316561633963376264363334
              37336666666231313766393837333730
              33653465633935353933316532363766
              32363236326230393439626331366463
              3439                            # "https://boards-int.oniudra.cc/storage/firmware/v1/df1eac9c7bd63473fffb117f9873703e4ec955931e267f26262b0949bc16dc49"
            1A 65DCB821                       # unsigned(1708963873)
            58 20                             # bytes(32)
              DF1EAC9C7BD63473FFFB117F9873703E
              4EC955931E267F26262B0949BC16DC49# "\xDF\u001E\xAC\x9C{\xD64s\xFF\xFB\u0011\u007F\x98sp>N\xC9U\x93\u001E&\u007F&&+\tI\xBC\u0016\xDCI"

    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x01, 0x00, 0x84, 0x50, 0xc7,
                               0x3c, 0xb0, 0x45, 0xf9, 0xc2, 0x43, 0x45, 0x85,
                               0xaf, 0xfa, 0x36, 0xa3, 0x07, 0xbf, 0xe7, 0x78,
                               0x72, 0x68, 0x74, 0x74, 0x70, 0x73, 0x3a, 0x2f,
                               0x2f, 0x62, 0x6f, 0x61, 0x72, 0x64, 0x73, 0x2d,
                               0x69, 0x6e, 0x74, 0x2e, 0x6f, 0x6e, 0x69, 0x75,
                               0x64, 0x72, 0x61, 0x2e, 0x63, 0x63, 0x2f, 0x73,
                               0x74, 0x6f, 0x72, 0x61, 0x67, 0x65, 0x2f, 0x66,
                               0x69, 0x72, 0x6d, 0x77, 0x61, 0x72, 0x65, 0x2f,
                               0x76, 0x31, 0x2f, 0x64, 0x66, 0x31, 0x65, 0x61,
                               0x63, 0x39, 0x63, 0x37, 0x62, 0x64, 0x36, 0x33,
                               0x34, 0x37, 0x33, 0x66, 0x66, 0x66, 0x62, 0x31,
                               0x31, 0x37, 0x66, 0x39, 0x38, 0x37, 0x33, 0x37,
                               0x30, 0x33, 0x65, 0x34, 0x65, 0x63, 0x39, 0x35,
                               0x35, 0x39, 0x33, 0x31, 0x65, 0x32, 0x36, 0x37,
                               0x66, 0x32, 0x36, 0x32, 0x36, 0x32, 0x62, 0x30,
                               0x39, 0x34, 0x39, 0x62, 0x63, 0x31, 0x36, 0x64,
                               0x63, 0x34, 0x39, 0x1A, 0x65, 0xDC, 0xB8, 0x21,
                               0x58, 0x20, 0xdf, 0x1e, 0xac, 0x9c, 0x7b, 0xd6,
                               0x34, 0x73, 0xff, 0xfb, 0x11, 0x7f, 0x98, 0x73,
                               0x70, 0x3e, 0x4e, 0xc9, 0x55, 0x93, 0x1e, 0x26,
                               0x7f, 0x26, 0x26, 0x2b, 0x09, 0x49, 0xbc, 0x16,
                               0xdc, 0x49};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    MessageDecoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is successful") {
      REQUIRE(err == MessageDecoder::Status::Error);
    }
  }

  WHEN("Decode the OtaUpdateCmdDown message with corrupted fields 2")
  {
    CommandDown command;

    /*
      DA 00010100                             # tag(65792)
        84                                    # array(4)
            50                                # bytes(16)
              C73CB045F9C2434585AFFA36A307BFE7"\xC7<\xB0E\xF9\xC2CE\x85\xAF\xFA6\xA3\a\xBF\xE7"
            78 72                             # text(141)
              68747470733A2F2F626F617264732D69
              6E742E6F6E69756472612E63632F7374
              6F726167652F6669726D776172652F76
              312F6466316561633963376264363334
              37336666666231313766393837333730
              33653465633935353933316532363766
              32363236326230393439626331366463
              3439                            # "https://boards-int.oniudra.cc/storage/firmware/v1/df1eac9c7bd63473fffb117f9873703e4ec955931e267f26262b0949bc16dc49"
            58 20                             # bytes(32)
              DF1EAC9C7BD63473FFFB117F9873703E
              4EC955931E267F26262B0949BC16DC49# "\xDF\u001E\xAC\x9C{\xD64s\xFF\xFB\u0011\u007F\x98sp>N\xC9U\x93\u001E&\u007F&&+\tI\xBC\u0016\xDCI"
            1A 65DCB821                       # unsigned(1708963873)

    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x01, 0x00, 0x84, 0x50, 0xc7,
                               0x3c, 0xb0, 0x45, 0xf9, 0xc2, 0x43, 0x45, 0x85,
                               0xaf, 0xfa, 0x36, 0xa3, 0x07, 0xbf, 0xe7, 0x78,
                               0x72, 0x68, 0x74, 0x74, 0x70, 0x73, 0x3a, 0x2f,
                               0x2f, 0x62, 0x6f, 0x61, 0x72, 0x64, 0x73, 0x2d,
                               0x69, 0x6e, 0x74, 0x2e, 0x6f, 0x6e, 0x69, 0x75,
                               0x64, 0x72, 0x61, 0x2e, 0x63, 0x63, 0x2f, 0x73,
                               0x74, 0x6f, 0x72, 0x61, 0x67, 0x65, 0x2f, 0x66,
                               0x69, 0x72, 0x6d, 0x77, 0x61, 0x72, 0x65, 0x2f,
                               0x76, 0x31, 0x2f, 0x64, 0x66, 0x31, 0x65, 0x61,
                               0x63, 0x39, 0x63, 0x37, 0x62, 0x64, 0x36, 0x33,
                               0x34, 0x37, 0x33, 0x66, 0x66, 0x66, 0x62, 0x31,
                               0x31, 0x37, 0x66, 0x39, 0x38, 0x37, 0x33, 0x37,
                               0x30, 0x33, 0x65, 0x34, 0x65, 0x63, 0x39, 0x35,
                               0x35, 0x39, 0x33, 0x31, 0x65, 0x32, 0x36, 0x37,
                               0x66, 0x32, 0x36, 0x32, 0x36, 0x32, 0x62, 0x30,
                               0x39, 0x34, 0x39, 0x62, 0x63, 0x31, 0x36, 0x64,
                               0x63, 0x34, 0x39, 0x58, 0x20, 0xdf, 0x1e, 0xac,
                               0x9c, 0x7b, 0xd6, 0x34, 0x73, 0xff, 0xfb, 0x11,
                               0x7f, 0x98, 0x73, 0x70, 0x3e, 0x4e, 0xc9, 0x55,
                               0x93, 0x1e, 0x26, 0x7f, 0x26, 0x26, 0x2b, 0x09,
                               0x49, 0xbc, 0x16, 0xdc, 0x49, 0x1A, 0x65, 0xDC,
                               0xB8, 0x21};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    MessageDecoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is successful") {
      REQUIRE(err == MessageDecoder::Status::Error);
    }
  }

  WHEN("Decode the OtaUpdateCmdDown message without url field")
  {
    CommandDown command;

    /*
      DA 00010100                             # tag(65792)
        80                                    # array(1)
            50                                # bytes(16)
              C73CB045F9C2434585AFFA36A307BFE7"\xC7<\xB0E\xF9\xC2CE\x85\xAF\xFA6\xA3\a\xBF\xE7"

    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x01, 0x00, 0x81, 0x50, 0xc7,
                               0x3c, 0xb0, 0x45, 0xf9, 0xc2, 0x43, 0x45, 0x85,
                               0xaf, 0xfa, 0x36, 0xa3, 0x07, 0xbf, 0xe7};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    MessageDecoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is unsuccessful") {
      REQUIRE(err == MessageDecoder::Status::Error);
    }
  }

  WHEN("Decode the OtaBeginUp message")
  {
    CommandDown command;
    /*
      DA 00010000                             # tag(65536)
         81                                   # array(1)
            58 20                             # bytes(32)
               01020304
    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x00, 0x00, 0x81, 0x58, 0x20,
                               0x01, 0x02, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    MessageDecoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is unsuccessful - OtaBeginUp is not supported") {
      REQUIRE(err == MessageDecoder::Status::Error);
    }
  }

  WHEN("Decode the ThingBeginCmd message")
  {
    CommandDown command;
    /*
      DA 00010300               # tag(66304)
         81                     # array(1)
            68                  # text(8)
               7468696E675F6964 # "thing_id"
    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x03, 0x00, 0x81, 0x68, 0x74,
                               0x68, 0x69, 0x6e, 0x67, 0x5f, 0x69, 0x64};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    MessageDecoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is unsuccessful - ThingBeginCmd is not supported") {
      REQUIRE(err == MessageDecoder::Status::Error);
    }
  }

  WHEN("Decode the LastValuesBeginCmd message")
  {
    CommandDown command;
    /*
       DA 00010500 # tag(66816)
         80        # array(0)
    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x05, 0x00, 0x80};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    MessageDecoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is unsuccessful - LastValuesBeginCmd is not supported") {
      REQUIRE(err == MessageDecoder::Status::Error);
    }
  }

  WHEN("Decode the DeviceBeginCmd message")
  {
    CommandDown command;
    /*
      DA 00010700         # tag(67328)
         81               # array(1)
            65            # text(5)
               322E302E30 # "2.0.0"
    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x07, 0x00, 0x81, 0x65, 0x32,
                               0x2e, 0x30, 0x2e, 0x30};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    MessageDecoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is unsuccessful - DeviceBeginCmd is not supported") {
      REQUIRE(err == MessageDecoder::Status::Error);
    }
  }

  WHEN("Decode the OtaProgressCmdUp message")
  {
    CommandDown command;
    /*
      DA 00010200                             # tag(66048)
         84                                   # array(4)
            50                                # bytes(16)
               000102030405060708090A0B0C0D0E0F # "\u0000\u0001\u0002\u0003\u0004\u0005\u0006\u0007\b\t\n\u000b\f\r\u000e\u000f"
            E1                                # primitive(1)
            20                                # negative(0)
            18 64                             # unsigned(100)
    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x02, 0x00, 0x84, 0x50, 0x00,
                               0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                               0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0xe1,
                               0x20, 0x18, 0x64};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    MessageDecoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is unsuccessful - OtaProgressCmdUp is not supported") {
      REQUIRE(err == MessageDecoder::Status::Error);
    }
  }

  WHEN("Decode the TimezoneCommandUp message")
  {
    CommandDown command;
    /*
      DA 00010800 # tag(67584)
         80       # array(0)
    */
    uint8_t const payload[] = {0xda, 0x00, 0x01, 0x08, 0x00, 0x80};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    MessageDecoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is unsuccessful - TimezoneCommandUp is not supported") {
      REQUIRE(err == MessageDecoder::Status::Error);
    }
  }

  WHEN("Decode a message with invalid CBOR tag")
  {
    CommandDown command;

    /*
      DA ffffffff       # invalid tag
        82              # array(2)
            1A 65DCB821 # unsigned(1708963873)
            1A 78ACA191 # unsigned(2024579473)
    */

    uint8_t const payload[] = {0xDA, 0xff, 0xff, 0xff, 0xff, 0x82, 0x1A, 0x65,
                               0xDC, 0xB8, 0x21, 0x1A, 0x78, 0xAC, 0xA1, 0x91};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    MessageDecoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is unsuccessful") {
      REQUIRE(err == MessageDecoder::Status::Error);
    }
  }

  WHEN("Decode a message not starting with a CBOR tag")
  {
    CommandDown command;

    /*
        82              # array(2)
            1A 65DCB821 # unsigned(1708963873)
            1A 78ACA191 # unsigned(2024579473)
    */

    uint8_t const payload[] = {0x82, 0x1A, 0x65, 0xDC, 0xB8, 0x21, 0x1A, 0x78,
                               0xAC, 0xA1, 0x91};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    MessageDecoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is unsuccessful") {
      REQUIRE(err == MessageDecoder::Status::Error);
    }
  }

  WHEN("Decode an invalid CBOR message")
  {
    CommandDown command;

    uint8_t const payload[] = {0xFF};

    size_t payload_length = sizeof(payload) / sizeof(uint8_t);
    CBORMessageDecoder decoder;
    MessageDecoder::Status err =  decoder.decode((Message*)&command, payload, payload_length);

    THEN("The decode is unsuccessful") {
      REQUIRE(err == MessageDecoder::Status::Error);
    }
  }
}
