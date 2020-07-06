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

#ifndef ARDUINO_OTA_STORAGE_MKRGSM_H_
#define ARDUINO_OTA_STORAGE_MKRGSM_H_

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include "OTAStorage.h"

#include <GSMFileUtils.h>

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class OTAStorage_MKRGSM : public OTAStorage
{
public:

  virtual ~OTAStorage_MKRGSM() { }


  virtual Type   type  () override { return Type::MKRGSMFile; }
  virtual bool   init  () override;
  virtual bool   open  (char const * file_name) override;
  virtual size_t write (uint8_t const * const buf, size_t const num_bytes) override;
  virtual void   close () override;
  virtual void   remove(char const * file_name) override;
  virtual bool   rename(char const * old_file_name, char const * new_file_name) override;
  virtual void   deinit() override;

private:

   GSMFileUtils _fileUtils;

};

#endif /* ARDUINO_OTA_STORAGE_MKRGSM_H_ */
