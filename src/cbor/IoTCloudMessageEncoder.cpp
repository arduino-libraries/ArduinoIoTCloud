/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/******************************************************************************
  INCLUDE
 ******************************************************************************/

#include "CBOREncoder.h"

#include "IoTCloudMessageEncoder.h"

/******************************************************************************
  PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/


/******************************************************************************
  PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

MessageEncoder::Status OtaBeginCommandEncoder::encode(CborEncoder* encoder, Message *msg) {
  OtaBeginUp * otaBeginUp = (OtaBeginUp*) msg;
  CborEncoder array_encoder;

  if(cbor_encoder_create_array(encoder, &array_encoder, 1) != CborNoError) {
    return MessageEncoder::Status::Error;
  }

  if(cbor_encode_byte_string(&array_encoder, otaBeginUp->params.sha, SHA256_SIZE) != CborNoError) {
    return MessageEncoder::Status::Error;
  }

  if(cbor_encoder_close_container(encoder, &array_encoder) != CborNoError) {
    return MessageEncoder::Status::Error;
  }

  return MessageEncoder::Status::Complete;
}

MessageEncoder::Status ThingBeginCommandEncoder::encode(CborEncoder* encoder, Message *msg) {
  ThingBeginCmd * thingBeginCmd = (ThingBeginCmd*) msg;
  CborEncoder array_encoder;

  if(cbor_encoder_create_array(encoder, &array_encoder, 1) != CborNoError) {
      return MessageEncoder::Status::Error;
  }

  if(cbor_encode_text_stringz(&array_encoder, thingBeginCmd->params.thing_id) != CborNoError) {
    return MessageEncoder::Status::Error;
  }

  if(cbor_encoder_close_container(encoder, &array_encoder) != CborNoError) {
    return MessageEncoder::Status::Error;
  }

  return MessageEncoder::Status::Complete;
}

MessageEncoder::Status LastValuesBeginCommandEncoder::encode(CborEncoder* encoder, Message *msg) {
  // This command contains no parameters, it contains just the id of the message
  // nothing to perform here
  // (void)(encoder);
  (void)(msg);
  CborEncoder array_encoder;

  /* FIXME  We are encoiding an empty array, this could be avoided
            if the cloud cbor decode is able to accept an empty array */
  if (cbor_encoder_create_array(encoder, &array_encoder, 0) != CborNoError){
    return MessageEncoder::Status::Error;
  }

  if(cbor_encoder_close_container(encoder, &array_encoder) != CborNoError) {
    return MessageEncoder::Status::Error;
  }

  return MessageEncoder::Status::Complete;
}

MessageEncoder::Status DeviceBeginCommandEncoder::encode(CborEncoder* encoder, Message *msg) {
  DeviceBeginCmd * deviceBeginCmd = (DeviceBeginCmd*) msg;
  CborEncoder array_encoder;

  if(cbor_encoder_create_array(encoder, &array_encoder, 1) != CborNoError) {
      return MessageEncoder::Status::Error;
  }

  if(cbor_encode_text_stringz(&array_encoder, deviceBeginCmd->params.lib_version) != CborNoError) {
    return MessageEncoder::Status::Error;
  }

  if(cbor_encoder_close_container(encoder, &array_encoder) != CborNoError) {
    return MessageEncoder::Status::Error;
  }

  return MessageEncoder::Status::Complete;
}

MessageEncoder::Status OtaProgressCommandUpEncoder::encode(CborEncoder* encoder, Message *msg) {
  OtaProgressCmdUp * ota = (OtaProgressCmdUp*) msg;
  CborEncoder array_encoder;

  if(cbor_encoder_create_array(encoder, &array_encoder, 4) != CborNoError) {
      return MessageEncoder::Status::Error;
  }

  if(cbor_encode_byte_string(&array_encoder, ota->params.id, ID_SIZE) != CborNoError) {
    return MessageEncoder::Status::Error;
  }

  if(cbor_encode_simple_value(&array_encoder, ota->params.state) != CborNoError) {
    return MessageEncoder::Status::Error;
  }

  if(cbor_encode_int(&array_encoder, ota->params.state_data) != CborNoError) {
    return MessageEncoder::Status::Error;
  }

  if(cbor_encode_uint(&array_encoder, ota->params.time) != CborNoError) {
    return MessageEncoder::Status::Error;
  }

  if(cbor_encoder_close_container(encoder, &array_encoder) != CborNoError) {
    return MessageEncoder::Status::Error;
  }

  return MessageEncoder::Status::Complete;
}

