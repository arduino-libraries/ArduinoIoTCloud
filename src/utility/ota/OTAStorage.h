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

#ifndef ARDUINO_OTA_STORAGE_H_
#define ARDUINO_OTA_STORAGE_H_

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class OTAStorage
{
public:

  virtual ~OTAStorage() { }


  virtual bool   init  () = 0;
  virtual bool   open  () = 0;
  virtual size_t write (uint8_t const * const buf, size_t const num_bytes) = 0;
  virtual void   close () = 0;
  virtual void   remove() = 0;
  virtual bool   rename() = 0;
  virtual void   deinit() = 0;

};

#endif /* ARDUINO_OTA_STORAGE_H_ */
