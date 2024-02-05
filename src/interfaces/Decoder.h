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

// using decoderFactory=std::function<Decoder*()>;

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

  /**
   * sets the preallocated empty message structure that needs to be decoded, before iterativley decoding it
   * @param Message the message that needs to be decoded, msg should be big enough to accommodate all the required fields
   */
  virtual void setMessage(Message* msg) = 0;

  /**
   * after having the Message struct set call this function to decode he chunks of buffers provided to this call
   * @param buffer the buffer the message will be encoded into
   * @param len the length of the provided buffer, value will be updated with the used len of the buffer
   * @return SUCCESS:     the message is completely encoded
   *         IN_PROGRESS: the message is encoded correctly so far, provide another buffer to continue
   *         ERROR:       error during the encoding of the message
   */
  virtual Status feed(uint8_t* buf, size_t &len) = 0;
};
