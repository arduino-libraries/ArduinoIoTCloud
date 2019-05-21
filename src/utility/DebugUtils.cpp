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

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include "DebugUtils.h"

#include <stdarg.h>

/******************************************************************************
   GLOBAL VARIABLES
 ******************************************************************************/

static DebugLevel current_debug_level = ARDUINO_IOT_CLOUD_DEFAULT_DEBUG_LEVEL;
static Stream *   debug_output_stream = &Serial;

/******************************************************************************
   PRIVATE PROTOTYPES
 ******************************************************************************/

void vDebugMessage(char const * fmt, va_list args);

/******************************************************************************
   PUBLIC FUNCTIONS
 ******************************************************************************/

void setDebugMessageLevel(int const debug_level) {
  switch (debug_level) {
    case -1: setDebugMessageLevel(DebugLevel::None); break;
    case  0: setDebugMessageLevel(DebugLevel::Error); break;
    case  1: setDebugMessageLevel(DebugLevel::Warning); break;
    case  2: setDebugMessageLevel(DebugLevel::Info); break;
    case  3: setDebugMessageLevel(DebugLevel::Debug); break;
    case  4: setDebugMessageLevel(DebugLevel::Verbose); break;
    default: setDebugMessageLevel(ARDUINO_IOT_CLOUD_DEFAULT_DEBUG_LEVEL); break;
  }
}

void setDebugMessageLevel(DebugLevel const debug_level) {
  current_debug_level = debug_level;
}

void setDebugOutputStream(Stream * stream) {
  debug_output_stream = stream;
}

void debugMessage(DebugLevel const debug_level, const char * fmt, ...) {
  if (debug_level >= DebugLevel::Error   &&
      debug_level <= DebugLevel::Verbose &&
      debug_level <= current_debug_level) {

    char timestamp[20];
    snprintf(timestamp, 20, "[ %lu ] ", millis());
    debug_output_stream->print(timestamp);

    va_list args;
    va_start(args, fmt);
    vDebugMessage(fmt, args);
    va_end(args);
  }
}

void debugMessageNoTimestamp(DebugLevel const debug_level, const char * fmt, ...) {
  if (debug_level >= DebugLevel::Error   &&
      debug_level <= DebugLevel::Verbose &&
      debug_level <= current_debug_level) {

    va_list args;
    va_start(args, fmt);
    vDebugMessage(fmt, args);
    va_end(args);
  }
}

/******************************************************************************
   PRIVATE FUNCTIONS
 ******************************************************************************/

void vDebugMessage(char const * fmt, va_list args) {
  static size_t const MSG_BUF_SIZE = 120;
  char msg_buf[MSG_BUF_SIZE] = {0};

  vsnprintf(msg_buf, MSG_BUF_SIZE, fmt, args);

  debug_output_stream->println(msg_buf);
}
