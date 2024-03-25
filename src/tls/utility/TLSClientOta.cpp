/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <AIoTC_Config.h>

#if defined(HAS_TCP) && OTA_ENABLED

#include "TLSClientOta.h"

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

void TLSClientOta::begin(ConnectionHandler &connection) {
#if defined(BOARD_HAS_OFFLOADED_ECCX08)
  /* AWS Root CAs are configured in nina-fw
   * https://github.com/arduino/nina-fw/blob/master/data/roots.pem
   */
#elif defined(BOARD_HAS_ECCX08)
  setClient(*getNewClient(connection.getInterface()));
  setProfile(aiotc_client_profile_init);
  setTrustAnchors(ArduinoIoTCloudTrustAnchor, ArduinoIoTCloudTrustAnchor_NUM);
  onGetTime(getTime);
#elif defined(ARDUINO_PORTENTA_C33)
  setClient(*getNewClient(connection.getInterface()));
  setCACert(AIoTSSCert);
#elif defined(ARDUINO_NICLA_VISION)
  appendCustomCACert(AIoTSSCert);
#elif defined(ARDUINO_EDGE_CONTROL)
  appendCustomCACert(AIoTUPCert);
#elif defined(ARDUINO_UNOR4_WIFI)
  /* AWS Root CAs are configured in uno-r4-wifi-usb-bridge/libraries/Arduino_ESP32_OTA
   * https://github.com/arduino-libraries/Arduino_ESP32_OTA/blob/fc755e7d1d3946232107e2590662ee08d6ccdec4/src/tls/amazon_root_ca.h
   */
#elif defined(ARDUINO_ARCH_ESP32)
  setCACertBundle(x509_crt_bundle);
#elif defined(ARDUINO_ARCH_ESP8266)
  setInsecure();
#endif
}

#endif
