/*
   Copyright (c) 2024 Arduino.  All rights reserved.
*/

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include <catch.hpp>

#include <TimedAttempt.h>
#include <AIoTC_Config.h>
#include <Arduino.h>
#include <limits.h>

/******************************************************************************
   TEST CODE
 ******************************************************************************/

SCENARIO("Test broker connection retries")
{
  TimedAttempt _connection_attempt(0,0);

  _connection_attempt.begin(AIOT_CONFIG_RECONNECTION_RETRY_DELAY_ms,
                            AIOT_CONFIG_MAX_RECONNECTION_RETRY_DELAY_ms);

  /* 100000 retries are more or less 37 days without connection */
  while(_connection_attempt.getRetryCount() < 100000) {
    _connection_attempt.retry();

   switch(_connection_attempt.getRetryCount()) {
      case 1:
      REQUIRE(_connection_attempt.getWaitTime() == 2000);
      break;
      case 2:
      REQUIRE(_connection_attempt.getWaitTime() == 4000);
      break;
      case 3:
      REQUIRE(_connection_attempt.getWaitTime() == 8000);
      break;
      case 4:
      REQUIRE(_connection_attempt.getWaitTime() == 16000);
      break;
      default:
      REQUIRE(_connection_attempt.getWaitTime() == 32000);
      break;
    }
  }
}

SCENARIO("Test thing id request with no answer from the cloud")
{
  TimedAttempt _attachAttempt(0,0);

  _attachAttempt.begin(AIOT_CONFIG_THING_ID_REQUEST_RETRY_DELAY_ms,
                       AIOT_CONFIG_MAX_THING_ID_REQUEST_RETRY_DELAY_ms);

  /* 100000 retries are more or less 37 days of requests */
  while(_attachAttempt.getRetryCount() < 100000) {
    _attachAttempt.retry();

  switch(_attachAttempt.getRetryCount()) {
      case 1:
      REQUIRE(_attachAttempt.getWaitTime() == 4000);
      break;
      case 2:
      REQUIRE(_attachAttempt.getWaitTime() == 8000);
      break;
      case 3:
      REQUIRE(_attachAttempt.getWaitTime() == 16000);
      break;
      default:
      REQUIRE(_attachAttempt.getWaitTime() == 32000);
      break;
    }
  }
}

SCENARIO("Test thing id request of a detached device")
{
  TimedAttempt _attachAttempt(0,0);

  _attachAttempt.begin(AIOT_CONFIG_THING_ID_REQUEST_RETRY_DELAY_ms,
                       AIOT_CONFIG_MAX_THING_ID_REQUEST_RETRY_DELAY_ms);

  while(_attachAttempt.getRetryCount() < 100000) {
    _attachAttempt.retry();

    switch(_attachAttempt.getRetryCount()) {
      case 1:
      REQUIRE(_attachAttempt.getWaitTime() == 4000);
      _attachAttempt.reconfigure(AIOT_CONFIG_THING_ID_REQUEST_RETRY_DELAY_ms *
                                 AIOT_CONFIG_DEVICE_REGISTERED_RETRY_DELAY_k,
                                 AIOT_CONFIG_MAX_THING_ID_REQUEST_RETRY_DELAY_ms *
                                 AIOT_CONFIG_MAX_DEVICE_REGISTERED_RETRY_DELAY_k);
      break;
      case 2:
      REQUIRE(_attachAttempt.getWaitTime() == 80000);
      break;
      case 3:
      REQUIRE(_attachAttempt.getWaitTime() == 160000);
      break;
      case 4:
      REQUIRE(_attachAttempt.getWaitTime() == 320000);
      break;
      case 5:
      REQUIRE(_attachAttempt.getWaitTime() == 640000);
      break;
      default:
      REQUIRE(_attachAttempt.getWaitTime() == 1280000);
      break;
    }
  }
}

SCENARIO("Test last value request")
{
  TimedAttempt _syncAttempt(0,0);

  _syncAttempt.begin(AIOT_CONFIG_TIMEOUT_FOR_LASTVALUES_SYNC_ms);

  /* 100000 retries are more or less 37 days of requests */
  while(_syncAttempt.getRetryCount() < 100000) {
    _syncAttempt.retry();

    switch(_syncAttempt.getRetryCount()) {
      default:
      REQUIRE(_syncAttempt.getWaitTime() == 30000);
      break;
    }
  }
}

SCENARIO("Test isExpired() and isRetry()")
{
  TimedAttempt attempt(0,0);

  attempt.begin(AIOT_CONFIG_RECONNECTION_RETRY_DELAY_ms,
                AIOT_CONFIG_MAX_RECONNECTION_RETRY_DELAY_ms);

  /* Initial condition */
  set_millis(0);
  REQUIRE(attempt.isExpired() == false);
  REQUIRE(attempt.isRetry() == false);

  /* Normal retry 2000ms */
  attempt.retry();
  REQUIRE(attempt.isRetry() == true);
  set_millis(1000);
  REQUIRE(attempt.isExpired() == false);
  set_millis(1999);
  REQUIRE(attempt.isExpired() == false);
  set_millis(2000);
  REQUIRE(attempt.isExpired() == false);
  set_millis(2001);
  REQUIRE(attempt.isExpired() == true);

  /* Retry with rollover 4000ms */
  set_millis(ULONG_MAX - 1999);
  attempt.retry();
  REQUIRE(attempt.isRetry() == true);
  set_millis(0);
  REQUIRE(attempt.isExpired() == false);
  set_millis(1999);
  REQUIRE(attempt.isExpired() == false);
  set_millis(2000);
  REQUIRE(attempt.isExpired() == false);
  set_millis(2001);
  REQUIRE(attempt.isExpired() == true);

  /* Normal retry 8000ms */
  set_millis(4000);
  attempt.retry();
  REQUIRE(attempt.isRetry() == true);
  set_millis(4000);
  REQUIRE(attempt.isExpired() == false);
  set_millis(11999);
  REQUIRE(attempt.isExpired() == false);
  set_millis(12000);
  REQUIRE(attempt.isExpired() == false);
  set_millis(12001);
  REQUIRE(attempt.isExpired() == true);

  attempt.reset();
  REQUIRE(attempt.isRetry() == false);
}
