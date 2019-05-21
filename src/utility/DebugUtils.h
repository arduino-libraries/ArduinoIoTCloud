/*
   This file is part of ArduinoIoTCloud.

   Copyright 2019 ARDUINO SA (http://www.arduino.cc/)

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

#ifndef DEBUG_UTILS_H_
#define DEBUG_UTILS_H_

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include <Arduino.h>

/******************************************************************************
   TYPEDEF
 ******************************************************************************/

enum class DebugLevel : int {
  None    = -1,
  Error   =  0,
  Warning =  1,
  Info    =  2,
  Debug   =  3,
  Verbose =  4
};

/******************************************************************************
   CONSTANTS
 ******************************************************************************/

static DebugLevel const ARDUINO_IOT_CLOUD_DEFAULT_DEBUG_LEVEL = DebugLevel::Info;

/******************************************************************************
   PROTOTYPES
 ******************************************************************************/

void setDebugMessageLevel(int        const debug_level);   /* For backwards compatibility */
void setDebugMessageLevel(DebugLevel const debug_level);
void setDebugOutputStream(Stream         * stream);
void debugMessage(DebugLevel const debug_level, const char * fmt, ...);
void debugMessageNoTimestamp(DebugLevel const debug_level, const char * fmt, ...);

#endif /* DEBUG_UTILS_H_ */