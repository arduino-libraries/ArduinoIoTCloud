/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef ARDUINO_CBOR_MESSAGE_ENCODER_H_
#define ARDUINO_CBOR_MESSAGE_ENCODER_H_

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include "./CBOR.h"
#include <cbor/MessageEncoder.h>
#include "message/Commands.h"

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class OtaBeginCommandEncoder: public CBORMessageEncoderInterface {
public:
  OtaBeginCommandEncoder()
  : CBORMessageEncoderInterface(CBOROtaBeginUp, OtaBeginUpId) {}
protected:
  MessageEncoder::Status encode(CborEncoder* encoder, Message *msg) override;
};

class ThingBeginCommandEncoder: public CBORMessageEncoderInterface {
public:
  ThingBeginCommandEncoder()
  : CBORMessageEncoderInterface(CBORThingBeginCmd, ThingBeginCmdId) {}
protected:
  MessageEncoder::Status encode(CborEncoder* encoder, Message *msg) override;
};

class LastValuesBeginCommandEncoder: public CBORMessageEncoderInterface {
public:
  LastValuesBeginCommandEncoder()
  : CBORMessageEncoderInterface(CBORLastValuesBeginCmd, LastValuesBeginCmdId) {}
protected:
  MessageEncoder::Status encode(CborEncoder* encoder, Message *msg) override;
};

class DeviceBeginCommandEncoder: public CBORMessageEncoderInterface {
public:
  DeviceBeginCommandEncoder()
  : CBORMessageEncoderInterface(CBORDeviceBeginCmd, DeviceBeginCmdId) {}
protected:
  MessageEncoder::Status encode(CborEncoder* encoder, Message *msg) override;
};

class OtaProgressCommandUpEncoder: public CBORMessageEncoderInterface {
public:
  OtaProgressCommandUpEncoder()
  : CBORMessageEncoderInterface(CBOROtaProgressCmdUp, OtaProgressCmdUpId) {}
protected:
  MessageEncoder::Status encode(CborEncoder* encoder, Message *msg) override;
};

class TimezoneCommandUpEncoder: public CBORMessageEncoderInterface {
public:
  TimezoneCommandUpEncoder()
  : CBORMessageEncoderInterface(CBORTimezoneCommandUp, TimezoneCommandUpId) {}
protected:
  MessageEncoder::Status encode(CborEncoder* encoder, Message *msg) override;
};


#endif /* ARDUINO_CBOR_MESSAGE_ENCODER_H_ */
