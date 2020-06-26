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

#include <ArduinoIoTCloud_Config.h>
#if OTA_STORAGE_NINA

#include "OTAStorage_Nina.h"

/******************************************************************************
 * CTOR/DTOR
 ******************************************************************************/

OTAStorage_Nina::OTAStorage_Nina()
: _file{nullptr}
{

}

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

bool OTAStorage_Nina::init()
{
  /* Nothing to do */
}

bool OTAStorage_Nina::open(char const * file_name)
{
  /* It is necessary to prepend "/fs/" when opening a file on the nina
   * for the rename operation "/fs/"" does not need to be prepended.
   */
  char nina_file_name[32] = {0};
  strcpy(nina_file_name, "/fs/");
  strcat(nina_file_name, file_name);
  
  WiFiStorageFile file = WiFiStorage.open(nina_file_name);
  if(!file)
    return false;
  
  _file = new WiFiStorageFile(file);
  return true;
}

size_t OTAStorage_Nina::write(uint8_t const * const buf, size_t const num_bytes)
{
  return _file->write(buf, num_bytes);
}

void OTAStorage_Nina::close()
{
  /* There is no close API within WiFiNiNa */
  delete _file;
}

void OTAStorage_Nina::remove(char const * file_name)
{
  WiFiStorage.remove(file_name);
}

bool OTAStorage_Nina::rename(char const * old_file_name, char const * new_file_name)
{
  return WiFiStorage.rename(old_file_name, new_file_name);
}

void OTAStorage_Nina::deinit()
{
  /* Nothing to do */
}

#endif /* OTA_STORAGE_NINA */
