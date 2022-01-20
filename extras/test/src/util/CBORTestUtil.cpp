/*
   Copyright (c) 2019 Arduino.  All rights reserved.
*/

/**************************************************************************************
   INCLUDE
 **************************************************************************************/

#include <util/CBORTestUtil.h>

#include <iomanip>
#include <iostream>

#include <CBOREncoder.h>

/**************************************************************************************
   NAMESPACE
 **************************************************************************************/

namespace cbor
{

/**************************************************************************************
   PUBLIC FUNCTIONS
 **************************************************************************************/

std::vector<uint8_t> encode(PropertyContainer & property_container, bool lightPayload)
{
  int bytes_encoded = 0;
  unsigned int starting_property_index = 0;
  uint8_t buf[256] = {0};

  if (CBOREncoder::encode(property_container, buf, 256, bytes_encoded, starting_property_index, lightPayload) == CborNoError)
    return std::vector<uint8_t>(buf, buf + bytes_encoded);
  else
    return std::vector<uint8_t>();
}

void print(std::vector<uint8_t> const & vect)
{
  for (auto i = vect.begin(); i != vect.end(); i++) {
    std::cout << std::uppercase
              << std::hex
              << std::setw(2)
              << std::setfill('0')
              << static_cast<size_t>(*i)
              << std::dec
              << std::nouppercase
              << " ";
  }
  std::cout << std::endl;
}

/**************************************************************************************
   NAMESPACE
 **************************************************************************************/

} /* cbor */
