/*
 * This file is part of ArduinoIoTCloud.
 *
 * Copyright 2019 ARDUINO SA (http://www.arduino.cc/)
 *
 * This software is released under the GNU General Public License version 3,
 * which covers the main part of arduino-cli.
 * The terms of this license can be found at:
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * You can be released from the requirements of the above licenses by purchasing
 * a commercial license. Buying such a license is mandatory if you want to modify or
 * otherwise use the software for commercial activities involving the Arduino
 * software without disclosing the source code of your own applications. To purchase
 * a commercial license, send an email to license@arduino.cc.
 */

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include "DebugUtils.h"

#include <stdarg.h>

/******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

static int debugMessageLevel = ARDUINO_IOT_CLOUD_DEFAULT_DEBUG_LEVEL;

/******************************************************************************
 * PRIVATE PROTOTYPES
 ******************************************************************************/

void vDebugMessage(char const * fmt, va_list args);

/******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

void setDebugMessageLevel(int const debugLevel) {
  debugMessageLevel = debugLevel;
}

void debugMessage(int const debugLevel, char * fmt, ...) {
  if(debugLevel >= 0 && debugLevel <= debugMessageLevel) {
    char timestamp[20];
    snprintf(timestamp, 20, "[ %d ] ", millis());
    Serial.print(timestamp);
    
    va_list args;
    va_start(args, fmt);
    vDebugMessage(fmt, args);
    va_end(args);
  }
}

/******************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************/

void vDebugMessage(char const * fmt, va_list args)
{
  static size_t const MSG_BUF_SIZE = 120;
  char msg_buf[MSG_BUF_SIZE] = {0};

  vsnprintf(msg_buf, MSG_BUF_SIZE, fmt, args);

  Serial.println(msg_buf);
}
