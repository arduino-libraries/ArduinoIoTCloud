/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef ARDUINO_BEARSSL_CONFIG_H_
#define ARDUINO_BEARSSL_CONFIG_H_

/* Enabling this define allows the usage of ArduinoBearSSL without crypto chip. */
//#define ARDUINO_DISABLE_ECCX08

/* Enable/Disable global instances*/
#define ARDUINO_BEARSSL_DISABLE_AES128
#define ARDUINO_BEARSSL_DISABLE_DES
#define ARDUINO_BEARSSL_DISABLE_MD5
#define ARDUINO_BEARSSL_DISABLE_SHA1
#define ARDUINO_BEARSSL_DISABLE_SHA256

#define ARDUINO_BEARSSL_DISABLE_KEY_DECODER

/* If uncommented profile should be configured using client.setProfile(...) */
//#define ARDUINO_BEARSSL_DISABLE_FULL_CLIENT_PROFILE

/* If uncommented TA should be configured via constructor */
//#define ARDUINO_BEARSSL_DISABLE_BUILTIN_TRUST_ANCHORS

/* If uncommented disables br_sslio_close call.From BearSSL docs:
 *
 * br_sslio_close(): perform the SSL closure protocol. This entails sending a
 * close_notify alert, and receiving a close_notify response.
 *
 * Note that a number of deployed SSL implementations do not follow the protocol
 * for closure, and may drop the underlying socket abruptly. As such, errors are
 * often reported by br_sslio_close().
 *
 * In case of mbed-os + ArduinoIoTCloud br_sslio_close is endless looping
 * blocking sketch execution.
 */
#define ARDUINO_BEARSSL_DISABLE_TLS_CLOSE

#define BEAR_SSL_CLIENT_CHAIN_SIZE 1

#if defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_NICLA_VISION) || defined(ARDUINO_OPTA) ||\
    defined(ARDUINO_GIGA) || defined(ARDUINO_NANO_RP2040_CONNECT)
  /* Allows download from OTA storage API */
  #define BEAR_SSL_CLIENT_IBUF_SIZE (16384 + 325)
#endif

#endif /* ARDUINO_BEARSSL_CONFIG_H_ */
