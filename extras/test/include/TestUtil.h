/*
   Copyright (c) 2019 Arduino.  All rights reserved.
*/

#ifndef INCLUDE_TESTUTIL_H_
#define INCLUDE_TESTUTIL_H_

/**************************************************************************************
   INCLUDE
 **************************************************************************************/

#include <ArduinoCloudThing.h>

#include <vector>

/**************************************************************************************
   PROTOTYPES
 **************************************************************************************/

std::vector<uint8_t> encode(ArduinoCloudThing & thing, bool lightPayload = false);
void print(std::vector<uint8_t> const & vect);

#endif /* INCLUDE_TESTUTIL_H_ */
