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

#ifndef ARDUINO_IOT_CLOUD_UTILITY_CRYPTO_CRYPTO_UTIL_H_
#define ARDUINO_IOT_CLOUD_UTILITY_CRYPTO_CRYPTO_UTIL_H_

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <AIoTC_Config.h>

#if defined(BOARD_HAS_ECCX08) || defined(BOARD_HAS_OFFLOADED_ECCX08) || defined(BOARD_HAS_SE050)
#include <Arduino.h>
#include "Cert.h"

#if defined(BOARD_HAS_SE050)
#include <SE05X.h>
#else
#include <ArduinoECCX08.h>
#endif

/******************************************************************************
 * DEFINE
 ******************************************************************************/
#if defined(BOARD_HAS_SE050)
#define CRYPTO_SLOT_OFFSET                100
#else
#define CRYPTO_SLOT_OFFSET                0
#endif

/******************************************************************************
   TYPEDEF
 ******************************************************************************/
enum class CryptoSlot : int
{
  Key                                   = (0  + CRYPTO_SLOT_OFFSET),
  CompressedCertificate                 = (10 + CRYPTO_SLOT_OFFSET),
  SerialNumberAndAuthorityKeyIdentifier = (11 + CRYPTO_SLOT_OFFSET),
  DeviceId                              = (12 + CRYPTO_SLOT_OFFSET)
};

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class CryptoUtil
{
public:

  CryptoUtil();

  inline int begin() { return _crypto.begin(); }
  inline int locked() { return _crypto.locked(); }
  inline int writeConfiguration(const byte config[]) { return _crypto.writeConfiguration(config); }
  inline int lock() { return _crypto.lock(); }

  int buildCSR(ArduinoIoTCloudCertClass & cert, const CryptoSlot keySlot, bool newPrivateKey);
  int buildCert(ArduinoIoTCloudCertClass & cert, const CryptoSlot keySlot);

  int readDeviceId(String & device_id, const CryptoSlot device_id_slot);
  int writeDeviceId(String & device_id, const CryptoSlot device_id_slot);
  int writeCert(ArduinoIoTCloudCertClass & cert, const CryptoSlot certSlot);
  int readCert(ArduinoIoTCloudCertClass & cert, const CryptoSlot certSlot);

private:
#if defined(BOARD_HAS_SE050)
  SE05XClass & _crypto;
#else
  ECCX08Class & _crypto;
#endif

};

#endif /* BOARD_HAS_ECCX08 || BOARD_HAS_OFFLOADED_ECCX08 || BOARD_HAS_SE050 */

#endif /* ARDUINO_IOT_CLOUD_UTILITY_CRYPTO_CRYPTO_UTIL_H_ */
