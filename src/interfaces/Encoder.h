/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

/******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include <message/Commands.h>
#include <stdint.h>

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class Encoder {
public:
  enum Status: uint8_t {
      Complete,
      InProgress,
      Error
  };

  /**
   * Encode a message into a buffer in a single shot
   * @param msg: the message that needs to be encoded
   * @param buf: the buffer the message will be encoded into
   * @param len: the length of the provided buffer, value will be updated with the consumed len of the buffer
   * @return SUCCESS: if the message is encoded correctly
   *         ERROR:   error during the encoding of the message
   */
  virtual Status encode(Message* msg, uint8_t* buf, size_t& len) = 0;
};
