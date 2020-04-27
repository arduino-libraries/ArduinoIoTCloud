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

#include "OTAStorage_MKRMEM.h"
#if OTA_STORAGE_MKRMEM

#include <Arduino_DebugUtils.h>

/******************************************************************************
 * CTOR/DTOR
 ******************************************************************************/

OTAStorage_MKRMEM::OTAStorage_MKRMEM()
: _file{nullptr}
{

}

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

bool OTAStorage_MKRMEM::init()
{
  flash.begin();
  if(SPIFFS_OK != filesystem.mount()) {
    Debug.print(DBG_ERROR, "OTAStorage_MKRMEM::init - mount() failed with error code %d", filesystem.err());
    return false;
  }

  if(SPIFFS_OK != filesystem.check()) {
    Debug.print(DBG_ERROR, "OTAStorage_MKRMEM::init - check() failed with error code %d", filesystem.err());
    return false;
  }

  return true;
}

bool OTAStorage_MKRMEM::open()
{
  filesystem.clearerr();
  _file = new File(filesystem.open("UPDATE.bin", CREATE | WRITE_ONLY| TRUNCATE));
  if(SPIFFS_OK != filesystem.err()) {
    Debug.print(DBG_ERROR, "OTAStorage_MKRMEM::open - open() failed with error code %d", filesystem.err());
    delete _file;
    return false;
  }
  return true;
}

size_t OTAStorage_MKRMEM::write(uint8_t const * const buf, size_t const num_bytes)
{
  return _file->write(const_cast<uint8_t *>(buf), num_bytes);
}

void OTAStorage_MKRMEM::close()
{
  _file->close();
  delete _file;
}

void OTAStorage_MKRMEM::deinit()
{
  filesystem.unmount();
}

#endif /* OTA_STORAGE_MKRMEM */
