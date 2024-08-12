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

#include "SHA256.h"

/******************************************************************************
 * STATIC MEMBER DECLARATION
 ******************************************************************************/

constexpr size_t SHA256::HASH_SIZE;

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

void SHA256::begin()
{
#if defined(HAS_BEARSSL)
  br_sha256_init(&_ctx);
#else
  mbedtls_sha256_init(&_ctx);
  mbedtls_sha256_starts(&_ctx, 0);
#endif
}

void SHA256::update(uint8_t const * data, size_t const len)
{
#if defined(HAS_BEARSSL)
  br_sha256_update(&_ctx, data, len);
#else
  mbedtls_sha256_update(&_ctx, data, len);
#endif
}

void SHA256::finalize(uint8_t * hash)
{
#if defined(HAS_BEARSSL)
  br_sha256_out(&_ctx, hash);
#else
  mbedtls_sha256_finish(&_ctx, hash);
#endif
}
