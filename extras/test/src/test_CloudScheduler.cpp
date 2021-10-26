/*
   Copyright (c) 2021 Arduino.  All rights reserved.
*/

/**************************************************************************************
   INCLUDE
 **************************************************************************************/

#include <catch.hpp>

#include <property/types/CloudScheduler.h>

unsigned long time_now = 1;

/**************************************************************************************
 * TimeService Fake CTOR/DTOR
 **************************************************************************************/

TimeService::TimeService() {}

/**************************************************************************************
 * TimeService Fake Methods
 **************************************************************************************/

unsigned long TimeService::getTime() {return time_now;}

/**************************************************************************************
  TEST CODE
 **************************************************************************************/

SCENARIO("Tesing cloud type 'Scheduler' Ctor", "[Scheduler::Scheduler]")
{
  WHEN("A Scheduler(0,0,0,0) is being instantiated")
  {
    Scheduler schedule(0,0,0,0);
    THEN("The member variable 'start' should be 0") {
      REQUIRE(schedule.start == 0);
    }
    THEN("The member variable 'end' should be 0") {
      REQUIRE(schedule.end == 0);
    }
    THEN("The member variable 'duration' should be 0") {
      REQUIRE(schedule.duration == 0);
    }
    THEN("The member variable 'mask' should be 0") {
      REQUIRE(schedule.mask == 0);
    }
  }
}

/**************************************************************************************/

SCENARIO("Setup a schedule that repeats each 20 minutes and test isActive Method", "[Scheduler::isActive]")
{
  Scheduler schedule(1633305600,   /* Start 4/10/2021 00:00:00 */
                     1633651200,   /* End   8/10/2021 00:00:00 */
                            600,   /* Duration        00:10:00 */
                     1140850708    /* Minutes                  */
                                   /* Repeats         00:20:00 */
                    );

  WHEN("Time is 4/10/2021 00:00:00")
  {
    time_now = 1633305600;
    THEN("Schedule must be active") {
      REQUIRE(schedule.isActive() == true);
    }
  }

  WHEN("Time is 4/10/2021 00:10:00")
  {
    time_now = 1633306200;
    THEN("Schedule must be active") {
      REQUIRE(schedule.isActive() == true);
    }
  }

  WHEN("Time is 4/10/2021 00:10:01")
  {
    time_now = 1633306201;
    THEN("Schedule must be inactive") {
      REQUIRE(schedule.isActive() == false);
    }
  }

  WHEN("Time is 4/10/2021 00:19:59")
  {
    time_now = 1633306799;
    THEN("Schedule must be inactive") {
      REQUIRE(schedule.isActive() == false);
    }
  }

  WHEN("Time is 4/10/2021 00:20:00")
  {
    time_now = 1633306800;
    THEN("Schedule must be active") {
      REQUIRE(schedule.isActive() == true);
    }
  }

  WHEN("Time is 7/10/2021 23:45:00")
  {
    time_now = 1633650300;
    THEN("Schedule must be active") {
      REQUIRE(schedule.isActive() == true);
    }
  }

  WHEN("Time is 7/10/2021 23:55:00")
  {
    time_now = 1633650900;
    THEN("Schedule must be inactive") {
      REQUIRE(schedule.isActive() == false);
    }
  }

  WHEN("Time is 8/10/2021 00:00:00")
  {
    time_now = 1633651200;
    THEN("Schedule must be inactive") {
      REQUIRE(schedule.isActive() == false);
    }
  }

  WHEN("Time is 8/10/2021 00:05:00")
  {
    time_now = 1633651500;
    THEN("Schedule must be inactive") {
      REQUIRE(schedule.isActive() == false);
    }
  }
}

/**************************************************************************************/

SCENARIO("Setup a weekly schedule and test isActive Method", "[Scheduler::isActive]")
{
  Scheduler schedule(1633305600,   /* Start 4/10/2021 00:00:00 */
                     1633651200,   /* End   8/10/2021 00:00:00 */
                            600,   /* Duration        00:10:00 */
                      134217798    /* Weekly                   */
                                   /* Daymask          1000110 */
                    );

  WHEN("Time is 4/10/2021 00:05:00")
  {
    time_now = 1633305900;
    THEN("Schedule must be active") {
      REQUIRE(schedule.isActive() == true);
    }
  }

  WHEN("Time is 4/10/2021 00:25:00")
  {
    time_now = 1633307100;
    THEN("Schedule must be inactive") {
      REQUIRE(schedule.isActive() == false);
    }
  }

  WHEN("Time is 5/10/2021 00:05:00")
  {
    time_now = 1633392300;
    THEN("Schedule must be active") {
      REQUIRE(schedule.isActive() == true);
    }
  }

  WHEN("Time is 5/10/2021 00:25:00")
  {
    time_now = 1633393500;
    THEN("Schedule must be inactive") {
      REQUIRE(schedule.isActive() == false);
    }
  }

  WHEN("Time is 6/10/2021 00:05:00")
  {
    time_now = 1633478700;
    THEN("Schedule must be inactive") {
      REQUIRE(schedule.isActive() == false);
    }
  }

  WHEN("Time is 7/10/2021 00:05:00")
  {
    time_now = 1633565100;
    THEN("Schedule must be inactive") {
      REQUIRE(schedule.isActive() == false);
    }
  }
}
