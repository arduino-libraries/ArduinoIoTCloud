/*
   This file is part of ArduinoIoTCloud.

   Copyright 2024 ARDUINO SA (http://www.arduino.cc/)

   This software is released under the GNU General Public License version 3,
   which covers the main part of arduino-cli.
   The terms of this license can be found at:
   https://www.gnu.org/licenses/gpl-3.0.en.html

   You can be released from the requirements of the above licenses by purchasing
   a commercial license. Buying such a license is mandatory if you want to modify or
   otherwise use the software for commercial activities involving the Arduino
   software without disclosing the source code of your own applications. To purchase
   a commercial license, send an email to license@arduino.cc.
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
