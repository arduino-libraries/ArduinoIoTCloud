/*
 * Copyright (c) 2020 Arduino.  All rights reserved.
 */

/**************************************************************************************
   INCLUDE
 **************************************************************************************/

#include <vector>
#include <algorithm>

#include <catch.hpp>
#include <fakeit.hpp>

#include <util/OTATestData.h>

#include <OTALogic.h>
#include <OTAStorage.h>

/**************************************************************************************
   NAMESPACE
 **************************************************************************************/

using namespace fakeit;

/**************************************************************************************
   TEST HELPER
 **************************************************************************************/

void simulateOTABinaryReception(OTALogic & ota_logic, OTAData const & ota_test_data)
{
  uint32_t bytes_written = 0;
  uint32_t const bytes_to_write = sizeof(uint32_t) + sizeof(uint32_t) + ota_test_data.data.len;
  for(; bytes_written < (bytes_to_write - MQTT_OTA_BUF_SIZE); bytes_written += MQTT_OTA_BUF_SIZE)
  {
    ota_logic.onOTADataReceived(ota_test_data.buf + bytes_written, MQTT_OTA_BUF_SIZE);
    ota_logic.update();
  }

  if(bytes_written < bytes_to_write)
  {
    uint32_t const remaining_bytes = (bytes_to_write - bytes_written);
    ota_logic.onOTADataReceived(ota_test_data.buf + bytes_written, remaining_bytes);
    ota_logic.update();
  }
}

/**************************************************************************************
   TEST CODE
 **************************************************************************************/

TEST_CASE("OTAStorage initialisation fails", "[OTAStorage::init() -> returns false]")
{
  Mock<OTAStorage> ota_storage;

  /* Configure mock object */
  When(Method(ota_storage, init)).Return(false);
  Fake(Method(ota_storage, open));
  Fake(Method(ota_storage, write));
  Fake(Method(ota_storage, close));
  Fake(Method(ota_storage, remove));
  Fake(Method(ota_storage, deinit));


  /* Perform test */
  OTALogic ota_logic;
  ota_logic.setOTAStorage(ota_storage.get());

  WHEN("OTALogic::update() is called")
  {
    ota_logic.update();
    THEN("The OTA logic should be in the 'Error' state")
    {
      REQUIRE(ota_logic.state()  == OTAState::Error);
    }
    THEN("The OTA error should be set to OTAError::StorageInitFailed")
    {
      REQUIRE(ota_logic.error() == OTAError::StorageInitFailed);
    }
  }
}

/**************************************************************************************/

TEST_CASE("OTAStorage opening of storage file fails", "[OTAStorage::open() -> returns false]")
{
  Mock<OTAStorage> ota_storage;

  /* Configure mock object */
  When(Method(ota_storage, init)).Return(true);
  When(Method(ota_storage, open)).Return(false);
  Fake(Method(ota_storage, write));
  Fake(Method(ota_storage, close));
  Fake(Method(ota_storage, remove));
  Fake(Method(ota_storage, deinit));


  /* Perform test */
  OTALogic ota_logic;
  ota_logic.setOTAStorage(ota_storage.get());

  WHEN("OTALogic::update() is called and some bytes have been received")
  {
    uint8_t const SOME_FAKE_DATA[16] = {0};
    ota_logic.onOTADataReceived(SOME_FAKE_DATA, sizeof(SOME_FAKE_DATA));
    ota_logic.update();
    
    THEN("The OTA logic should be in the 'Error' state")
    {
      REQUIRE(ota_logic.state()  == OTAState::Error);
    }
    THEN("The OTA error should be set to OTAError::StorageOpenFailed")
    {
      REQUIRE(ota_logic.error() == OTAError::StorageOpenFailed);
    }
  }
}


/**************************************************************************************/

TEST_CASE("OTAStorage writing to storage file fails", "[OTAStorage::write() -> fails]")
{
  Mock<OTAStorage> ota_storage;

  /* Configure mock object */
  When(Method(ota_storage, init)).Return(true);
  When(Method(ota_storage, open)).Return(true);
  When(Method(ota_storage, write)).AlwaysDo([](uint8_t const * const /* buf */, size_t const /* num_bytes */) -> size_t { return 0 /* should return num_bytes in case of success */;});
  Fake(Method(ota_storage, close));
  Fake(Method(ota_storage, remove));
  Fake(Method(ota_storage, deinit));


  /* Perform test */
  OTALogic ota_logic;
  ota_logic.setOTAStorage(ota_storage.get());

  WHEN("OTALogic::update() is called and some bytes have been received")
  {
    uint8_t const SOME_FAKE_DATA[16] = {0};
    ota_logic.onOTADataReceived(SOME_FAKE_DATA, sizeof(SOME_FAKE_DATA));
    ota_logic.update();

    THEN("The OTA logic should be in the 'Error' state")
    {
      REQUIRE(ota_logic.state()  == OTAState::Error);
    }
    THEN("The OTA error should be set to OTAError::StorageWriteFailed")
    {
      REQUIRE(ota_logic.error() == OTAError::StorageWriteFailed);
    }
  }
}

