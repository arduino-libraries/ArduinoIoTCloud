/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once
#include <message/Commands.h>
#include <stdint.h>

// using encoderFactory=std::function<Encoder*()>;

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

  /**
   * sets the message that needs to be encoded, before iterativley encoding it
   * @param Message the message that needs to be encoded
   */
  virtual void setMessage(Message* msg) = 0;

  /**
   * after having the Message struct set call this function to encode the message into a buffer
   * this action is performed incrementally into chunks of buffers
   * @param buf: the buffer the message will be encoded into
   * @param len: the length of the provided buffer, value will be updated with the consumed len of the buffer
   * @return SUCCESS:     the message is completely encoded
   *         IN_PROGRESS: the message is encoded correctly so far, provide another buffer to continue
   *         ERROR:       error during the encoding of the message
   */
  virtual Status encode(uint8_t* buf, size_t& len) = 0;
};