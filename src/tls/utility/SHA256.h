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

#ifndef ARDUINO_TLS_UTILITY_SHA256_H_
#define ARDUINO_TLS_UTILITY_SHA256_H_

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include "../bearssl/bearssl_hash.h"

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class SHA256
{

public:

  static constexpr size_t HASH_SIZE = 32;

  void begin   ();
  void update  (uint8_t const * data, size_t const len);
  void finalize(uint8_t * hash);

private:

  br_sha256_context _ctx;

};

#endif /* ARDUINO_TLS_UTILITY_SHA256_H_ */
