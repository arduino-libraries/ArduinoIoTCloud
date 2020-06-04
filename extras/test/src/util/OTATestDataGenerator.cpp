/*
 * Copyright (c) 2020 Arduino.  All rights reserved.
 */

/**************************************************************************************
   INCLUDE
 **************************************************************************************/

#include <util/OTATestDataGenerator.h>

#include <algorithm>

#include <crc.h>

/**************************************************************************************
   FUNCTION DEFINITION
 **************************************************************************************/

void generate_valid_ota_data(OTAData & ota_data)
{
  /* Set length. Attention: header length (8 bytes for len and crc32 are not included - this is only the payload length. */
  ota_data.data.len = 64 * 1024;
  
  /* Fill array */
  std::generate(ota_data.data.bin,
                ota_data.data.bin + ota_data.data.len,
                [](void) -> uint8_t
                {
                  static uint8_t val = 0;
                  return val++;
                });

  /* Generate CRC */
  crc_t crc32 = crc_init();
  std::for_each(ota_data.data.bin,
                ota_data.data.bin + ota_data.data.len,
                [&crc32](uint8_t const data)
                {
                  crc32 = crc_update(crc32, &data, 1);
                });
  ota_data.data.crc32 = crc_finalize(crc32);
}

void generate_invalid_ota_data_crc_wrong(OTAData & ota_data)
{
  /* Set length. Attention: header length (8 bytes for len and crc32 are not included - this is only the payload length. */
  ota_data.data.len = 64 * 1024;

  /* Fill array */
  std::generate(ota_data.data.bin,
                ota_data.data.bin + ota_data.data.len,
                [](void) -> uint8_t
                {
                  static uint8_t val = 0;
                  return val++;
                });

  /* Generate CRC */
  ota_data.data.crc32 = 0xDEADBEEF;
}
