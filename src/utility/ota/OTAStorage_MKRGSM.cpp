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

#include "OTAStorage_MKRGSM.h"

#include <Arduino_DebugUtils.h>

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/

OTAStorage_MKRGSM::OTAStorage_MKRGSM()
{
}

// GSMFileUtils _fileUtils;
// String filename = "UPDATE.BIN";
constexpr char * filename { "UPDATE.BIN" };
/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

bool OTAStorage_MKRGSM::init()
{
    return _fileUtils.begin();
}

bool OTAStorage_MKRGSM::open()
{
    auto size = _fileUtils.listFile(filename);
    _fileUtils.deleteFile(filename);
    return true;
}

size_t OTAStorage_MKRGSM::write(uint8_t const* const buf, size_t const num_bytes)
{
    // Serial.print("size: ");
    // Serial.println(num_bytes);
    _fileUtils.appendFile(filename, (const char*)buf, num_bytes);

    return num_bytes;
}

void OTAStorage_MKRGSM::close()
{
}

void OTAStorage_MKRGSM::remove()
{
    _fileUtils.deleteFile(filename);
}

void OTAStorage_MKRGSM::deinit()
{
}