MessageEncoder::Status TimezoneCommandUpEncoder::encode(CborEncoder* encoder, Message *msg) {
  // This command contains no parameters, it contains just the id of the message
  // nothing to perform here
  // (void)(encoder);
  (void)(msg);
  CborEncoder array_encoder;

  /* FIXME  We are encoiding an empty array, this could be avoided
            if the cloud cbor decode is able to accept an empty array */
  if (cbor_encoder_create_array(encoder, &array_encoder, 0) != CborNoError){
    return MessageEncoder::Status::Error;
  }

  if(cbor_encoder_close_container(encoder, &array_encoder) != CborNoError) {
    return MessageEncoder::Status::Error;
  }

  return MessageEncoder::Status::Complete;
}

MessageEncoder::Status DeviceNetConfigCmdUpEncoder::encode(CborEncoder* encoder, Message *msg) {
  DeviceNetConfigCmdUp * netConfig = (DeviceNetConfigCmdUp*) msg;
  CborEncoder array_encoder;

  uint8_t typeID, paramsNum;
  getEncodingParams(netConfig->params.type, &typeID, &paramsNum);

  if(cbor_encoder_create_array(encoder, &array_encoder, 1 + paramsNum) != CborNoError) {
    return MessageEncoder::Status::Error;
  }

  if(cbor_encode_uint(&array_encoder, typeID) != CborNoError) {
    return MessageEncoder::Status::Error;
  }

  MessageEncoder::Status encodeStatus = MessageEncoder::Status::Complete;

  switch (netConfig->params.type)
  {
  #if defined(BOARD_HAS_WIFI)
    case NetworkAdapter::WIFI:
      encodeStatus = encodeWiFiNetwork(&array_encoder, &netConfig->params.wifi);
      break;
  #endif // defined(BOARD_HAS_WIFI)

  #if defined(BOARD_HAS_LORA)
    case NetworkAdapter::LORA:
      encodeStatus = encodeLoRaNetwork(&array_encoder, &netConfig->params.lora);
      break;
  #endif // defined(BOARD_HAS_LORA)

  #if defined(BOARD_HAS_GSM)
    case NetworkAdapter::GSM:
      encodeStatus = encodeCellularNetwork(&array_encoder, &netConfig->params.gsm);
      break;
  #endif // defined(BOARD_HAS_GSM)

  #if defined(BOARD_HAS_NB)
    case NetworkAdapter::NB:
      encodeStatus = encodeCellularNetwork(&array_encoder, &netConfig->params.nb);
      break;
  #endif // defined(BOARD_HAS_NB)

  #if defined(BOARD_HAS_CATM1_NBIOT)
    case NetworkAdapter::CATM1:
      encodeStatus = encodeCatM1Network(&array_encoder, &netConfig->params.catm1);
      break;
  #endif // defined(BOARD_HAS_CATM1_NBIOT)

  #if defined(BOARD_HAS_ETHERNET)
    case NetworkAdapter::ETHERNET:
      encodeStatus = encodeEthernetNetwork(&array_encoder, &netConfig->params.eth);
      break;
  #endif // defined(BOARD_HAS_ETHERNET)

  #if defined(BOARD_HAS_CELLULAR)
    case NetworkAdapter::CELL:
      encodeStatus = encodeCellularNetwork(&array_encoder, &netConfig->params.cell);
      break;
  #endif // defined(BOARD_HAS_CELLULAR)

    default:
      // Nothing to encode
      break;
  }

  if(encodeStatus != MessageEncoder::Status::Complete) {
    return MessageEncoder::Status::Error;
  }

  if(cbor_encoder_close_container(encoder, &array_encoder) != CborNoError) {
    return MessageEncoder::Status::Error;
  }

  return MessageEncoder::Status::Complete;
}

void DeviceNetConfigCmdUpEncoder::getEncodingParams(NetworkAdapter type, uint8_t *typeID, uint8_t *paramsNum) {
  switch (type)
  {
    case NetworkAdapter::WIFI:     *typeID = 1; *paramsNum = 1; break;
    case NetworkAdapter::LORA:     *typeID = 2; *paramsNum = 1; break;
    case NetworkAdapter::GSM:      *typeID = 3; *paramsNum = 2; break;
    case NetworkAdapter::NB:       *typeID = 4; *paramsNum = 2; break;
    case NetworkAdapter::CATM1:    *typeID = 5; *paramsNum = 2; break;
    case NetworkAdapter::ETHERNET: *typeID = 6; *paramsNum = 4; break;
    case NetworkAdapter::CELL:     *typeID = 7; *paramsNum = 2; break;
    case NetworkAdapter::NOTECARD: *typeID = 8; *paramsNum = 0; break;
    default:                       *typeID = 0; *paramsNum = 0; break;
  }
}

#if defined(BOARD_HAS_WIFI)
MessageEncoder::Status DeviceNetConfigCmdUpEncoder::encodeWiFiNetwork(CborEncoder *encoder, models::WiFiSetting *config) {

  if(cbor_encode_text_stringz(encoder, config->ssid) != CborNoError) {
    return MessageEncoder::Status::Error;
  }

  return MessageEncoder::Status::Complete;
}
#endif // defined(BOARD_HAS_WIFI)

