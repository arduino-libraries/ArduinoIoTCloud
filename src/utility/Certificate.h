/*
   This file is part of ArduinoIoTCloud.

   Copyright 2019 ARDUINO SA (http://www.arduino.cc/)

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

#ifndef CERTIFICATE_H_
#define CERTIFICATE_H_

#include <ArduinoIoTCloud_Defines.h>

#ifndef BOARD_HAS_ECCX08

  static const char MQTTS_UP_ARDUINO_CC_CERTIFICATE[] = \
  "-----BEGIN CERTIFICATE-----\n" \
  "MIIFqTCCBJGgAwIBAgIQAkXDTIa5iG2fCGE2cU98EzANBgkqhkiG9w0BAQsFADBG\n" \
  "MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRUwEwYDVQQLEwxTZXJ2ZXIg\n" \
  "Q0EgMUIxDzANBgNVBAMTBkFtYXpvbjAeFw0xOTA4MjEwMDAwMDBaFw0yMDA5MjEx\n" \
  "MjAwMDBaMBkxFzAVBgNVBAMTDmlvdC5hcmR1aW5vLmNjMIIBIjANBgkqhkiG9w0B\n" \
  "AQEFAAOCAQ8AMIIBCgKCAQEArhKKVxZV7yTDkA6J953mYaEMbz0c4PIq+XRUkNbZ\n" \
  "cbUg9nwew+Lg20DvvtPTavufS+i0zM4wDAxCgxGCUUaRHxFCg1zzsxHx+hsc0l92\n" \
  "CFegNzgbV+DC99beoStEmuNgAcEX+3RKfXleb5/GKjML9fV3ms3QDdw9DUatSstL\n" \
  "TJm6rIEmscXduvcPnPktYLEzzSKSd5GxBVdKbqwiGbPkqkXS8KsScwQ0+iqIAu6h\n" \
  "30eHPcA/BhM/nSbl4IPFjOgLv/aZl3hv7tG48RJCzGprA7EoO4tvNjdh16f4I4xI\n" \
  "68ESoKi8+BQMmnXTywuZUsFR2+U5dFajWZGjw7ZNwBbcCwIDAQABo4ICvjCCArow\n" \
  "HwYDVR0jBBgwFoAUWaRmBlKge5WSPKOUByeWdFv5PdAwHQYDVR0OBBYEFK+X+jBL\n" \
  "q0qtnorOxvgH+fozNswOMF8GA1UdEQRYMFaCDmlvdC5hcmR1aW5vLmNjghdtcXR0\n" \
  "cy1zYS5pb3QuYXJkdWluby5jY4IXbXF0dHMtdXAuaW90LmFyZHVpbm8uY2OCEndz\n" \
  "cy5pb3QuYXJkdWluby5jYzAOBgNVHQ8BAf8EBAMCBaAwHQYDVR0lBBYwFAYIKwYB\n" \
  "BQUHAwEGCCsGAQUFBwMCMDsGA1UdHwQ0MDIwMKAuoCyGKmh0dHA6Ly9jcmwuc2Nh\n" \
  "MWIuYW1hem9udHJ1c3QuY29tL3NjYTFiLmNybDAgBgNVHSAEGTAXMAsGCWCGSAGG\n" \
  "/WwBAjAIBgZngQwBAgEwdQYIKwYBBQUHAQEEaTBnMC0GCCsGAQUFBzABhiFodHRw\n" \
  "Oi8vb2NzcC5zY2ExYi5hbWF6b250cnVzdC5jb20wNgYIKwYBBQUHMAKGKmh0dHA6\n" \
  "Ly9jcnQuc2NhMWIuYW1hem9udHJ1c3QuY29tL3NjYTFiLmNydDAMBgNVHRMBAf8E\n" \
  "AjAAMIIBAgYKKwYBBAHWeQIEAgSB8wSB8ADuAHUA7ku9t3XOYLrhQmkfq+GeZqMP\n" \
  "fl+wctiDAMR7iXqo/csAAAFsscrkQwAABAMARjBEAiBDpHC99dp3Ugl2VPhZvFw4\n" \
  "Xspn+spYtrrL9D5qBibEGwIgN9Pd8AOT9Vq2WdG5ndL6eF/MTHN+4tQVcbH+6S3S\n" \
  "Y44AdQCHdb/nWXz4jEOZX73zbv9WjUdWNv9KtWDBtOr/XqCDDwAAAWyxyuSRAAAE\n" \
  "AwBGMEQCIHPwjUphiC5eogbUg5WZhjMQp7ri1XnjKKp+RVpIGIUzAiAdo2DeKjxz\n" \
  "Wrhsi9iU/QLwbzhtw3TymEmfQf5pO0mY5TANBgkqhkiG9w0BAQsFAAOCAQEACEfs\n" \
  "QvYDXjhb0GY3PIyzzoZPgdyoKk3MCzGyaMGceCiwy5+dJAratt6oozIjM3yuQhm6\n" \
  "zG5RvRBMLGJb01500Y1sPETrXEMiqa7ZCqG5++XL2vfiLohfxvx2RC9gSAj8fEp+\n" \
  "D1LqP4reEXbWxjEwBQ2rcphFn4UntQP+1EArzBrAvYmDsDw3a2QQC7Wsj6KWh25i\n" \
  "CeNXAupLO9/20BQ+QcZv3wPRwnpDgv6qtM0v1tbLWdhWCF4Qg/+tH238sHwuiwTQ\n" \
  "+bwoDjrEzOKlzU4qgjLPeZzkNgSVuq9PPScNb/RothuPkH3dPBwM/AWk5L8rV+YL\n" \
  "3QwJMcxcVmrJV1OHog==\n" \
  "-----END CERTIFICATE-----\n";

#endif /* BOARD_HAS_ECCX08 */

#endif /* CERTIFICATE_H_ */
