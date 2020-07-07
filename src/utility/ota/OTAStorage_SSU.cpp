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
   INCLUDE
 ******************************************************************************/

#include <ArduinoIoTCloud_Config.h>
#if OTA_STORAGE_SSU

#include "OTAStorage_SSU.h"

/******************************************************************************
   CONSTANTS
 ******************************************************************************/

static char const SSU_UPDATE_FILENAME[] = "UPDATE.BIN";
static char const SSU_CHECK_FILE_NAME[] = "UPDATE.OK";

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

bool OTAStorage_SSU::init()
{
  if (!_fileUtils.begin())
    return false;

  if (_fileUtils.listFile(SSU_UPDATE_FILENAME) > 0)
    if (!_fileUtils.deleteFile(SSU_UPDATE_FILENAME))
      return false;

  if (_fileUtils.listFile(SSU_CHECK_FILE_NAME) > 0)
    if (!_fileUtils.deleteFile(SSU_CHECK_FILE_NAME))
      return false;
}

bool OTAStorage_SSU::open(char const * /* file_name */)
{
  return true;
}

size_t OTAStorage_SSU::write(uint8_t const* const buf, size_t const num_bytes)
{
  _fileUtils.appendFile(SSU_UPDATE_FILENAME, (const char*)buf, num_bytes);
  return num_bytes;
}

void OTAStorage_SSU::close()
{
  /* Nothing to do */
}

void OTAStorage_SSU::remove(char const * /* file_name */)
{
  _fileUtils.deleteFile(SSU_UPDATE_FILENAME);
}

bool OTAStorage_SSU::rename(char const * /* old_file_name */, char const * /* new_file_name */)
{
  /* Create a file 'UPDATE.OK' which is used by the SSU
   * 2nd stage bootloader to recognise that the update
   * went okay. Normally this is done by renaming 'UPDATE.BIN.TMP'
   * to 'UPDATE.BIN' but the SARE module does not support
   * a rename function.
   */
  char c = 'X';
  return (_fileUtils.appendFile(SSU_CHECK_FILE_NAME, &c, sizeof(c)) == sizeof(c));
}

void OTAStorage_SSU::deinit()
{
  /* Nothing to do */
}

#endif /* OTA_STORAGE_SSU */
