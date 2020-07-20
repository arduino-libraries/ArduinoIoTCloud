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

#ifndef ARDUINO_OTA_FLASH_SHA256_H_
#define ARDUINO_OTA_FLASH_SHA256_H_

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <AIoTC_Config.h>
#if OTA_ENABLED

#include <Arduino.h>

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class FlashSHA256
{
public:

   static String calc(uint32_t const start_addr, uint32_t const max_flash_size);

private:

  FlashSHA256() { }
  FlashSHA256(FlashSHA256 const &) { }

  static constexpr uint32_t FLASH_READ_CHUNK_SIZE = 64;

};

#endif /* OTA_ENABLED */

#endif /* ARDUINO_OTA_FLASH_SHA256_H_ */
