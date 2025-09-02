/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2019 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef _AIOTC_SS_CERT_H_
#define _AIOTC_SS_CERT_H_

/******************************************************************************
  INCLUDE
 ******************************************************************************/

#include <AIoTC_Config.h>
#if defined(BOARD_HAS_SE050) || defined(BOARD_HAS_SOFTSE)

/******************************************************************************
  CONSTANTS
 ******************************************************************************/
static const char AIoTSSCert[] =
/* https://iot.arduino.cc:8885 */
"-----BEGIN CERTIFICATE-----\n"
"MIIB0DCCAXagAwIBAgIUb62eK/Vv1baaPAaY5DADBUbxB1owCgYIKoZIzj0EAwIw\n"
"RTELMAkGA1UEBhMCVVMxFzAVBgNVBAoTDkFyZHVpbm8gTExDIFVTMQswCQYDVQQL\n"
"EwJJVDEQMA4GA1UEAxMHQXJkdWlubzAgFw0yNTAxMTAxMDUzMjJaGA8yMDU1MDEw\n"
"MzEwNTMyMlowRTELMAkGA1UEBhMCVVMxFzAVBgNVBAoTDkFyZHVpbm8gTExDIFVT\n"
"MQswCQYDVQQLEwJJVDEQMA4GA1UEAxMHQXJkdWlubzBZMBMGByqGSM49AgEGCCqG\n"
"SM49AwEHA0IABKHhU2w1UhozDegrrFsSwY9QN7M+ZJug7icCNceNWhBF0Mr1UuyX\n"
"8pr/gcbieZc/0znG16HMa2GFcPY7rmIdccijQjBAMA8GA1UdEwEB/wQFMAMBAf8w\n"
"DgYDVR0PAQH/BAQDAgEGMB0GA1UdDgQWBBRCZSmE0ASI0cYD9AmzeOM7EijgPjAK\n"
"BggqhkjOPQQDAgNIADBFAiEAz6TLYP9eiVOr/cVU/11zwGofe/FoNe4p1BlzMl7G\n"
"VVACIG8tL3Ta2WbIOaUVpBL2gfLuI9WSW1sR++zXP+zFhmen\n"
"-----END CERTIFICATE-----\n"
/* https://iot.oniudra.cc:8885 */
"-----BEGIN CERTIFICATE-----\n"
"MIIBzzCCAXagAwIBAgIUI5fEitwlnwujc/mU0d8LnDiDXBIwCgYIKoZIzj0EAwIw\n"
"RTELMAkGA1UEBhMCVVMxFzAVBgNVBAoTDkFyZHVpbm8gTExDIFVTMQswCQYDVQQL\n"
"EwJJVDEQMA4GA1UEAxMHQXJkdWlubzAgFw0yNTAxMDgxMTA4MzdaGA8yMDU1MDEw\n"
"MTExMDgzN1owRTELMAkGA1UEBhMCVVMxFzAVBgNVBAoTDkFyZHVpbm8gTExDIFVT\n"
"MQswCQYDVQQLEwJJVDEQMA4GA1UEAxMHQXJkdWlubzBZMBMGByqGSM49AgEGCCqG\n"
"SM49AwEHA0IABBFwNODDPgC9C1kDmKBbawtQ31FmTudAXVpGSOUwcDX582z820cD\n"
"eIaCwOxghmI+p/CpOH63f5F6h23ErqZMBkijQjBAMA8GA1UdEwEB/wQFMAMBAf8w\n"
"DgYDVR0PAQH/BAQDAgEGMB0GA1UdDgQWBBQdnBmQGLB7ls/r1Tetdp+MVMqxfTAK\n"
"BggqhkjOPQQDAgNHADBEAiBPSZ9HpF7MuFoK4Jsz//PHILQuHM4WmRopQR9ysSs0\n"
"HAIgNadMPgxv01dy59kCgzehgKzmKdTF0rG1SniYqnkLqPA=\n"
"-----END CERTIFICATE-----\n";

#endif /* #if defined(BOARD_HAS_SE050) || defined(BOARD_HAS_SOFTSE) */

#endif /* _AIOTC_SS_CERT_H_ */
