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

#include <AIoTC_Config.h>
#if OTA_STORAGE_SNU

#include "OTAStorage_SNU.h"

#include <WiFiStorage.h>

/******************************************************************************
 * CONSTANTS
 ******************************************************************************/

static char const SNU_UPDATE_FILENAME[]      = "/fs/UPDATE.BIN";
static char const SNU_TEMP_UPDATE_FILENAME[] = "/fs/UPDATE.BIN.TMP";

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

bool OTAStorage_SNU::init()
{
  /* Ensure that there are no remains of previous
   * aborted downloads still existing in the memory
   * of the nina module.
   */
  WiFiStorage.remove(SNU_TEMP_UPDATE_FILENAME);
  return true;
}

bool OTAStorage_SNU::open(char const * /* file_name */)
{
  /* There's no need to explicitly open the file
   * because when writing to it the file will always
   * be opened with "ab+" mode and closed after each
   * call to 'write'.
   */
  return true;
}

size_t OTAStorage_SNU::write(uint8_t const * const buf, size_t const num_bytes)
{
  WiFiStorageFile file(SNU_TEMP_UPDATE_FILENAME);

  /* We have to write in chunks because otherwise we exceed the size of
   * the SPI buffer within the nina module.
   */
  size_t bytes_written = 0;
  size_t const WRITE_CHUNK_SIZE = 32;
  
  for(; bytes_written < (num_bytes - WRITE_CHUNK_SIZE); bytes_written += WRITE_CHUNK_SIZE)
  {
    if (file.write(buf + bytes_written, WRITE_CHUNK_SIZE) != WRITE_CHUNK_SIZE)
      return bytes_written;
  }

  bytes_written += file.write(buf + bytes_written, num_bytes - bytes_written);

  return bytes_written;
}

void OTAStorage_SNU::close()
{
  /* Files are closed after each file operation on the nina side. */
}

void OTAStorage_SNU::remove(char const * /* file_name */)
{
  WiFiStorage.remove(SNU_TEMP_UPDATE_FILENAME);
}

bool OTAStorage_SNU::rename(char const * /* old_file_name */, char const * /* new_file_name */)
{
  return WiFiStorage.rename(SNU_TEMP_UPDATE_FILENAME, SNU_UPDATE_FILENAME);
}

void OTAStorage_SNU::deinit()
{
  /* Nothing to do */
}

#endif /* OTA_STORAGE_SNU */
