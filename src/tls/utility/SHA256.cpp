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
  br_sha256_init(&_ctx);
}

void SHA256::update(uint8_t const * data, size_t const len)
{
  br_sha256_update(&_ctx, data, len);
}

void SHA256::finalize(uint8_t * hash)
{
  br_sha256_out(&_ctx, hash);
}
