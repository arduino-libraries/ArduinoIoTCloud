/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef ARDUINO_CBOR_MESSAGE_DECODER_H_
#define ARDUINO_CBOR_MESSAGE_DECODER_H_

/******************************************************************************
  INCLUDE
 ******************************************************************************/

#include "./CBOR.h"
#include <cbor/MessageDecoder.h>
#include "message/Commands.h"

/******************************************************************************
  CLASS DECLARATION
 ******************************************************************************/

class OtaUpdateCommandDecoder: public CBORMessageDecoderInterface {
public:
  OtaUpdateCommandDecoder()
  : CBORMessageDecoderInterface(CBOROtaUpdateCmdDown, OtaUpdateCmdDownId) {}
protected:
  MessageDecoder::Status decode(CborValue* iter, Message *msg) override;
};

class ThingUpdateCommandDecoder: public CBORMessageDecoderInterface {
public:
  ThingUpdateCommandDecoder()
  : CBORMessageDecoderInterface(CBORThingUpdateCmd, ThingUpdateCmdId) {}
protected:
  MessageDecoder::Status decode(CborValue* iter, Message *msg) override;
};

class ThingDetachCommandDecoder: public CBORMessageDecoderInterface {
public:
  ThingDetachCommandDecoder()
  : CBORMessageDecoderInterface(CBORThingDetachCmd, ThingDetachCmdId) {}
protected:
  MessageDecoder::Status decode(CborValue* iter, Message *msg) override;
};

class LastValuesUpdateCommandDecoder: public CBORMessageDecoderInterface {
public:
  LastValuesUpdateCommandDecoder()
  : CBORMessageDecoderInterface(CBORLastValuesUpdate, LastValuesUpdateCmdId) {}
protected:
  MessageDecoder::Status decode(CborValue* iter, Message *msg) override;
};

class TimezoneCommandDownDecoder: public CBORMessageDecoderInterface {
public:
  TimezoneCommandDownDecoder()
  : CBORMessageDecoderInterface(CBORTimezoneCommandDown, TimezoneCommandDownId) {}
protected:
  MessageDecoder::Status decode(CborValue* iter, Message *msg) override;
};

namespace cbor { namespace decoder { namespace iotcloud {
  /**
   * Some link time optimization may exclude these classes to be instantiated
   * thus it may be required to reference them from outside of this file
   */
  void commandDecoders();
}}}

#endif /* ARDUINO_CBOR_MESSAGE_DECODER_H_ */
