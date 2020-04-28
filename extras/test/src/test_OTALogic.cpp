/*
 * Copyright (c) 2020 Arduino.  All rights reserved.
 */

/**************************************************************************************
   INCLUDE
 **************************************************************************************/

#include <catch.hpp>

#include <OTAStorage_Mock.h>

#include <OTALogic.h>

/**************************************************************************************
   TEST CODE
 **************************************************************************************/

TEST_CASE("A OTALogic object is instantiated", "[OTALogic::OTALogic]")
{
  OTALogic ota_logic;

  REQUIRE(ota_logic.state() == OTAState::Init);

  WHEN("update(nullptr) is run")
  {
    ota_logic.update(nullptr);
    THEN("the logic shall transitition to the Error state") {
      REQUIRE(ota_logic.state() == OTAState::Error);
    }
  }
}

TEST_CASE("A OTALogic object checks is valid OTAStorage is available", "[OTALogic::update-1]")
{
  OTALogic ota_logic;
  OTAStorage_Mock ota_storage;

  WHEN("update(&ota_storage) is run")
  {
    WHEN("OTAStorage::init() succeeds")
    {
      ota_storage._init_return_val = true;
      ota_logic.update(&ota_storage);
      THEN("the logic shall transitition to the Idle state") REQUIRE(ota_logic.state() == OTAState::Idle);
    }
    WHEN("OTAStorage::init() fails")
    {
      ota_storage._init_return_val = false;
      ota_logic.update(&ota_storage);
      THEN("the logic shall transitition to the Error state") REQUIRE(ota_logic.state() == OTAState::Error);
    }
  }
}
