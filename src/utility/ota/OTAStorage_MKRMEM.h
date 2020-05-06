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

#ifndef ARDUINO_OTA_STORAGE_MKRKEM_H_
#define ARDUINO_OTA_STORAGE_MKRKEM_H_

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include "OTAConfig.h"
#if OTA_STORAGE_MKRMEM

#include "OTAStorage.h"

#include <Arduino_MKRMEM.h>

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class OTAStorage_MKRMEM : public OTAStorage
{
public:

           OTAStorage_MKRMEM();
  virtual ~OTAStorage_MKRMEM() { }


  virtual bool   init  () override;
  virtual bool   open  () override;
  virtual size_t write (uint8_t const * const buf, size_t const num_bytes) override;
  virtual void   close () override;
  virtual void   deinit() override;


private:

  File  * _file;

};

#endif /* OTA_STORAGE_MKRMEM */

#endif /* ARDUINO_OTA_STORAGE_MKRKEM_H_ */
