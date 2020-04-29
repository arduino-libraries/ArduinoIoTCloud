/*
 * Copyright (c) 2020 Arduino.  All rights reserved.
 */

#ifndef OTA_TEST_DATA_H_
#define OTA_TEST_DATA_H_

/**************************************************************************************
   INCLUDE
 **************************************************************************************/

#include <stdint.h>

/**************************************************************************************
   TYPEDEF
 **************************************************************************************/

union OTAData
{
  struct __attribute__((packed))
  {
    uint32_t len;
    uint32_t crc32;
    uint8_t  bin[256*1024]; /* Maximum flash size of ATSAMD21G18 is 256 KB */
  } data;
  uint8_t buf[sizeof(data)];
};

/**************************************************************************************
   FUNCTION DECLARATION
 **************************************************************************************/

void generate_valid_ota_data(OTAData & ota_data);
void generate_invalid_ota_data_crc_wrong(OTAData & ota_data);


#endif /* OTA_TEST_DATA_H_ */
