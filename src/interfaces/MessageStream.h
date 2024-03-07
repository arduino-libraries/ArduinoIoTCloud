/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <message/Commands.h>
#include <functional>

using upstreamFunction = std::function<void(Message*)>;

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class MessageStream {
public:
  MessageStream(upstreamFunction upstream): upstream(upstream) {}

  /**
   * Send message upstream
   * @param m: message to send
   */
  virtual inline void sendUpstream(Message* m) {
    upstream(m);
  }

private:
  upstreamFunction upstream;
};
