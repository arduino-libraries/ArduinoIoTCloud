/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef ARDUINO_IOT_CLOUD_PROCESS
#define ARDUINO_IOT_CLOUD_PROCESS

/******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <message/Commands.h>
#include <interfaces/MessageStream.h>
#include <assert.h>
#include <functional>

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class CloudProcess {
public:
  CloudProcess(MessageStream* stream): stream(stream) {}

  /**
   * Abstract method that is called whenever a message comes from Message stream
   * @param m: the incoming message
   */
  virtual void handleMessage(Message* m) = 0;

  /**
   * Abstract method that is called to update the FSM of the CloudProcess
   */
  virtual void update() = 0;

protected:
  /**
   * Used by a derived class to send a message to the underlying messageStream
   * @param msg: the message to send
   */
  void deliver(Message* msg) {
    assert(stream != nullptr);
    stream->sendUpstream(msg);
  }

private:
  MessageStream* stream;
};

#endif /* ARDUINO_IOT_CLOUD_PROCESS */
