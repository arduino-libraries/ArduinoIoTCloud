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
#include <memory>
#include <util/CBORTestUtil.h>
#include <IoTCloudMessageEncoder.h>
#include <MessageEncoder.h>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include <ConnectionHandlerDefinitions.h>

/******************************************************************************
  TEST CODE
 ******************************************************************************/

SCENARIO("Test the encoding of command messages") {

  WHEN("Encode the OtaBeginUp message")
  {
    OtaBeginUp command;
    uint8_t sha[SHA256_SIZE] = {0x01, 0x02, 0x03, 0x04};
    memcpy(command.params.sha, sha, SHA256_SIZE);

    command.c.id = CommandId::OtaBeginUpId;

    uint8_t buffer[512];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    // Test the encoding is
    // DA 00010000                             # tag(65536)
    //    81                                   # array(1)
    //       58 20                             # bytes(32)
    //          01020304
    THEN("The encoding is successful") {
      REQUIRE(err == MessageEncoder::Status::Complete);
      std::vector<int> res(buffer, buffer+bytes_encoded);

      REQUIRE_THAT(res, Catch::Matchers::Equals(std::vector<int>{
        0xda, 0x00, 0x01, 0x00, 0x00, 0x81, 0x58, 0x20,
        0x01, 0x02, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
      }));
    }
  }

  WHEN("Encode the OtaBeginUp message, but the buffer is not big enough to accommodate the encode the array open")
  {
    OtaBeginUp command;
    uint8_t sha[SHA256_SIZE] = {0x01, 0x02, 0x03, 0x04};
    memcpy(command.params.sha, sha, SHA256_SIZE);

    command.c.id = CommandId::OtaBeginUpId;

    uint8_t buffer[5];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    // Test the encoding is
    // DA 00010000                             # tag(65536)
    //    81                                   # array(1)
    //       58 20                             # bytes(32)
    //          01020304
    REQUIRE(err == MessageEncoder::Status::Error);
  }

  WHEN("Encode the OtaBeginUp message, but the buffer is not big enough to accommodate the sha")
  {
    OtaBeginUp command;
    uint8_t sha[SHA256_SIZE] = {0x01, 0x02, 0x03, 0x04};
    memcpy(command.params.sha, sha, SHA256_SIZE);

    command.c.id = CommandId::OtaBeginUpId;

    uint8_t buffer[6];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    // Test the encoding is
    // DA 00010000                             # tag(65536)
    //    81                                   # array(1)
    //       58 20                             # bytes(32)
    //          01020304
    REQUIRE(err == MessageEncoder::Status::Error);
  }

  WHEN("Encode the ThingBeginCmd message")
  {
    ThingBeginCmd command;
    String thing_id = "thing_id";
    strcpy(command.params.thing_id, thing_id.c_str());

    command.c.id = CommandId::ThingBeginCmdId;

    uint8_t buffer[512];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    // Test the encoding is
    // DA 00010300               # tag(66304)
    //    81                     # array(1)
    //       68                  # text(8)
    //          7468696E675F6964 # "thing_id"

    THEN("The encoding is successful") {
      REQUIRE(err == MessageEncoder::Status::Complete);
      std::vector<int> res(buffer, buffer+bytes_encoded);

      REQUIRE_THAT(res, Catch::Matchers::Equals(std::vector<int>{
        0xda, 0x00, 0x01, 0x03, 0x00, 0x81, 0x68, 0x74,
        0x68, 0x69, 0x6e, 0x67, 0x5f, 0x69, 0x64
      }));
    }
  }

  WHEN("Encode the ThingBeginCmd message, but the buffer is not big enough to accommodate the array open")
  {
    ThingBeginCmd command;
    String thing_id = "thing_id";
    strcpy(command.params.thing_id, thing_id.c_str());

    command.c.id = CommandId::ThingBeginCmdId;

    uint8_t buffer[5];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    // Test the encoding is
    // DA 00010300               # tag(66304)
    //    81                     # array(1)
    //       68                  # text(8)
    //          7468696E675F6964 # "thing_id"
    REQUIRE(err == MessageEncoder::Status::Error);
  }

  WHEN("Encode the ThingBeginCmd message, but the buffer is not big enough to accommodate the thing id")
  {
    ThingBeginCmd command;
    String thing_id = "thing_id";
    strcpy(command.params.thing_id, thing_id.c_str());

    command.c.id = CommandId::ThingBeginCmdId;

    uint8_t buffer[6];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    // Test the encoding is
    // DA 00010300               # tag(66304)
    //    81                     # array(1)
    //       68                  # text(8)
    //          7468696E675F6964 # "thing_id"
    REQUIRE(err == MessageEncoder::Status::Error);
  }

  WHEN("Encode the LastValuesBeginCmd message")
  {
    LastValuesBeginCmd command;
    command.c.id = CommandId::LastValuesBeginCmdId;

    uint8_t buffer[512];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    // Test the encoding is
    // DA 00010500 # tag(66816)
    //    80       # array(0)
    THEN("The encoding is successful") {
      REQUIRE(err == MessageEncoder::Status::Complete);
      std::vector<int> res(buffer, buffer+bytes_encoded);

      REQUIRE_THAT(res, Catch::Matchers::Equals(std::vector<int>{
        0xda, 0x00, 0x01, 0x05, 0x00, 0x80
      }));
    }
  }

  WHEN("Encode the LastValuesBeginCmd message, but the buffer is not big enough to accommodate the array open")
  {
    LastValuesBeginCmd command;
    command.c.id = CommandId::LastValuesBeginCmdId;

    uint8_t buffer[5];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    // Test the encoding is
    // DA 00010500 # tag(66816)
    //    80       # array(0)
    REQUIRE(err == MessageEncoder::Status::Error);
  }

  WHEN("Encode the DeviceBeginCmd message")
  {
    DeviceBeginCmd command;
    String lib_version = "2.0.0";
    strcpy(command.params.lib_version, lib_version.c_str());

    command.c.id = CommandId::DeviceBeginCmdId;

    uint8_t buffer[512];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    // Test the encoding is
    // DA 00010700         # tag(67328)
    //    81               # array(1)
    //       65            # text(5)
    //          322E302E30 # "2.0.0"
    THEN("The encoding is successful") {
      REQUIRE(err == MessageEncoder::Status::Complete);
      std::vector<int> res(buffer, buffer+bytes_encoded);

      REQUIRE_THAT(res, Catch::Matchers::Equals(std::vector<int>{
        0xda, 0x00, 0x01, 0x07, 0x00, 0x81, 0x65, 0x32,
        0x2e, 0x30, 0x2e, 0x30
      }));
    }
  }

  WHEN("Encode the DeviceBeginCmd message, but the buffer is not big enough to accommodate the array open")
  {
    DeviceBeginCmd command;
    String lib_version = "2.0.0";
    strcpy(command.params.lib_version, lib_version.c_str());

    command.c.id = CommandId::DeviceBeginCmdId;

    uint8_t buffer[5];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    // Test the encoding is
    // DA 00010700         # tag(67328)
    //    81               # array(1)
    //       65            # text(5)
    //          322E302E30 # "2.0.0"
    REQUIRE(err == MessageEncoder::Status::Error);
  }

  WHEN("Encode the DeviceBeginCmd message, but the buffer is not big enough to accommodate the lib version")
  {
    DeviceBeginCmd command;
    String lib_version = "2.0.0";
    strcpy(command.params.lib_version, lib_version.c_str());

    command.c.id = CommandId::DeviceBeginCmdId;

    uint8_t buffer[6];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    // Test the encoding is
    // DA 00010700         # tag(67328)
    //    81               # array(1)
    //       65            # text(5)
    //          322E302E30 # "2.0.0"
    REQUIRE(err == MessageEncoder::Status::Error);
  }

  WHEN("Encode the OtaProgressCmdUp message")
  {
    OtaProgressCmdUp command;
    command.params.time = 2;

    uint8_t id[ID_SIZE] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf};
    memcpy(command.params.id, id, ID_SIZE);
    command.params.state = 1;
    command.params.state_data = -1;
    command.params.time = 100;

    command.c.id = CommandId::OtaProgressCmdUpId;

    uint8_t buffer[512];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    // Test the encoding is
    // DA 00010200                             # tag(66048)
    //    84                                   # array(4)
    //       50                                # bytes(16)
    //          000102030405060708090A0B0C0D0E0F # "\u0000\u0001\u0002\u0003\u0004\u0005\u0006\u0007\b\t\n\u000b\f\r\u000e\u000f"
    //       E1                                # primitive(1)
    //       20                                # negative(0)
    //       18 64                             # unsigned(100)
    THEN("The encoding is successful") {
      REQUIRE(err == MessageEncoder::Status::Complete);
      std::vector<int> res(buffer, buffer+bytes_encoded);

      REQUIRE_THAT(res, Catch::Matchers::Equals(std::vector<int>{
        0xda, 0x00, 0x01, 0x02, 0x00, 0x84, 0x50, 0x00,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0xe1,
        0x20, 0x18, 0x64
      }));
    }
  }

  WHEN("Encode the OtaProgressCmdUp message, but the buffer is not big enough to accommodate the array open")
  {
    OtaProgressCmdUp command;
    command.params.time = 2;

    uint8_t id[ID_SIZE] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf};
    memcpy(command.params.id, id, ID_SIZE);
    command.params.state = 1;
    command.params.state_data = -1;
    command.params.time = 100;

    command.c.id = CommandId::OtaProgressCmdUpId;

    uint8_t buffer[5];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    // Test the encoding is
    // DA 00010200                             # tag(66048)
    //    84                                   # array(4)
    //       50                                # bytes(16)
    //          000102030405060708090A0B0C0D0E0F # "\u0000\u0001\u0002\u0003\u0004\u0005\u0006\u0007\b\t\n\u000b\f\r\u000e\u000f"
    //       E1                                # primitive(1)
    //       20                                # negative(0)
    //       18 64                             # unsigned(100)
    REQUIRE(err == MessageEncoder::Status::Error);
  }

  WHEN("Encode the OtaProgressCmdUp message, but the buffer is not big enough to accommodate the id")
  {
    OtaProgressCmdUp command;
    command.params.time = 2;

    uint8_t id[ID_SIZE] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf};
    memcpy(command.params.id, id, ID_SIZE);
    command.params.state = 1;
    command.params.state_data = -1;
    command.params.time = 100;

    command.c.id = CommandId::OtaProgressCmdUpId;

    uint8_t buffer[6];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    // Test the encoding is
    // DA 00010200                             # tag(66048)
    //    84                                   # array(4)
    //       50                                # bytes(16)
    //          000102030405060708090A0B0C0D0E0F # "\u0000\u0001\u0002\u0003\u0004\u0005\u0006\u0007\b\t\n\u000b\f\r\u000e\u000f"
    //       E1                                # primitive(1)
    //       20                                # negative(0)
    //       18 64                             # unsigned(100)
    REQUIRE(err == MessageEncoder::Status::Error);
  }

  WHEN("Encode the OtaProgressCmdUp message, but the buffer is not big enough to accommodate the state")
  {
    OtaProgressCmdUp command;
    command.params.time = 2;

    uint8_t id[ID_SIZE] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf};
    memcpy(command.params.id, id, ID_SIZE);
    command.params.state = 1;
    command.params.state_data = -1;
    command.params.time = 100;

    command.c.id = CommandId::OtaProgressCmdUpId;

    uint8_t buffer[23];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    // Test the encoding is
    // DA 00010200                             # tag(66048)
    //    84                                   # array(4)
    //       50                                # bytes(16)
    //          000102030405060708090A0B0C0D0E0F # "\u0000\u0001\u0002\u0003\u0004\u0005\u0006\u0007\b\t\n\u000b\f\r\u000e\u000f"
    //       E1                                # primitive(1)
    //       20                                # negative(0)
    //       18 64                             # unsigned(100)
    REQUIRE(err == MessageEncoder::Status::Error);
  }

  WHEN("Encode the OtaProgressCmdUp message, but the buffer is not big enough to accommodate the state_data")
  {
    OtaProgressCmdUp command;
    command.params.time = 2;

    uint8_t id[ID_SIZE] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf};
    memcpy(command.params.id, id, ID_SIZE);
    command.params.state = 1;
    command.params.state_data = -1;
    command.params.time = 100;

    command.c.id = CommandId::OtaProgressCmdUpId;

    uint8_t buffer[24];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    // Test the encoding is
    // DA 00010200                             # tag(66048)
    //    84                                   # array(4)
    //       50                                # bytes(16)
    //          000102030405060708090A0B0C0D0E0F # "\u0000\u0001\u0002\u0003\u0004\u0005\u0006\u0007\b\t\n\u000b\f\r\u000e\u000f"
    //       E1                                # primitive(1)
    //       20                                # negative(0)
    //       18 64                             # unsigned(100)
    REQUIRE(err == MessageEncoder::Status::Error);
  }

  WHEN("Encode the OtaProgressCmdUp message, but the buffer is not big enough to accommodate the time")
  {
    OtaProgressCmdUp command;
    command.params.time = 2;

    uint8_t id[ID_SIZE] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf};
    memcpy(command.params.id, id, ID_SIZE);
    command.params.state = 1;
    command.params.state_data = -1;
    command.params.time = 100;

    command.c.id = CommandId::OtaProgressCmdUpId;

    uint8_t buffer[25];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    // Test the encoding is
    // DA 00010200                             # tag(66048)
    //    84                                   # array(4)
    //       50                                # bytes(16)
    //          000102030405060708090A0B0C0D0E0F # "\u0000\u0001\u0002\u0003\u0004\u0005\u0006\u0007\b\t\n\u000b\f\r\u000e\u000f"
    //       E1                                # primitive(1)
    //       20                                # negative(0)
    //       18 64                             # unsigned(100)
    REQUIRE(err == MessageEncoder::Status::Error);
  }

  WHEN("Encode the TimezoneCommandUp message")
  {
    TimezoneCommandUp command;
    command.c.id = CommandId::TimezoneCommandUpId;

    uint8_t buffer[512];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    // Test the encoding is
    // DA 00010800 # tag(67584)
    //    80       # array(0)
    THEN("The encoding is successful") {
      REQUIRE(err == MessageEncoder::Status::Complete);
      std::vector<int> res(buffer, buffer+bytes_encoded);

      REQUIRE_THAT(res, Catch::Matchers::Equals(std::vector<int>{
        0xda, 0x00, 0x01, 0x08, 0x00, 0x80
      }));
    }
  }

  WHEN("Encode the TimezoneCommandUp message, but the buffer is not big enough to accommodate the array open")
  {
    TimezoneCommandUp command;
    command.c.id = CommandId::TimezoneCommandUpId;

    uint8_t buffer[5];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    // Test the encoding is
    // DA 00010800 # tag(67584)
    //    80       # array(0)
    REQUIRE(err == MessageEncoder::Status::Error);
  }

  WHEN("Encode the ThingUpdateCmdId message")
  {
    ThingUpdateCmd command;
    command.c.id = CommandId::ThingUpdateCmdId;

    String thing_id = "e4494d55-872a-4fd2-9646-92f87949394c";
    strcpy(command.params.thing_id, thing_id.c_str());

    uint8_t buffer[512];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    THEN("The encoding is unsuccessful - ThingUpdateCmdId is not supported") {
      REQUIRE(err == MessageEncoder::Status::Error);
    }
  }

  WHEN("Encode the TimezoneCommandDown message")
  {
    TimezoneCommandDown command;
    command.c.id = CommandId::TimezoneCommandDownId;

    command.params.offset = 1708963873;
    command.params.until = 2024579473;

    uint8_t buffer[512];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    THEN("The encoding is unsuccessful - SetTimezoneCommand is not supported") {
      REQUIRE(err == MessageEncoder::Status::Error);
    }
  }

  WHEN("Encode the LastValuesUpdateCmd message")
  {
    LastValuesUpdateCmd command;
    command.c.id = CommandId::LastValuesUpdateCmdId;

    command.params.length = 13;
    uint8_t last_values[13] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                               0x08, 0x09, 0x10, 0x11, 0x12};
    command.params.last_values = last_values;

    uint8_t buffer[512];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    THEN("The encoding is unsuccessful - LastValuesUpdateCmd is not supported") {
      REQUIRE(err == MessageEncoder::Status::Error);
    }
  }

  WHEN("Encode the OtaUpdateCmdDown message")
  {
    OtaUpdateCmdDown command;
    command.c.id = CommandId::OtaUpdateCmdDownId;

    uint8_t buffer[512];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    THEN("The encoding is unsuccessful - OtaUpdateCmdDown is not supported") {
      REQUIRE(err == MessageEncoder::Status::Error);
    }
  }

  WHEN("Encode the ThingDetachCmd message")
  {
    ThingDetachCmd command;
    command.c.id = CommandId::ThingDetachCmdId;

    uint8_t buffer[512];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    THEN("The encoding is unsuccessful - ThingDetachCmd is not supported") {
      REQUIRE(err == MessageEncoder::Status::Error);
    }
  }

  WHEN("Encode a message with unknown command Id")
  {
    OtaUpdateCmdDown command;
    command.c.id = CommandId::UnknownCmdId;

    uint8_t buffer[512];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    THEN("The encoding is unsuccessful - UnknownCmdId is not supported") {
      REQUIRE(err == MessageEncoder::Status::Error);
    }
  }

  WHEN("Encode the DeviceNetConfigCmdUp message with WiFi")
  {
    DeviceNetConfigCmdUp command;
    command.c.id = CommandId::DeviceNetConfigCmdUpId;

    command.params.type = NetworkAdapter::WIFI;
    String ssid = "SSID";
    strcpy(command.params.wifi.ssid, ssid.c_str());
    uint8_t buffer[512];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    uint8_t expected_result[] = {
    0xda, 0x00, 0x01, 0x11, 0x00, 0x82, 0x01, 0x64, 0x53, 0x53, 0x49, 0x44
    };

    // Test the encoding is
    // DA 00011100      # tag(73728)
    //    82            # array(2)
    //      01          # unsigned(1)
    //      64          # text(4)
    //         53534944 # "SSID"
    THEN("The encoding is successful") {
        REQUIRE(err == MessageEncoder::Status::Complete);
        REQUIRE(bytes_encoded == sizeof(expected_result));
        REQUIRE(memcmp(buffer, expected_result, sizeof(expected_result)) == 0);
    }
  }

  WHEN("Encode the DeviceNetConfigCmdUp message with Wifi and buffer without enough space")
  {
    DeviceNetConfigCmdUp command;
    command.c.id = CommandId::DeviceNetConfigCmdUpId;

    command.params.type = NetworkAdapter::WIFI;
    String longSsid = "longSSID";
    strcpy(command.params.wifi.ssid, longSsid.c_str());

    uint8_t buffer[7];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    THEN("The encoding fails") {
        REQUIRE(err == MessageEncoder::Status::Error);
    }
  }

  WHEN("Encode the DeviceNetConfigCmdUp message with LoraWan")
  {
    DeviceNetConfigCmdUp command;
    command.c.id = CommandId::DeviceNetConfigCmdUpId;

    command.params.type = NetworkAdapter::LORA;

    String app_eui = "APPEUI";
    strcpy(command.params.lora.appeui, app_eui.c_str());
    String app_key = "APPKEY";
    strcpy(command.params.lora.appkey, app_key.c_str());
    uint8_t buffer[512];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    uint8_t expected_result[] = {
      0xda, 0x00, 0x01, 0x11, 0x00, 0x82,
      0x02, 0x66, 0x41, 0x50, 0x50, 0x45, 0x55, 0x49
    };

    // Test the encoding is
    // DA 00011100          # tag(73728)
    //   82                 # array(2)
    //      02              # unsigned(2)
    //      66              # text(6)
    //         415050455549 # "APPEUI"
    THEN("The encoding is successful") {
        REQUIRE(err == MessageEncoder::Status::Complete);
        REQUIRE(bytes_encoded == sizeof(expected_result));
        REQUIRE(memcmp(buffer, expected_result, sizeof(expected_result)) == 0);
    }
  }

  WHEN("Encode the DeviceNetConfigCmdUp message with LoRa buffer without enough space")
  {
    DeviceNetConfigCmdUp command;
    command.c.id = CommandId::DeviceNetConfigCmdUpId;

    command.params.type = NetworkAdapter::LORA;

    String app_eui = "APPEUI";
    strcpy(command.params.lora.appeui, app_eui.c_str());
    String app_key = "APPKEY";
    strcpy(command.params.lora.appkey, app_key.c_str());
    uint8_t buffer[7];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    THEN("The encoding fails") {
        REQUIRE(err == MessageEncoder::Status::Error);
    }
  }

  WHEN("Encode the DeviceNetConfigCmdUp message with GSM")
  {
    DeviceNetConfigCmdUp command;
    command.c.id = CommandId::DeviceNetConfigCmdUpId;

    command.params.type = NetworkAdapter::GSM;
    String apn = "apn.arduino.cc";
    strcpy(command.params.gsm.apn, apn.c_str());
    String user = "username";
    strcpy(command.params.gsm.login, user.c_str());
    String password = "PASSWORD";
    strcpy(command.params.gsm.pass, password.c_str());
    uint8_t buffer[512];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    uint8_t expected_result[] = {
      0xda, 0x00, 0x01, 0x11, 0x00, 0x83,
      0x03, 0x6e, 0x61, 0x70, 0x6e, 0x2e, 0x61, 0x72,
      0x64, 0x75, 0x69, 0x6e, 0x6f, 0x2e, 0x63, 0x63,
      0x68, 0x75, 0x73, 0x65, 0x72, 0x6e, 0x61, 0x6d,
      0x65
    };

    // Test the encoding is
    // DA 00011100                          # tag(73728)
    //   83                                 # array(3)
    //      03                              # unsigned(3)
    //      6E                              # text(14)
    //         61706E2E61726475696E6F2E6363 # "apn.arduino.cc"
    //      68                              # text(8)
    //         757365726E616D65             # "username"

    THEN("The encoding is successful") {
        REQUIRE(err == MessageEncoder::Status::Complete);
        REQUIRE(bytes_encoded == sizeof(expected_result));
        REQUIRE(memcmp(buffer, expected_result, sizeof(expected_result)) == 0);
    }
  }

  WHEN("Encode the DeviceNetConfigCmdUp message with GSM buffer without enough space for login")
  {
    DeviceNetConfigCmdUp command;
    command.c.id = CommandId::DeviceNetConfigCmdUpId;

    command.params.type = NetworkAdapter::GSM;
    String apn = "apn.arduino.cc";
    strcpy(command.params.nb.apn, apn.c_str());
    String user = "username";
    strcpy(command.params.nb.login, user.c_str());
    String password = "PASSWORD";
    strcpy(command.params.nb.pass, password.c_str());
    uint8_t buffer[25];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    THEN("The encoding fails") {
        REQUIRE(err == MessageEncoder::Status::Error);
    }
  }

  WHEN("Encode the DeviceNetConfigCmdUp message with NB-IoT")
  {
    DeviceNetConfigCmdUp command;
    command.c.id = CommandId::DeviceNetConfigCmdUpId;

    command.params.type = NetworkAdapter::NB;
    String apn = "apn.arduino.cc";
    strcpy(command.params.gsm.apn, apn.c_str());
    String user = "username";
    strcpy(command.params.gsm.login, user.c_str());
    String password = "PASSWORD";
    strcpy(command.params.gsm.pass, password.c_str());
    uint8_t buffer[512];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    uint8_t expected_result[] = {
      0xda, 0x00, 0x01, 0x11, 0x00, 0x83,
      0x04, 0x6e, 0x61, 0x70, 0x6e, 0x2e, 0x61, 0x72,
      0x64, 0x75, 0x69, 0x6e, 0x6f, 0x2e, 0x63, 0x63,
      0x68, 0x75, 0x73, 0x65, 0x72, 0x6e, 0x61, 0x6d,
      0x65
    };

    // Test the encoding is
    // DA 00011100                          # tag(73728)
    //   83                                 # array(3)
    //      03                              # unsigned(3)
    //      6E                              # text(14)
    //         61706E2E61726475696E6F2E6363 # "apn.arduino.cc"
    //      68                              # text(8)
    //         757365726E616D65             # "username"

    THEN("The encoding is successful") {
        REQUIRE(err == MessageEncoder::Status::Complete);
        REQUIRE(bytes_encoded == sizeof(expected_result));
        REQUIRE(memcmp(buffer, expected_result, sizeof(expected_result)) == 0);
    }
  }

  WHEN("Encode the DeviceNetConfigCmdUp message with NB-IoT buffer without enough space")
  {
    DeviceNetConfigCmdUp command;
    command.c.id = CommandId::DeviceNetConfigCmdUpId;

    command.params.type = NetworkAdapter::NB;
    String apn = "apn.arduino.cc";
    strcpy(command.params.nb.apn, apn.c_str());
    String user = "username";
    strcpy(command.params.nb.login, user.c_str());
    String password = "PASSWORD";
    strcpy(command.params.nb.pass, password.c_str());
    uint8_t buffer[12];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    THEN("The encoding fails") {
        REQUIRE(err == MessageEncoder::Status::Error);
    }
  }

  WHEN("Encode the DeviceNetConfigCmdUp message with CAT-M1")
  {
    DeviceNetConfigCmdUp command;
    command.c.id = CommandId::DeviceNetConfigCmdUpId;

    command.params.type = NetworkAdapter::CATM1;
    String apn = "apn.arduino.cc";
    strcpy(command.params.catm1.apn, apn.c_str());
    String user = "username";
    strcpy(command.params.catm1.login, user.c_str());
    String password = "PASSWORD";
    strcpy(command.params.catm1.pass, password.c_str());
    uint8_t buffer[512];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    uint8_t expected_result[] = {
      0xda, 0x00, 0x01, 0x11, 0x00, 0x83,
      0x05, 0x6e, 0x61, 0x70, 0x6e, 0x2e, 0x61, 0x72,
      0x64, 0x75, 0x69, 0x6e, 0x6f, 0x2e, 0x63, 0x63,
      0x68, 0x75, 0x73, 0x65, 0x72, 0x6e, 0x61, 0x6d,
      0x65
    };

    // Test the encoding is
    // DA 00011100                          # tag(73728)
    //   83                                 # array(3)
    //      05                              # unsigned(5)
    //      6E                              # text(14)
    //         61706E2E61726475696E6F2E6363 # "apn.arduino.cc"
    //      68                              # text(8)
    //         757365726E616D65             # "username"

    THEN("The encoding is successful") {
        REQUIRE(err == MessageEncoder::Status::Complete);
        REQUIRE(bytes_encoded == sizeof(expected_result));
        REQUIRE(memcmp(buffer, expected_result, sizeof(expected_result)) == 0);
    }
  }

  WHEN("Encode the DeviceNetConfigCmdUp message with CAT-M1 buffer without enough space")
  {
    DeviceNetConfigCmdUp command;
    command.c.id = CommandId::DeviceNetConfigCmdUpId;

    command.params.type = NetworkAdapter::CATM1;
    String apn = "apn.arduino.cc";
    strcpy(command.params.nb.apn, apn.c_str());
    String user = "username";
    strcpy(command.params.nb.login, user.c_str());
    String password = "PASSWORD";
    strcpy(command.params.nb.pass, password.c_str());
    uint8_t buffer[12];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    THEN("The encoding fails") {
        REQUIRE(err == MessageEncoder::Status::Error);
    }
  }

  WHEN("Encode the DeviceNetConfigCmdUp message with CAT-M1 buffer without enough space for login")
  {
    DeviceNetConfigCmdUp command;
    command.c.id = CommandId::DeviceNetConfigCmdUpId;

    command.params.type = NetworkAdapter::CATM1;
    String apn = "apn.arduino.cc";
    strcpy(command.params.nb.apn, apn.c_str());
    String user = "username";
    strcpy(command.params.nb.login, user.c_str());
    String password = "PASSWORD";
    strcpy(command.params.nb.pass, password.c_str());
    uint8_t buffer[25];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    THEN("The encoding fails") {
        REQUIRE(err == MessageEncoder::Status::Error);
    }
  }

  WHEN("Encode the DeviceNetConfigCmdUp message with Ethernet")
  {
    DeviceNetConfigCmdUp command;
    command.c.id = CommandId::DeviceNetConfigCmdUpId;

    command.params.type = NetworkAdapter::ETHERNET;
    uint8_t ip [4] = {192, 168, 0, 2};
    command.params.eth.ip.type = IPType::IPv4;
    memcpy(command.params.eth.ip.bytes, ip, sizeof(ip));
    uint8_t dns[4] = {8, 8, 8, 8};
    command.params.eth.dns.type = IPType::IPv4;
    memcpy(command.params.eth.dns.bytes, dns, sizeof(dns));
    uint8_t gateway [4] = {192, 168, 1, 1};
    command.params.eth.gateway.type = IPType::IPv4;
    memcpy(command.params.eth.gateway.bytes, gateway, sizeof(gateway));
    uint8_t netmask [4] = {255, 255, 255, 0};
    command.params.eth.netmask.type = IPType::IPv4;
    memcpy(command.params.eth.netmask.bytes, netmask, sizeof(netmask));
    uint8_t buffer[512];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    uint8_t expected_result[] = {
      0xda, 0x00, 0x01, 0x11, 0x00, 0x85,
      0x06, 0x44, 0xc0, 0xa8, 0x00, 0x02,
      0x44, 0x08, 0x08, 0x08, 0x08,
      0x44, 0xc0, 0xa8, 0x01, 0x01,
      0x44, 0xff, 0xff, 0xff, 0x00
    };

    // Test the encoding is
    // DA 00011100         # tag(73728)
    //   85               # array(5)
    //      06            # unsigned(6)
    //      44            # bytes(4)
    //         C0A80002   # "\xC0\xA8\u0000\u0002"
    //      44            # bytes(4)
    //         08080808   # "\b\b\b\b"
    //      44            # bytes(4)
    //         C0A80101   # "\xC0\xA8\u0001\u0001"
    //      44            # bytes(4)
    //         FFFFFF00   # "\xFF\xFF\xFF\u0000"

    THEN("The encoding is successful") {
        REQUIRE(err == MessageEncoder::Status::Complete);
        REQUIRE(bytes_encoded == sizeof(expected_result));
        REQUIRE(memcmp(buffer, expected_result, sizeof(expected_result)) == 0);
    }
  }

  WHEN("Encode the DeviceNetConfigCmdUp message with Ethernet IPv6")
  {
    DeviceNetConfigCmdUp command;
    command.c.id = CommandId::DeviceNetConfigCmdUpId;

    command.params.type = NetworkAdapter::ETHERNET;
    uint8_t ip[] = {0x1a, 0x4f, 0xa7, 0xa9, 0x92, 0x8f, 0x7b, 0x1c, 0xec, 0x3b, 0x1e, 0xcd, 0x88, 0x58, 0x0d, 0x1e};
    command.params.eth.ip.type = IPType::IPv6;
    memcpy(command.params.eth.ip.bytes, ip, sizeof(ip));
    uint8_t dns[] = {0x21, 0xf6, 0x3b, 0x22, 0x99, 0x6f, 0x5b, 0x72, 0x25, 0xd9, 0xe0, 0x24, 0xf0, 0x36, 0xb5, 0xd2};
    command.params.eth.dns.type = IPType::IPv6;
    memcpy(command.params.eth.dns.bytes, dns, sizeof(dns));
    uint8_t gateway[] = {0x2e, 0xc2, 0x27, 0xf1, 0xf1, 0x9a, 0x0c, 0x11, 0x47, 0x1b, 0x84, 0xaf, 0x96, 0x10, 0xb0, 0x17};
    command.params.eth.gateway.type = IPType::IPv6;
    memcpy(command.params.eth.gateway.bytes, gateway, sizeof(gateway));
    uint8_t netmask[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    command.params.eth.netmask.type = IPType::IPv6;
    memcpy(command.params.eth.netmask.bytes, netmask, sizeof(netmask));


    uint8_t buffer[512];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    uint8_t expected_result[] = {
      0xda, 0x00, 0x01, 0x11, 0x00, 0x85,
      0x06,0x50, 0x1A, 0x4F, 0xA7, 0xA9, 0x92, 0x8F, 0x7B, 0x1C,
      0xEC, 0x3B, 0x1E, 0xCD, 0x88, 0x58, 0x0D, 0x1E,
      0x50, 0x21, 0xF6, 0x3B, 0x22, 0x99, 0x6F,
      0x5B, 0x72, 0x25, 0xD9, 0xE0, 0x24, 0xF0, 0x36,
      0xB5, 0xD2, 0x50, 0x2E, 0xC2, 0x27, 0xF1,
      0xF1, 0x9A, 0x0C, 0x11, 0x47, 0x1B, 0x84, 0xAF,
      0x96, 0x10, 0xB0, 0x17, 0x50, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    // Test the encoding is
    // DA 00011100                              # tag(73728)
    //   85                                     # array(5)
    //      06                                  # unsigned(6)
    //      50                                  # bytes(16)
    //         1A4FA7A9928F7B1CEC3B1ECD88580D1E # "\u001AO\xA7\xA9\x92\x8F{\u001C\xEC;\u001E͈X\r\u001E"
    //      50                                  # bytes(16)
    //         21F63B22996F5B7225D9E024F036B5D2 # "!\xF6;\"\x99o[r%\xD9\xE0$\xF06\xB5\xD2"
    //      50                                  # bytes(16)
    //         2EC227F1F19A0C11471B84AF9610B017 # ".\xC2'\xF1\xF1\x9A\f\u0011G\e\x84\xAF\x96\u0010\xB0\u0017"
    //      50                                  # bytes(16)
    //         FFFFFFFFFFFFFFFF0000000000000000 # "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\u0000\u0000\u0000\u0000\u0000\u0000\u0000\u0000"

    THEN("The encoding is successful") {
        REQUIRE(err == MessageEncoder::Status::Complete);
        REQUIRE(bytes_encoded == sizeof(expected_result));
        REQUIRE(memcmp(buffer, expected_result, sizeof(expected_result)) == 0);
    }
  }

  WHEN("Encode the DeviceNetConfigCmdUp message with Ethernet DHCP")
  {
    DeviceNetConfigCmdUp command;
    command.c.id = CommandId::DeviceNetConfigCmdUpId;

    command.params.type = NetworkAdapter::ETHERNET;

    memset(command.params.eth.ip.bytes, 0, sizeof(command.params.eth.ip.bytes));
    memset(command.params.eth.dns.bytes, 0, sizeof(command.params.eth.dns.bytes));
    memset(command.params.eth.gateway.bytes, 0, sizeof(command.params.eth.gateway.bytes));
    memset(command.params.eth.netmask.bytes, 0, sizeof(command.params.eth.netmask.bytes));
    uint8_t buffer[512];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    uint8_t expected_result[] = {
      0xda, 0x00, 0x01, 0x11, 0x00, 0x85,
      0x06, 0x40, 0x40, 0x40, 0x40,
    };

    // Test the encoding is
    // DA 00011100 # tag(73737)
    //    85      # array(5)
    //      06    # unsigned(6)
    //      40    # bytes(0)
    //            # ""
    //      40    # bytes(0)
    //            # ""
    //      40    # bytes(0)
    //            # ""
    //      40    # bytes(0)
    //            # ""

    THEN("The encoding is successful") {
        REQUIRE(err == MessageEncoder::Status::Complete);
        REQUIRE(bytes_encoded == sizeof(expected_result));
        REQUIRE(memcmp(buffer, expected_result, sizeof(expected_result)) == 0);
    }
  }

  WHEN("Encode the DeviceNetConfigCmdUp message with Ethernet IPv6 not enough space")
  {
    DeviceNetConfigCmdUp command;
    command.c.id = CommandId::DeviceNetConfigCmdUpId;

    command.params.type = NetworkAdapter::ETHERNET;
    uint8_t ip[] = {0x1a, 0x4f, 0xa7, 0xa9, 0x92, 0x8f, 0x7b, 0x1c, 0xec, 0x3b, 0x1e, 0xcd, 0x88, 0x58, 0x0d, 0x1e};
    command.params.eth.ip.type = IPType::IPv6;
    memcpy(command.params.eth.ip.bytes, ip, sizeof(ip));
    uint8_t dns[] = {0x21, 0xf6, 0x3b, 0x22, 0x99, 0x6f, 0x5b, 0x72, 0x25, 0xd9, 0xe0, 0x24, 0xf0, 0x36, 0xb5, 0xd2};
    command.params.eth.dns.type = IPType::IPv6;
    memcpy(command.params.eth.dns.bytes, dns, sizeof(dns));
    uint8_t gateway[] = {0x2e, 0xc2, 0x27, 0xf1, 0xf1, 0x9a, 0x0c, 0x11, 0x47, 0x1b, 0x84, 0xaf, 0x96, 0x10, 0xb0, 0x17};
    command.params.eth.gateway.type = IPType::IPv6;
    memcpy(command.params.eth.gateway.bytes, gateway, sizeof(gateway));
    uint8_t netmask[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    command.params.eth.netmask.type = IPType::IPv6;
    memcpy(command.params.eth.netmask.bytes, netmask, sizeof(netmask));


    uint8_t buffer[35];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    THEN("The encoding fails") {
        REQUIRE(err == MessageEncoder::Status::Error);
    }
  }

  WHEN("Encode the DeviceNetConfigCmdUp message with Ethernet IPv4 not enough space")
  {
    DeviceNetConfigCmdUp command;
    command.c.id = CommandId::DeviceNetConfigCmdUpId;

    command.params.type = NetworkAdapter::ETHERNET;
    uint8_t ip [4] = {192, 168, 0, 2};
    command.params.eth.ip.type = IPType::IPv4;
    memcpy(command.params.eth.ip.bytes, ip, sizeof(ip));
    uint8_t dns[4] = {8, 8, 8, 8};
    command.params.eth.dns.type = IPType::IPv4;
    memcpy(command.params.eth.dns.bytes, dns, sizeof(dns));
    uint8_t gateway [4] = {192, 168, 1, 1};
    command.params.eth.gateway.type = IPType::IPv4;
    memcpy(command.params.eth.gateway.bytes, gateway, sizeof(gateway));
    uint8_t netmask [4] = {255, 255, 255, 0};
    command.params.eth.netmask.type = IPType::IPv4;
    memcpy(command.params.eth.netmask.bytes, netmask, sizeof(netmask));
    uint8_t buffer[20];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    THEN("The encoding fails") {
      REQUIRE(err == MessageEncoder::Status::Error);
    }
  }

  WHEN("Encode the DeviceNetConfigCmdUp message with Ethernet IPv4 not enough space for netmask")
  {
    DeviceNetConfigCmdUp command;
    command.c.id = CommandId::DeviceNetConfigCmdUpId;

    command.params.type = NetworkAdapter::ETHERNET;
    uint8_t ip [4] = {192, 168, 0, 2};
    command.params.eth.ip.type = IPType::IPv4;
    memcpy(command.params.eth.ip.bytes, ip, sizeof(ip));
    uint8_t dns[4] = {8, 8, 8, 8};
    command.params.eth.dns.type = IPType::IPv4;
    memcpy(command.params.eth.dns.bytes, dns, sizeof(dns));
    uint8_t gateway [4] = {192, 168, 1, 1};
    command.params.eth.gateway.type = IPType::IPv4;
    memcpy(command.params.eth.gateway.bytes, gateway, sizeof(gateway));
    uint8_t netmask [4] = {255, 255, 255, 0};
    command.params.eth.netmask.type = IPType::IPv4;
    memcpy(command.params.eth.netmask.bytes, netmask, sizeof(netmask));
    uint8_t buffer[25];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    THEN("The encoding fails") {
      REQUIRE(err == MessageEncoder::Status::Error);
    }
  }

  WHEN("Encode the DeviceNetConfigCmdUp message with Ethernet IPv6 not enough space for any")
  {
    DeviceNetConfigCmdUp command;
    command.c.id = CommandId::DeviceNetConfigCmdUpId;

    command.params.type = NetworkAdapter::ETHERNET;
    uint8_t ip[] = {0x1a, 0x4f, 0xa7, 0xa9, 0x92, 0x8f, 0x7b, 0x1c, 0xec, 0x3b, 0x1e, 0xcd, 0x88, 0x58, 0x0d, 0x1e};
    command.params.eth.ip.type = IPType::IPv6;
    memcpy(command.params.eth.ip.bytes, ip, sizeof(ip));
    uint8_t dns[] = {0x21, 0xf6, 0x3b, 0x22, 0x99, 0x6f, 0x5b, 0x72, 0x25, 0xd9, 0xe0, 0x24, 0xf0, 0x36, 0xb5, 0xd2};
    command.params.eth.dns.type = IPType::IPv6;
    memcpy(command.params.eth.dns.bytes, dns, sizeof(dns));
    uint8_t gateway[] = {0x2e, 0xc2, 0x27, 0xf1, 0xf1, 0x9a, 0x0c, 0x11, 0x47, 0x1b, 0x84, 0xaf, 0x96, 0x10, 0xb0, 0x17};
    command.params.eth.gateway.type = IPType::IPv6;
    memcpy(command.params.eth.gateway.bytes, gateway, sizeof(gateway));
    uint8_t netmask[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    command.params.eth.netmask.type = IPType::IPv6;
    memcpy(command.params.eth.netmask.bytes, netmask, sizeof(netmask));

    uint8_t buffer[12];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    THEN("The encoding fails") {
        REQUIRE(err == MessageEncoder::Status::Error);
    }
  }

  WHEN("Encode the DeviceNetConfigCmdUp message with Cellular")
  {
    DeviceNetConfigCmdUp command;
    command.c.id = CommandId::DeviceNetConfigCmdUpId;

    command.params.type = NetworkAdapter::CELL;
    String apn = "apn.arduino.cc";
    strcpy(command.params.cell.apn, apn.c_str());
    String user = "username";
    strcpy(command.params.cell.login, user.c_str());
    String password = "PASSWORD";
    strcpy(command.params.cell.pass, password.c_str());
    uint8_t buffer[512];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    uint8_t expected_result[] = {
      0xda, 0x00, 0x01, 0x11, 0x00, 0x83,
      0x07, 0x6e, 0x61, 0x70, 0x6e, 0x2e, 0x61, 0x72,
      0x64, 0x75, 0x69, 0x6e, 0x6f, 0x2e, 0x63, 0x63,
      0x68, 0x75, 0x73, 0x65, 0x72, 0x6e, 0x61, 0x6d,
      0x65
    };

    // Test the encoding is
    // DA 00011100                          # tag(73728)
    //   83                                 # array(3)
    //      07                              # unsigned(7)
    //      6E                              # text(14)
    //         61706E2E61726475696E6F2E6363 # "apn.arduino.cc"
    //      68                              # text(8)
    //         757365726E616D65             # "username"

    THEN("The encoding is successful") {
        REQUIRE(err == MessageEncoder::Status::Complete);
        REQUIRE(bytes_encoded == sizeof(expected_result));
        REQUIRE(memcmp(buffer, expected_result, sizeof(expected_result)) == 0);
    }
  }

  WHEN("Encode the DeviceNetConfigCmdUp message with Notecard")
  {
    DeviceNetConfigCmdUp command;
    command.c.id = CommandId::DeviceNetConfigCmdUpId;

    command.params.type = NetworkAdapter::NOTECARD;

    uint8_t buffer[512];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    uint8_t expected_result[] = {
      0xda, 0x00, 0x01, 0x11, 0x00, 0x81,
      0x08
    };

    // Test the encoding is
    // DA 00011100                          # tag(73728)
    //   81                                 # array(1)
    //      08                              # unsigned(8)

    THEN("The encoding is successful") {
        REQUIRE(err == MessageEncoder::Status::Complete);
        REQUIRE(bytes_encoded == sizeof(expected_result));
        REQUIRE(memcmp(buffer, expected_result, sizeof(expected_result)) == 0);
    }
  }

  WHEN("Encode the DeviceNetConfigCmdUp message with None")
  {
    DeviceNetConfigCmdUp command;
    command.c.id = CommandId::DeviceNetConfigCmdUpId;

    command.params.type = NetworkAdapter::NONE;

    uint8_t buffer[512];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    uint8_t expected_result[] = {
      0xda, 0x00, 0x01, 0x11, 0x00, 0x81, 0x00
    };

    // Test the encoding is
    // DA 00011100                          # tag(73728)
    //   80                                 # array(1)

    THEN("The encoding is successful") {
        REQUIRE(err == MessageEncoder::Status::Complete);
        REQUIRE(bytes_encoded == sizeof(expected_result));
        REQUIRE(memcmp(buffer, expected_result, sizeof(expected_result)) == 0);
    }
  }

  WHEN("Encode the DeviceNetConfigCmdUp message buffer without enough space")
  {
    DeviceNetConfigCmdUp command;
    command.c.id = CommandId::DeviceNetConfigCmdUpId;

    command.params.type = NetworkAdapter::ETHERNET;


    uint8_t buffer[6];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    THEN("The encoding fails") {
        REQUIRE(err == MessageEncoder::Status::Error);
    }
  }

  WHEN("Encode the DeviceNetConfigCmdUp message buffer without enough space for array")
  {
    DeviceNetConfigCmdUp command;
    command.c.id = CommandId::DeviceNetConfigCmdUpId;

    command.params.type = NetworkAdapter::ETHERNET;


    uint8_t buffer[5];
    size_t bytes_encoded = sizeof(buffer);

    CBORMessageEncoder encoder;
    MessageEncoder::Status err = encoder.encode((Message*)&command, buffer, bytes_encoded);

    THEN("The encoding fails") {
        REQUIRE(err == MessageEncoder::Status::Error);
    }
  }
}
