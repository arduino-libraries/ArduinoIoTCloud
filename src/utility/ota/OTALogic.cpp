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

#include "OTALogic.h"

#include <algorithm>

/******************************************************************************
 * CTOR/DTOR
 ******************************************************************************/

OTALogic::OTALogic()
: _ota_state{OTAState::Init}
, _mqtt_ota_buf_length{0}
, _mqtt_ota_buf{0}
{

}

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

void OTALogic::update(OTAStorage * ota_storage)
{
  OTAState next_ota_state = _ota_state;

  switch(_ota_state)
  {
  case OTAState::Init:  next_ota_state = handle_Init(ota_storage); break;
  case OTAState::Idle:  break;
  case OTAState::Error: break;
  }

  _ota_state = next_ota_state;
}

void OTALogic::onOTADataReceived(uint8_t const * const data, size_t const length)
{
  _mqtt_ota_buf_length = std::min(MQTT_OTA_BUF_SIZE, length);
  std::copy(data, data + _mqtt_ota_buf_length, _mqtt_ota_buf);
}

/******************************************************************************
 * PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

OTAState OTALogic::handle_Init(OTAStorage * ota_storage)
{
  if (ota_storage && ota_storage->init()) {
    return OTAState::Idle;
  } else {
    return OTAState::Error;
  }
}