/**************************************************************************************/

TEST_CASE("Data overrun due to receiving too much data", "[OTALogic - Data Overrun]")
{
  Mock<OTAStorage> ota_storage;

  /* Configure mock object */
  When(Method(ota_storage, init)).Return(true);
  When(Method(ota_storage, open)).Return(true);
  When(Method(ota_storage, write)).AlwaysDo([](uint8_t const * const /* buf */, size_t const num_bytes) -> size_t { return num_bytes; });
  Fake(Method(ota_storage, close));
  Fake(Method(ota_storage, remove));
  Fake(Method(ota_storage, deinit));


  /* Perform test */
  OTALogic ota_logic;
  ota_logic.setOTAStorage(ota_storage.get());

  WHEN("Too much data is received before OTALogic::update() is called again to process the incoming data")
  {
    uint8_t const SOME_FAKE_DATA[MQTT_OTA_BUF_SIZE] = {0};
    ota_logic.onOTADataReceived(SOME_FAKE_DATA, MQTT_OTA_BUF_SIZE);
    ota_logic.onOTADataReceived(SOME_FAKE_DATA, MQTT_OTA_BUF_SIZE);
    ota_logic.update();

    THEN("The OTA logic should be in the 'Error' state")
    {
      REQUIRE(ota_logic.state()  == OTAState::Error);
    }
    THEN("The OTA error should be set to OTAError::ReceivedDataOverrun")
    {
      REQUIRE(ota_logic.error() == OTAError::ReceivedDataOverrun);
    }
  }
}

/**************************************************************************************/

TEST_CASE("Valid OTA data is received ", "[OTALogic]")
{
  Mock<OTAStorage> ota_storage;
  std::vector<uint8_t> ota_binary_data;

  /* Configure mock object */
  When(Method(ota_storage, init)).Return(true);
  When(Method(ota_storage, open)).Return(true);
  When(Method(ota_storage, write)).AlwaysDo(
    [&ota_binary_data](uint8_t const * const buf, size_t const num_bytes) -> size_t
    {
      std::copy(buf, buf + num_bytes, std::back_inserter(ota_binary_data));
      return num_bytes;
    });
  Fake(Method(ota_storage, close)); 
  Fake(Method(ota_storage, remove)); 
  Fake(Method(ota_storage, deinit)); 


  /* Generate test data */
  OTAData valid_ota_test_data;
  generate_valid_ota_data(valid_ota_test_data);


  /* Perform test */
  OTALogic ota_logic;
  ota_logic.setOTAStorage(ota_storage.get());
  simulateOTABinaryReception(ota_logic, valid_ota_test_data);


  /* Perform checks */
  THEN("the complete binary file should have been written to the OTA storage")
  {
    REQUIRE(ota_binary_data.size() == valid_ota_test_data.data.len);
    REQUIRE(std::equal(ota_binary_data.begin(),
                       ota_binary_data.end(),
                       valid_ota_test_data.data.bin));
  }

  THEN("The OTA logic should be in the 'Reset' state")
  {
    REQUIRE(ota_logic.state() == OTAState::Reset);
  }

  THEN("No OTA error should have occurred")
  {
    REQUIRE(ota_logic.error() == OTAError::None);
  }
}

/**************************************************************************************/

TEST_CASE("Invalid OTA data is received ", "[OTALogic - CRC wrong]")
{
  Mock<OTAStorage> ota_storage;


  /* Configure mock object */
  When(Method(ota_storage, init)).Return(true);
  When(Method(ota_storage, open)).Return(true);
  When(Method(ota_storage, write)).AlwaysDo([](uint8_t const * const /* buf */, size_t const num_bytes) -> size_t { return num_bytes; });
  Fake(Method(ota_storage, close)); 
  Fake(Method(ota_storage, remove)); 
  Fake(Method(ota_storage, deinit)); 


  /* Generate test data */
  OTAData invalid_valid_ota_test_data_crc_wrong;
  generate_invalid_ota_data_crc_wrong(invalid_valid_ota_test_data_crc_wrong);


  /* Perform test */
  OTALogic ota_logic;
  ota_logic.setOTAStorage(ota_storage.get());
  simulateOTABinaryReception(ota_logic, invalid_valid_ota_test_data_crc_wrong);

  
  /* Perform checks */
  THEN("there should be no binary file be stored on the OTA storage")
  {
    Verify(Method(ota_storage, remove)).Once();
  }

  THEN("The OTA logic should be in the 'Error' state")
  {
    REQUIRE(ota_logic.state() == OTAState::Error);
  }

  THEN("The OTA error should be set to OTAError::ChecksumMismatch")
  {
    REQUIRE(ota_logic.error() == OTAError::ChecksumMismatch);
  }
}