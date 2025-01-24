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
#if defined(ARDUINO_ARCH_ESP32) ||  defined (ARDUINO_EDGE_CONTROL)

/******************************************************************************
 * CONSTANTS
 ******************************************************************************/
static const char AIoTUPCert[] =
/* https://www.amazontrust.com/repository/AmazonRootCA1.pem */
"-----BEGIN CERTIFICATE-----\n"
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n"
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"
"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n"
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n"
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n"
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n"
"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n"
"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n"
"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n"
"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n"
"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n"
"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n"
"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n"
"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n"
"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n"
"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n"
"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n"
"rqXRfboQnoZsG4q5WTP468SQvvG5\n"
"-----END CERTIFICATE-----\n"
/* https://www.amazontrust.com/repository/AmazonRootCA2.pem */
"-----BEGIN CERTIFICATE-----\n"
"MIIFQTCCAymgAwIBAgITBmyf0pY1hp8KD+WGePhbJruKNzANBgkqhkiG9w0BAQwF\n"
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"
"b24gUm9vdCBDQSAyMB4XDTE1MDUyNjAwMDAwMFoXDTQwMDUyNjAwMDAwMFowOTEL\n"
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n"
"b3QgQ0EgMjCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK2Wny2cSkxK\n"
"gXlRmeyKy2tgURO8TW0G/LAIjd0ZEGrHJgw12MBvIITplLGbhQPDW9tK6Mj4kHbZ\n"
"W0/jTOgGNk3Mmqw9DJArktQGGWCsN0R5hYGCrVo34A3MnaZMUnbqQ523BNFQ9lXg\n"
"1dKmSYXpN+nKfq5clU1Imj+uIFptiJXZNLhSGkOQsL9sBbm2eLfq0OQ6PBJTYv9K\n"
"8nu+NQWpEjTj82R0Yiw9AElaKP4yRLuH3WUnAnE72kr3H9rN9yFVkE8P7K6C4Z9r\n"
"2UXTu/Bfh+08LDmG2j/e7HJV63mjrdvdfLC6HM783k81ds8P+HgfajZRRidhW+me\n"
"z/CiVX18JYpvL7TFz4QuK/0NURBs+18bvBt+xa47mAExkv8LV/SasrlX6avvDXbR\n"
"8O70zoan4G7ptGmh32n2M8ZpLpcTnqWHsFcQgTfJU7O7f/aS0ZzQGPSSbtqDT6Zj\n"
"mUyl+17vIWR6IF9sZIUVyzfpYgwLKhbcAS4y2j5L9Z469hdAlO+ekQiG+r5jqFoz\n"
"7Mt0Q5X5bGlSNscpb/xVA1wf+5+9R+vnSUeVC06JIglJ4PVhHvG/LopyboBZ/1c6\n"
"+XUyo05f7O0oYtlNc/LMgRdg7c3r3NunysV+Ar3yVAhU/bQtCSwXVEqY0VThUWcI\n"
"0u1ufm8/0i2BWSlmy5A5lREedCf+3euvAgMBAAGjQjBAMA8GA1UdEwEB/wQFMAMB\n"
"Af8wDgYDVR0PAQH/BAQDAgGGMB0GA1UdDgQWBBSwDPBMMPQFWAJI/TPlUq9LhONm\n"
"UjANBgkqhkiG9w0BAQwFAAOCAgEAqqiAjw54o+Ci1M3m9Zh6O+oAA7CXDpO8Wqj2\n"
"LIxyh6mx/H9z/WNxeKWHWc8w4Q0QshNabYL1auaAn6AFC2jkR2vHat+2/XcycuUY\n"
"+gn0oJMsXdKMdYV2ZZAMA3m3MSNjrXiDCYZohMr/+c8mmpJ5581LxedhpxfL86kS\n"
"k5Nrp+gvU5LEYFiwzAJRGFuFjWJZY7attN6a+yb3ACfAXVU3dJnJUH/jWS5E4ywl\n"
"7uxMMne0nxrpS10gxdr9HIcWxkPo1LsmmkVwXqkLN1PiRnsn/eBG8om3zEK2yygm\n"
"btmlyTrIQRNg91CMFa6ybRoVGld45pIq2WWQgj9sAq+uEjonljYE1x2igGOpm/Hl\n"
"urR8FLBOybEfdF849lHqm/osohHUqS0nGkWxr7JOcQ3AWEbWaQbLU8uz/mtBzUF+\n"
"fUwPfHJ5elnNXkoOrJupmHN5fLT0zLm4BwyydFy4x2+IoZCn9Kr5v2c69BoVYh63\n"
"n749sSmvZ6ES8lgQGVMDMBu4Gon2nL2XA46jCfMdiyHxtN/kHNGfZQIG6lzWE7OE\n"
"76KlXIx3KadowGuuQNKotOrN8I1LOJwZmhsoVLiJkO/KdYE+HvJkJMcYr07/R54H\n"
"9jVlpNMKVv/1F2Rs76giJUmTtt8AF9pYfl3uxRuw0dFfIRDH+fO6AgonB8Xx1sfT\n"
"4PsJYGw=\n"
"-----END CERTIFICATE-----\n"
/* https://www.amazontrust.com/repository/AmazonRootCA3.pem */
"-----BEGIN CERTIFICATE-----\n"
"MIIBtjCCAVugAwIBAgITBmyf1XSXNmY/Owua2eiedgPySjAKBggqhkjOPQQDAjA5\n"
"MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6b24g\n"
"Um9vdCBDQSAzMB4XDTE1MDUyNjAwMDAwMFoXDTQwMDUyNjAwMDAwMFowOTELMAkG\n"
"A1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJvb3Qg\n"
"Q0EgMzBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABCmXp8ZBf8ANm+gBG1bG8lKl\n"
"ui2yEujSLtf6ycXYqm0fc4E7O5hrOXwzpcVOho6AF2hiRVd9RFgdszflZwjrZt6j\n"
"QjBAMA8GA1UdEwEB/wQFMAMBAf8wDgYDVR0PAQH/BAQDAgGGMB0GA1UdDgQWBBSr\n"
"ttvXBp43rDCGB5Fwx5zEGbF4wDAKBggqhkjOPQQDAgNJADBGAiEA4IWSoxe3jfkr\n"
"BqWTrBqYaGFy+uGh0PsceGCmQ5nFuMQCIQCcAu/xlJyzlvnrxir4tiz+OpAUFteM\n"
"YyRIHN8wfdVoOw==\n"
"-----END CERTIFICATE-----\n"
/* https://www.amazontrust.com/repository/AmazonRootCA4.pem */
"-----BEGIN CERTIFICATE-----\n"
"MIIB8jCCAXigAwIBAgITBmyf18G7EEwpQ+Vxe3ssyBrBDjAKBggqhkjOPQQDAzA5\n"
"MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6b24g\n"
"Um9vdCBDQSA0MB4XDTE1MDUyNjAwMDAwMFoXDTQwMDUyNjAwMDAwMFowOTELMAkG\n"
"A1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJvb3Qg\n"
"Q0EgNDB2MBAGByqGSM49AgEGBSuBBAAiA2IABNKrijdPo1MN/sGKe0uoe0ZLY7Bi\n"
"9i0b2whxIdIA6GO9mif78DluXeo9pcmBqqNbIJhFXRbb/egQbeOc4OO9X4Ri83Bk\n"
"M6DLJC9wuoihKqB1+IGuYgbEgds5bimwHvouXKNCMEAwDwYDVR0TAQH/BAUwAwEB\n"
"/zAOBgNVHQ8BAf8EBAMCAYYwHQYDVR0OBBYEFNPsxzplbszh2naaVvuc84ZtV+WB\n"
"MAoGCCqGSM49BAMDA2gAMGUCMDqLIfG9fhGt0O9Yli/W651+kI0rz2ZVwyzjKKlw\n"
"CkcO8DdZEv8tmZQoTipPNU0zWgIxAOp1AE47xDqUEpHJWEadIRNyp4iciuRMStuW\n"
"1KyLa2tJElMzrdfkviT8tQp21KW8EA==\n"
"-----END CERTIFICATE-----\n"
/* https://www.amazontrust.com/repository/SFSRootCAG2.pem */
"-----BEGIN CERTIFICATE-----\n"
"MIID7zCCAtegAwIBAgIBADANBgkqhkiG9w0BAQsFADCBmDELMAkGA1UEBhMCVVMx\n"
"EDAOBgNVBAgTB0FyaXpvbmExEzARBgNVBAcTClNjb3R0c2RhbGUxJTAjBgNVBAoT\n"
"HFN0YXJmaWVsZCBUZWNobm9sb2dpZXMsIEluYy4xOzA5BgNVBAMTMlN0YXJmaWVs\n"
"ZCBTZXJ2aWNlcyBSb290IENlcnRpZmljYXRlIEF1dGhvcml0eSAtIEcyMB4XDTA5\n"
"MDkwMTAwMDAwMFoXDTM3MTIzMTIzNTk1OVowgZgxCzAJBgNVBAYTAlVTMRAwDgYD\n"
"VQQIEwdBcml6b25hMRMwEQYDVQQHEwpTY290dHNkYWxlMSUwIwYDVQQKExxTdGFy\n"
"ZmllbGQgVGVjaG5vbG9naWVzLCBJbmMuMTswOQYDVQQDEzJTdGFyZmllbGQgU2Vy\n"
"dmljZXMgUm9vdCBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkgLSBHMjCCASIwDQYJKoZI\n"
"hvcNAQEBBQADggEPADCCAQoCggEBANUMOsQq+U7i9b4Zl1+OiFOxHz/Lz58gE20p\n"
"OsgPfTz3a3Y4Y9k2YKibXlwAgLIvWX/2h/klQ4bnaRtSmpDhcePYLQ1Ob/bISdm2\n"
"8xpWriu2dBTrz/sm4xq6HZYuajtYlIlHVv8loJNwU4PahHQUw2eeBGg6345AWh1K\n"
"Ts9DkTvnVtYAcMtS7nt9rjrnvDH5RfbCYM8TWQIrgMw0R9+53pBlbQLPLJGmpufe\n"
"hRhJfGZOozptqbXuNC66DQO4M99H67FrjSXZm86B0UVGMpZwh94CDklDhbZsc7tk\n"
"6mFBrMnUVN+HL8cisibMn1lUaJ/8viovxFUcdUBgF4UCVTmLfwUCAwEAAaNCMEAw\n"
"DwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAQYwHQYDVR0OBBYEFJxfAN+q\n"
"AdcwKziIorhtSpzyEZGDMA0GCSqGSIb3DQEBCwUAA4IBAQBLNqaEd2ndOxmfZyMI\n"
"bw5hyf2E3F/YNoHN2BtBLZ9g3ccaaNnRbobhiCPPE95Dz+I0swSdHynVv/heyNXB\n"
"ve6SbzJ08pGCL72CQnqtKrcgfU28elUSwhXqvfdqlS5sdJ/PHLTyxQGjhdByPq1z\n"
"qwubdQxtRbeOlKyWN7Wg0I8VRw7j6IPdj/3vQQF3zCepYoUz8jcI73HPdwbeyBkd\n"
"iEDPfUYd/x7H4c7/I9vG+o1VTqkC50cRRj70/b17KSa7qWFiNyi2LSr2EIZkyXCn\n"
"0q23KXB56jzaYyWf/Wi3MOxw+3WKt21gZ7IeyLnp2KhvAotnDU0mV3HaIPzBSlCN\n"
"sSi6\n"
"-----END CERTIFICATE-----\n"
/* iot.arduino.cc:8883 / iot.oniudra.cc:8883 */
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
"-----END CERTIFICATE--------\n"
/* iot.arduino.cc:8885 */
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
/* iot.oniudra.cc:8885 */
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
#else

#endif /* #ifdef ARDUINO_ARCH_ESP32 */

#endif /* _AIOTC_UP_CERT_H_ */
