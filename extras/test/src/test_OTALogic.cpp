/*
 * Copyright (c) 2020 Arduino.  All rights reserved.
 */

/**************************************************************************************
   INCLUDE
 **************************************************************************************/

#include <algorithm>

#include <catch.hpp>

#include <OTATestData.h>
#include <OTAStorage_Mock.h>

#include <OTALogic.h>

/**************************************************************************************
   TEST HELPER
 **************************************************************************************/

void simulateOTABinaryReception(OTALogic & ota_logic, OTAStorage_Mock & ota_storage, OTAData const & ota_test_data)
{
  uint32_t bytes_written = 0;
  uint32_t const bytes_to_write = sizeof(uint32_t) + sizeof(uint32_t) + ota_test_data.data.len;
  for(; bytes_written < (bytes_to_write - MQTT_OTA_BUF_SIZE); bytes_written += MQTT_OTA_BUF_SIZE)
  {
    ota_logic.onOTADataReceived(ota_test_data.buf + bytes_written, MQTT_OTA_BUF_SIZE);
    ota_logic.update(&ota_storage);
  }

  if(bytes_written < bytes_to_write)
  {
    uint32_t const remaining_bytes = (bytes_to_write - bytes_written);
    ota_logic.onOTADataReceived(ota_test_data.buf + bytes_written, remaining_bytes);
    ota_logic.update(&ota_storage);
  }
}

/**************************************************************************************
   TEST CODE
 **************************************************************************************/

TEST_CASE("Valid OTA data is received ", "[OTALogic]")
{
  OTALogic ota_logic;
  OTAData valid_ota_test_data;
  OTAStorage_Mock ota_storage;

  ota_storage._init_return_val = true;
  ota_storage._open_return_val = true;

  generate_valid_ota_data(valid_ota_test_data);

  simulateOTABinaryReception(ota_logic, ota_storage, valid_ota_test_data);

  THEN("the complete binary file should have been written to the OTA storage")
  {
    REQUIRE(ota_storage._binary.size() == valid_ota_test_data.data.len);
    REQUIRE(std::equal(ota_storage._binary.begin(),
                       ota_storage._binary.end(),
                       valid_ota_test_data.data.bin));
  }

  THEN("The OTA logic should be in the 'Reset' state")
  {
    REQUIRE(ota_logic.state() == OTAState::Reset);
  }
}

TEST_CASE("Invalid OTA data is received ", "[OTALogic - CRC wrong]")
{
  OTALogic ota_logic;
  OTAData invalid_valid_ota_test_data_crc_wrong;
  OTAStorage_Mock ota_storage;

  ota_storage._init_return_val = true;
  ota_storage._open_return_val = true;

  generate_invalid_ota_data_crc_wrong(invalid_valid_ota_test_data_crc_wrong);

  simulateOTABinaryReception(ota_logic, ota_storage, invalid_valid_ota_test_data_crc_wrong);

  THEN("there should be no binary file be stored on the OTA storage")
  {
    REQUIRE(ota_storage._binary.size() == 0);
  }

  THEN("The OTA logic should be in the 'Error' state")
  {
    REQUIRE(ota_logic.state() == OTAState::Error);
  }
}