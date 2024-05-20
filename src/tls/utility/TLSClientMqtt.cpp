/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <AIoTC_Config.h>

#ifdef HAS_TCP

#include "TLSClientMqtt.h"

#if defined(BOARD_HAS_SECRET_KEY)
  #include "tls/AIoTCUPCert.h"
#endif

#if defined(BOARD_HAS_SE050) || defined(BOARD_HAS_SOFTSE)
  #include "tls/AIoTCSSCert.h"
#endif

#ifdef BOARD_HAS_ECCX08
  #include "tls/BearSSLTrustAnchors.h"
  extern "C" {
  void aiotc_client_profile_init(br_ssl_client_context *cc,
                                 br_x509_minimal_context *xc,
                                 const br_x509_trust_anchor *trust_anchors,
                                 size_t trust_anchors_num);
  unsigned long getTime();
  }
#endif

void TLSClientMqtt::begin(ConnectionHandler & connection) {

#if defined(BOARD_HAS_OFFLOADED_ECCX08)
  /* Arduino Root CA is configured in nina-fw
   * https://github.com/arduino/nina-fw/blob/master/arduino/libraries/ArduinoBearSSL/src/BearSSLTrustAnchors.h
   */
#elif defined(BOARD_HAS_ECCX08)
  setClient(connection.getClient());
  setProfile(aiotc_client_profile_init);
  setTrustAnchors(ArduinoIoTCloudTrustAnchor, ArduinoIoTCloudTrustAnchor_NUM);
  onGetTime(getTime);
#elif defined(ARDUINO_PORTENTA_C33)
  setClient(connection.getClient());
  setCACert(AIoTSSCert);
#elif defined(ARDUINO_NICLA_VISION)
  appendCustomCACert(AIoTSSCert);
#elif defined(ARDUINO_EDGE_CONTROL)
  appendCustomCACert(AIoTUPCert);
#elif defined(ARDUINO_UNOR4_WIFI)
  /* Arduino Root CA is configured in uno-r4-wifi-usb-bridge fw >= 0.4.1
   * https://github.com/arduino/uno-r4-wifi-usb-bridge/blob/main/certificates/cacrt_all.pem
   * Boards using username/password authentication relies on Starfield Class 2 CA
   * also present in older firmware revisions
   * https://github.com/arduino/uno-r4-wifi-usb-bridge/blob/f09ca94fdcab845b8368d4435fdac9f6999d21d2/certificates/certificates.pem#L852
   */
#elif defined(ARDUINO_ARCH_ESP32)
  setCACertBundle(x509_crt_bundle);
#elif defined(ARDUINO_ARCH_ESP8266)
  setInsecure();
#endif
}

#endif
