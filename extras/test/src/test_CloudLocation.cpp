/*
   Copyright (c) 2019 Arduino.  All rights reserved.
*/

/**************************************************************************************
   INCLUDE
 **************************************************************************************/

#include <catch.hpp>

#include <property/types/CloudLocation.h>

/**************************************************************************************
  TEST CODE
 **************************************************************************************/

SCENARIO("Testing cloud type 'Location' Ctor", "[Location::Location]")
{
  WHEN("A Location(1.0f, 2.0f) is being instantiated")
  {
    Location loc(1.0f, 2.0f);
    THEN("The member variable 'lat' should be 1.0f") {
      REQUIRE(loc.lat == 1.0f);
    }
    THEN("The member variable 'lon' should be 2.0f") {
      REQUIRE(loc.lon == 2.0f);
    }
  }
}

/**************************************************************************************/

SCENARIO("Testing cloud type 'Location' assignment operator", "[Location::operator =]")
{
  Location loc1(1.0f, 2.0f),
           loc2(3.0f, 4.0f);
  loc1 = loc2;

  WHEN("One location is assigned to the other")
  {
    THEN("The coordinates of the second location should be assigned to the first") {
      REQUIRE(loc1.lat == 3.0f);
      REQUIRE(loc1.lon == 4.0f);
    }
  }
}

/**************************************************************************************/

SCENARIO("Testing cloud type 'Location' operator -", "[Location::operator -]")
{
  Location loc1(1.0f, 2.0f),
           loc2(3.0f, 4.0f);
  Location loc3 = loc1 - loc2;

  WHEN("One location is subtracted from the other")
  {
    THEN("The result should be calculated according the rule lon3 = lon1 - lon2, lat3 = lat1 - lat2") {
      REQUIRE(loc3.lat == loc1.lat - loc2.lat);
      REQUIRE(loc3.lon == loc1.lon - loc2.lon);
    }
  }
}

/**************************************************************************************/

SCENARIO("Testing cloud type 'Location' comparison operator ==", "[Location::operator ==]")
{
  Location loc1(1.0f, 2.0f),
           loc2(3.0f, 4.0f),
           loc3(1.0f, 2.0f);

  WHEN("Two locations are identical (lat as well as lon)")
  {
    THEN("The comparison operation should return true") {
      REQUIRE((loc1 == loc3) == true);
    }
  }

  WHEN("Two locations are not identical (either lat or lon do not match)")
  {
    THEN("The comparison operation should return false") {
      REQUIRE((loc1 == loc2) == false);
    }
  }
}

/**************************************************************************************/

SCENARIO("Testing cloud type 'Location' comparison operator !=", "[Location::operator !=]")
{
  Location loc1(1.0f, 2.0f),
           loc2(3.0f, 4.0f),
           loc3(1.0f, 2.0f);

  WHEN("Two locations are identical (lat as well as lon)")
  {
    THEN("The comparison operation should return false") {
      REQUIRE((loc1 != loc3) == false);
    }
  }

  WHEN("Two locations are not identical (either lat or lon do not match)")
  {
    THEN("The comparison operation should return true") {
      REQUIRE((loc1 != loc2) == true);
    }
  }
}

/**************************************************************************************/

SCENARIO("Testing cloud type 'Location' function distance for calculating Euclidean 2d distance between two points", "[Location::distance]")
{
  Location loc1(0.0f, 0.0f),
           loc2(1.0f, 1.0f);

  REQUIRE(Location::distance(loc1, loc2) == sqrt(2.0f));
}
