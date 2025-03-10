/*
   Copyright (c) 2024 Arduino.  All rights reserved.
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

/******************************************************************************
   TEST CODE
 ******************************************************************************/

SCENARIO("Test the encoding of command messages") {
  /****************************************************************************/

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

  /****************************************************************************/

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

  /****************************************************************************/

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

    /**************************************************************************/

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

  /****************************************************************************/

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

  /****************************************************************************/

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

  /****************************************************************************/

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

  /****************************************************************************/

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

  /****************************************************************************/

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

  /****************************************************************************/

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

  /****************************************************************************/

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

  /****************************************************************************/

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
}
