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
  "MIIFqzCCBJOgAwIBAgIQBG1R1P7yNtWlrHgjD6LbzDANBgkqhkiG9w0BAQsFADBG\n" \
  "MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRUwEwYDVQQLEwxTZXJ2ZXIg\n" \
  "Q0EgMUIxDzANBgNVBAMTBkFtYXpvbjAeFw0xODA5MTkwMDAwMDBaFw0xOTEwMTkx\n" \
  "MjAwMDBaMBkxFzAVBgNVBAMTDmlvdC5hcmR1aW5vLmNjMIIBIjANBgkqhkiG9w0B\n" \
  "AQEFAAOCAQ8AMIIBCgKCAQEApwMZB0pTN46+JtOBf5+DOpyahz+u72wDAklqYckj\n" \
  "Rk99acA/QbGA/xz/fAhQCPjsJ6yGDuUzExhVPW5hv4qUFhWL7U7sWxMowlM8e+hB\n" \
  "M00XpihgCgLy+uuODvkSMa7UcuhSXlO2z5+nU0ug1GRaEqpL8pRiXU8uxbwVX0oq\n" \
  "11QjW9ZiW8LtSI1w5GvIi3e7cKo4S+txVJGCiOzcGkk5GoskkaL57S4W6sWv43l7\n" \
  "dqSiAmNeeq0nqxPg0djIjvEbGL0taSDFGkLO79DlKIETj+ctmdymYb1XnTOPg8PE\n" \
  "3QSwg/8DVCW3CvvYwO4NR73dfFGnLMZcMvgxoneaEVS3swIDAQABo4ICwDCCArww\n" \
  "HwYDVR0jBBgwFoAUWaRmBlKge5WSPKOUByeWdFv5PdAwHQYDVR0OBBYEFLA93zdQ\n" \
  "93oUeTo6J/m2gZapVZ0UMF8GA1UdEQRYMFaCDmlvdC5hcmR1aW5vLmNjghdtcXR0\n" \
  "cy1zYS5pb3QuYXJkdWluby5jY4IXbXF0dHMtdXAuaW90LmFyZHVpbm8uY2OCEndz\n" \
  "cy5pb3QuYXJkdWluby5jYzAOBgNVHQ8BAf8EBAMCBaAwHQYDVR0lBBYwFAYIKwYB\n" \
  "BQUHAwEGCCsGAQUFBwMCMDsGA1UdHwQ0MDIwMKAuoCyGKmh0dHA6Ly9jcmwuc2Nh\n" \
  "MWIuYW1hem9udHJ1c3QuY29tL3NjYTFiLmNybDAgBgNVHSAEGTAXMAsGCWCGSAGG\n" \
  "/WwBAjAIBgZngQwBAgEwdQYIKwYBBQUHAQEEaTBnMC0GCCsGAQUFBzABhiFodHRw\n" \
  "Oi8vb2NzcC5zY2ExYi5hbWF6b250cnVzdC5jb20wNgYIKwYBBQUHMAKGKmh0dHA6\n" \
  "Ly9jcnQuc2NhMWIuYW1hem9udHJ1c3QuY29tL3NjYTFiLmNydDAMBgNVHRMBAf8E\n" \
  "AjAAMIIBBAYKKwYBBAHWeQIEAgSB9QSB8gDwAHcApLkJkLQYWBSHuxOizGdwCjw1\n" \
  "mAT5G9+443fNDsgN3BAAAAFl8l4c2wAABAMASDBGAiEAg8sVD1hMTtHfkB7uGs7X\n" \
  "ywRdSJS8Bm/wKCeUCo77r60CIQCfzE4tIMSKBIz1COYawO46+YGV0fGtqaWnzYVz\n" \
  "RuLKSgB1AId1v+dZfPiMQ5lfvfNu/1aNR1Y2/0q1YMG06v9eoIMPAAABZfJeHcAA\n" \
  "AAQDAEYwRAIgVIclh+Y/HaVt9+7kFtGPH0shW5lNXTWwk0vyl6Dt2ksCIE+h325g\n" \
  "YMF5rZcyCqsx9RzTsevdG0nDuDD7LRlJmyyfMA0GCSqGSIb3DQEBCwUAA4IBAQB2\n" \
  "scFbsvi/Gkwykr/UGpUNF9pYWLJCTPH5YcC3NhYMyHaMKZ6RTzdPffchluBKMPA1\n" \
  "ewWvnfYFXQ0FHsfMID9zKZgQwS0LlouSmMHATCRxr1RmZ/KScPQ4fAC8yaw1AH6W\n" \
  "SQwIs572cZc29zv33bkTSatU/IS7wh+/AUiR02oVxxYhY2lFdaxCC66RjaAbtcO/\n" \
  "i6Si3zqrmnyx9dZUT59bc8JJNG4N8hynMZxzt6fESTY3HPirV9QR5ujzh8ClHIM7\n" \
  "3CDNM1O+C825XJAFE9vRdp5Jjl/RWybOa8p6qZeUR0rp8oJFN5J+SrAVCib6WR+q\n" \
  "3Vin6s69xo2CRV0SYoCX\n" \
  "-----END CERTIFICATE-----\n";

#endif /* BOARD_HAS_ECCX08 */

#endif /* CERTIFICATE_H_ */
