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

#ifndef ARDUINO_AIOTC_TYPES_H_
#define ARDUINO_AIOTC_TYPES_H_

/******************************************************************************
   TYPEDEF
 ******************************************************************************/

typedef enum
{
  READ      = 0x01,
  WRITE     = 0x02,
  READWRITE = READ | WRITE
} permissionType;

enum class ArduinoIoTConnectionStatus
{
  IDLE,
  CONNECTING,
  CONNECTED,
  DISCONNECTED,
  RECONNECTING,
  ERROR,
};

enum class ArduinoIoTCloudEvent
{
  SYNC = 0, CONNECT = 1, DISCONNECT = 2
};

typedef void (*OnCloudEventCallback)(void);

typedef void (*ExecCloudEventCallback)(ArduinoIoTCloudEvent);

#endif /* ARDUINO_AIOTC_TYPES_H_ */