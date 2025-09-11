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
  INCLUDE
 ******************************************************************************/

#include <ConnectionHandlerDefinitions.h>
#include "./CBOR.h"
#include <cbor/MessageEncoder.h>
#include "message/Commands.h"

/******************************************************************************
  CLASS DECLARATION
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

class DeviceNetConfigCmdUpEncoder: public CBORMessageEncoderInterface {
public:
  DeviceNetConfigCmdUpEncoder()
  : CBORMessageEncoderInterface(CBORDeviceNetConfigCmdUp, DeviceNetConfigCmdUpId) {}
protected:
  MessageEncoder::Status encode(CborEncoder* encoder, Message *msg) override;
private:
  void getEncodingParams(NetworkAdapter type, uint8_t *typeID, uint8_t *paramsNum);

#if defined(BOARD_HAS_WIFI)
  MessageEncoder::Status encodeWiFiNetwork(CborEncoder* encoder, models::WiFiSetting *config);
#endif

#if defined(BOARD_HAS_CATM1_NBIOT)
  MessageEncoder::Status encodeCatM1Network(CborEncoder* encoder, models::CATM1Setting *config);
#endif

#if defined(BOARD_HAS_ETHERNET)
  MessageEncoder::Status encodeEthernetNetwork(CborEncoder* encoder, models::EthernetSetting *config);
  MessageEncoder::Status encodeIP(CborEncoder* encoder, const models::ip_addr *ip);
#endif

#if defined(BOARD_HAS_NB) || defined(BOARD_HAS_GSM) || defined(BOARD_HAS_CELLULAR)
  MessageEncoder::Status encodeCellularNetwork(CborEncoder* encoder, models::CellularSetting *config);
#endif

#if defined(BOARD_HAS_LORA)
  MessageEncoder::Status encodeLoRaNetwork(CborEncoder* encoder, models::LoraSetting *config);
#endif

};

namespace cbor { namespace encoder { namespace iotcloud {
  /**
   * Some link time optimization may exclude these classes to be instantiated
   * thus it may be required to reference them from outside of this file
   */
  void commandEncoders();
}}}

#endif /* ARDUINO_CBOR_MESSAGE_ENCODER_H_ */
