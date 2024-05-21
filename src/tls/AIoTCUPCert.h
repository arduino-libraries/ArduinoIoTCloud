/*
   This file is part of ArduinoIoTBearSSL.

   Copyright 2019 ARDUINO SA (http://www.arduino.cc/)

   This software is released under the GNU General Public License version 3,
   which covers the main part of ArduinoIoTBearSSL.
   The terms of this license can be found at:
   https://www.gnu.org/licenses/gpl-3.0.en.html

   You can be released from the requirements of the above licenses by purchasing
   a commercial license. Buying such a license is mandatory if you want to modify or
   otherwise use the software for commercial activities involving the Arduino
   software without disclosing the source code of your own applications. To purchase
   a commercial license, send an email to license@arduino.cc.

*/

#ifndef _AIOTC_UP_CERT_H_
#define _AIOTC_UP_CERT_H_

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <AIoTC_Config.h>
#if defined(ARDUINO_ARCH_ESP32)

/******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* This certificate bundle is created using the following certificates:

   * https://www.amazontrust.com/repository/AmazonRootCA1.pem
   * https://www.amazontrust.com/repository/AmazonRootCA2.pem
   * https://www.amazontrust.com/repository/AmazonRootCA3.pem
   * https://www.amazontrust.com/repository/AmazonRootCA4.pem
   * https://www.amazontrust.com/repository/SFSRootCAG2.pem
   * https://certs.secureserver.net/repository/sf-class2-root.crt
   * https://iot.arduino.cc

 */

extern const unsigned char x509_crt_bundle[];

#elif defined (ARDUINO_EDGE_CONTROL)
  /*
   * https://www.amazontrust.com/repository/AmazonRootCA1.pem
   * https://www.amazontrust.com/repository/AmazonRootCA2.pem
   * https://www.amazontrust.com/repository/AmazonRootCA3.pem
   * https://www.amazontrust.com/repository/AmazonRootCA4.pem
   * https://www.amazontrust.com/repository/SFSRootCAG2.pem
   */
extern const char AIoTUPCert[];
#else

#endif /* #ifdef ARDUINO_ARCH_ESP32 */

#endif /* _AIOTC_UP_CERT_H_ */
