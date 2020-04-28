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

#ifndef ARDUINO_OTA_LOGIC_H_
#define ARDUINO_OTA_LOGIC_H_

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include "OTAStorage.h"

/******************************************************************************
 * CONSTANT
 ******************************************************************************/

static size_t const MQTT_OTA_BUF_SIZE = 256;

/******************************************************************************
 * TYPEDEF
 ******************************************************************************/

enum class OTAState
{
  Init, Idle, Error
};

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class OTALogic
{

public:

  OTALogic();


  void update(OTAStorage * ota_storage);
  void onOTADataReceived(uint8_t const * const data, size_t const length);

  inline OTAState state() const { return _ota_state; }


private:

  OTAState _ota_state;
  size_t _mqtt_ota_buf_length;
  uint8_t _mqtt_ota_buf[MQTT_OTA_BUF_SIZE];

  OTAState handle_Init(OTAStorage * ota_storage);

};

#endif /* ARDUINO_OTA_LOGIC_H_ */
