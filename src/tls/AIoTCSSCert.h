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

#ifndef _AIOTC_SS_CERT_H_
#define _AIOTC_SS_CERT_H_

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <AIoTC_Config.h>
#if defined(BOARD_HAS_SE050) || defined(BOARD_HAS_SOFTSE)

/******************************************************************************
 * CONSTANTS
 ******************************************************************************/
static const char AIoTSSCert[] =
/* https://iot.arduino.cc:8883 */
"-----BEGIN CERTIFICATE-----\n"
"MIIBzzCCAXSgAwIBAgIUHxAd66fhJecnwaOR4+wNF03tSlkwCgYIKoZIzj0EAwIw\n"
"RTELMAkGA1UEBhMCVVMxFzAVBgNVBAoTDkFyZHVpbm8gTExDIFVTMQswCQYDVQQL\n"
"EwJJVDEQMA4GA1UEAxMHQXJkdWlubzAeFw0xODA3MjQwOTQ3MDBaFw00ODA3MTYw\n"
"OTQ3MDBaMEUxCzAJBgNVBAYTAlVTMRcwFQYDVQQKEw5BcmR1aW5vIExMQyBVUzEL\n"
"MAkGA1UECxMCSVQxEDAOBgNVBAMTB0FyZHVpbm8wWTATBgcqhkjOPQIBBggqhkjO\n"
"PQMBBwNCAARtd2xaz2EcfUSYUfJe4QJAd7ecvUmio4xOq16YrIL8aVtEIne0TS6O\n"
"3ypxwTls1jkUvdlrGEtL7LPV7kKJiVUio0IwQDAOBgNVHQ8BAf8EBAMCAQYwDwYD\n"
"VR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUWz4qa47JsBqoVOY2m4wJ+fzhuYAwCgYI\n"
"KoZIzj0EAwIDSQAwRgIhAL/T3CNmaLUK3D8NDsNz4grH92CqEA3TIL/hApabawXY\n"
"AiEA6tnZ2lrNElKXCajtZg/hjWRE/+giFzBP8riar8qOz2w=\n"
"-----END CERTIFICATE-----\n"
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