#if defined(BOARD_HAS_CATM1_NBIOT)
MessageEncoder::Status DeviceNetConfigCmdUpEncoder::encodeCatM1Network(CborEncoder *encoder, models::CATM1Setting *config)
{
  if(cbor_encode_text_stringz(encoder, config->apn) != CborNoError) {
    return MessageEncoder::Status::Error;
  }

  if(cbor_encode_text_stringz(encoder, config->login) != CborNoError) {
    return MessageEncoder::Status::Error;
  }

  return MessageEncoder::Status::Complete;
}
#endif // defined(BOARD_HAS_CATM1_NBIOT)

#if defined(BOARD_HAS_ETHERNET)
MessageEncoder::Status DeviceNetConfigCmdUpEncoder::encodeEthernetNetwork(CborEncoder *encoder, models::EthernetSetting *config) {

  if(encodeIP(encoder, &config->ip) != MessageEncoder::Status::Complete) {
    return MessageEncoder::Status::Error;
  }

  if(encodeIP(encoder, &config->dns) != MessageEncoder::Status::Complete) {
    return MessageEncoder::Status::Error;
  }

  if(encodeIP(encoder, &config->gateway) != MessageEncoder::Status::Complete) {
    return MessageEncoder::Status::Error;
  }

  if(encodeIP(encoder, &config->netmask) != MessageEncoder::Status::Complete) {
    return MessageEncoder::Status::Error;
  }

  return MessageEncoder::Status::Complete;
}

MessageEncoder::Status DeviceNetConfigCmdUpEncoder::encodeIP(CborEncoder *encoder, const models::ip_addr *ip)
{
  uint8_t ip_len = 0;
  uint8_t emptyIP[16];
  memset(emptyIP, 0, sizeof(emptyIP));
  // Check if the IP is empty, DHCP case
  if(memcmp(ip->bytes, emptyIP, sizeof(emptyIP)) == 0) {
    ip_len = 0;
  } else if(ip->type == IPType::IPv4) {
    ip_len = 4;
  } else if(ip->type == IPType::IPv6) {
    ip_len = 16;
  }

  if(cbor_encode_byte_string(encoder, ip->bytes, ip_len) != CborNoError) {
    return MessageEncoder::Status::Error;
  }

  return MessageEncoder::Status::Complete;
}
#endif // defined(BOARD_HAS_ETHERNET)

#if defined(BOARD_HAS_NB) || defined(BOARD_HAS_GSM) || defined(BOARD_HAS_CELLULAR)
MessageEncoder::Status DeviceNetConfigCmdUpEncoder::encodeCellularNetwork(CborEncoder *encoder, models::CellularSetting *config) {
  if(cbor_encode_text_stringz(encoder, config->apn) != CborNoError) {
    return MessageEncoder::Status::Error;
  }

  if(cbor_encode_text_stringz(encoder, config->login) != CborNoError) {
    return MessageEncoder::Status::Error;
  }

  return MessageEncoder::Status::Complete;
}
#endif // defined(BOARD_HAS_NB) || defined(BOARD_HAS_GSM) ||defined(BOARD_HAS_CELLULAR)

#if defined(BOARD_HAS_LORA)
MessageEncoder::Status DeviceNetConfigCmdUpEncoder::encodeLoRaNetwork(CborEncoder *encoder, models::LoraSetting *config) {
  if(cbor_encode_text_stringz(encoder, config->appeui) != CborNoError) {
    return MessageEncoder::Status::Error;
  }

  return MessageEncoder::Status::Complete;
}
#endif // defined(BOARD_HAS_LORA)

static OtaBeginCommandEncoder         otaBeginCommandEncoder;
static ThingBeginCommandEncoder       thingBeginCommandEncoder;
static LastValuesBeginCommandEncoder  lastValuesBeginCommandEncoder;
static DeviceBeginCommandEncoder      deviceBeginCommandEncoder;
static OtaProgressCommandUpEncoder    otaProgressCommandUpEncoder;
static TimezoneCommandUpEncoder       timezoneCommandUpEncoder;
static DeviceNetConfigCmdUpEncoder    deviceNetConfigCmdUpEncoder;

namespace cbor { namespace encoder { namespace iotcloud {
  void commandEncoders() {
    (void) otaBeginCommandEncoder;
    (void) thingBeginCommandEncoder;
    (void) lastValuesBeginCommandEncoder;
    (void) deviceBeginCommandEncoder;
    (void) otaProgressCommandUpEncoder;
    (void) timezoneCommandUpEncoder;
    (void) deviceNetConfigCmdUpEncoder;
  }
}}}
