/*
   This file is part of ArduinoIoTCloud.

   Copyright 2020 ARDUINO SA (http://www.arduino.cc/)

   This software is released under the GNU General Public License version 3,
   which covers the main part of arduino-cli.
   The terms of this license can be found at:
   https://www.gnu.org/licenses/gpl-3.0.en.html

   You can be released from the requirements of the above licenses by purchasing
   a commercial license. Buying such a license is mandatory if you want to modify or
   otherwise use the software for commercial activities involving the Arduino
   software without disclosing the source code of your own applications. To purchase
   a commercial license, send an email to license@arduino.cc.
*/

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <AIoTC_Config.h>
#if OTA_ENABLED && !defined(__AVR__)

#include "FlashSHA256.h"

#include "../../tls/utility/SHA256.h"

#include <Arduino_DebugUtils.h>

#undef max
#undef min
#include <algorithm>

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

String FlashSHA256::calc(uint32_t const start_addr, uint32_t const max_flash_size)
{
  SHA256  sha256;
  uint8_t chunk     [FLASH_READ_CHUNK_SIZE],
          next_chunk[FLASH_READ_CHUNK_SIZE];

  sha256.begin();

  /* Read the first two chunks of flash. */
  uint32_t flash_addr = start_addr;
  uint32_t bytes_read = 0;
  memcpy(chunk, reinterpret_cast<const void *>(flash_addr), FLASH_READ_CHUNK_SIZE);
  flash_addr += FLASH_READ_CHUNK_SIZE;

  for(; bytes_read < max_flash_size; flash_addr += FLASH_READ_CHUNK_SIZE)
  {
    /* Read the next chunk of memory. */
    memcpy(next_chunk, reinterpret_cast<const void *>(flash_addr), FLASH_READ_CHUNK_SIZE);

    /* Check if the next segment is erased, that is if all bytes within
     * a read segment are 0xFF -> then we've reached the end of the firmware.
     */
    bool const next_chunk_is_erased_flash = std::all_of(next_chunk,
                                                        next_chunk+FLASH_READ_CHUNK_SIZE,
                                                        [](uint8_t const elem) { return (elem == 0xFF); });
    /* Determine how many bytes at the end of the current chunk are
     * already set to 0xFF and therefore erased/non-written flash
     * memory.
     */
    if (next_chunk_is_erased_flash)
    {
      /* Eliminate trailing 0xFF. */
      size_t valid_bytes_in_chunk = 0;
      for(valid_bytes_in_chunk = FLASH_READ_CHUNK_SIZE; valid_bytes_in_chunk > 0; valid_bytes_in_chunk--)
      {
        if (chunk[valid_bytes_in_chunk-1] != 0xFF)
          break;
      }
      /* Update with the remaining bytes. */
      sha256.update(chunk, valid_bytes_in_chunk);
      bytes_read += valid_bytes_in_chunk;
      break;
    }

    /* We've read a normal segment with the next segment not containing
     * any erased elements, just update the SHA256 hash calcultion.
     */
    sha256.update(chunk, FLASH_READ_CHUNK_SIZE);
    bytes_read += FLASH_READ_CHUNK_SIZE;

    /* Copy next_chunk to chunk. */
    memcpy(chunk, next_chunk, FLASH_READ_CHUNK_SIZE);
  }

  /* Retrieve the final hash string. */
  uint8_t sha256_hash[SHA256::HASH_SIZE] = {0};
  sha256.finalize(sha256_hash);
  String sha256_str;
  std::for_each(sha256_hash,
                sha256_hash + SHA256::HASH_SIZE,
                [&sha256_str](uint8_t const elem)
                {
                  char buf[4];
                  snprintf(buf, 4, "%02X", elem);
                  sha256_str += buf;
                });
  /* Do some debug printout. */
  DEBUG_VERBOSE("SHA256: %d bytes read", bytes_read);
  return sha256_str;
}

#endif /* OTA_ENABLED */
