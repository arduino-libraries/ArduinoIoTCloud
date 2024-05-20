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

class Decoder {
public:
  enum Status: uint8_t {
      Complete,
      InProgress,
      Error
  };

  /**
   * Decode a buffer into a provided message structure
   * @param msg: the message structure that is going to be filled with data provided in the buffer
   * @param buf: the incoming buffer that needs to be decoded
   * @param len: the length of the incoming buffer, value will be updated with the used len of the buffer
   * @return SUCCESS: if the message is decoded correctly
   *         ERROR:   if the message wasn't decoded correctly
   */
  virtual Status decode(Message* msg, const uint8_t* const buf, size_t &len) = 0;
};
