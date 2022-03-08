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

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <AIoTC_Config.h>

#if defined(BOARD_HAS_ECCX08) || defined(BOARD_HAS_OFFLOADED_ECCX08) || defined(BOARD_HAS_SE050)

#include "CryptoUtil.h"
#include "SHA256.h"

/******************************************************************************
 * DEFINE
 ******************************************************************************/
#define CRYPTO_SHA256_BUFFER_LENGTH  32
#define CRYPTO_CERT_BUFFER_LENGTH  1024

/**************************************************************************************
 * CTOR/DTOR
 **************************************************************************************/
CryptoUtil::CryptoUtil()
#if defined(BOARD_HAS_SE050)
: _crypto {SE05X}
#else
: _crypto {ECCX08}
#endif
{

}

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

int CryptoUtil::buildCSR(ArduinoIoTCloudCertClass & cert, const CryptoSlot keySlot, bool newPrivateKey)
{
  byte publicKey[CERT_PUBLIC_KEY_LENGTH];
  byte signature[CERT_SIGNATURE_LENGTH];

  if (newPrivateKey) {
    if (!_crypto.generatePrivateKey(static_cast<int>(keySlot), publicKey)) {
      return 0;
    }
  } else {
    if (!_crypto.generatePublicKey(static_cast<int>(keySlot), publicKey)) {
      return 0;
    }
  }

  /* Store public key in csr */
  if (!cert.setPublicKey(publicKey, CERT_PUBLIC_KEY_LENGTH)) {
    return 0;
  }
  
  /* Build CSR */
  if (!cert.buildCSR()) {
    return 0;
  }

  /* compute CSR SHA256 */
  SHA256 sha256;
  byte sha256buf[CRYPTO_SHA256_BUFFER_LENGTH];
  sha256.begin();
  sha256.update(cert.bytes(), cert.length());
  sha256.finalize(sha256buf);

  if (!_crypto.ecSign(static_cast<int>(keySlot), sha256buf, signature)) {
    return 0;
  }

  /* sign CSR */
  return cert.signCSR(signature);
}

int CryptoUtil::buildCert(ArduinoIoTCloudCertClass & cert, const CryptoSlot keySlot)
{
  byte publicKey[CERT_PUBLIC_KEY_LENGTH];

  if (!_crypto.generatePublicKey(static_cast<int>(keySlot), publicKey)) {
    return 0;
  }

  /* Store public key in csr */
  if (!cert.setPublicKey(publicKey, CERT_PUBLIC_KEY_LENGTH)) {
    return 0;
  }

  /* Build CSR */
  if (!cert.buildCert()) {
    return 0;
  }

  /* sign CSR */
  return cert.signCert();
}

int CryptoUtil::readDeviceId(String & device_id, const CryptoSlot device_id_slot)
{
  byte device_id_bytes[CERT_COMPRESSED_CERT_SLOT_LENGTH] = {0};

  if (!_crypto.readSlot(static_cast<int>(device_id_slot), device_id_bytes, sizeof(device_id_bytes))) {
    return 0;
  }

  device_id = String(reinterpret_cast<char *>(device_id_bytes));
  return 1;
}

int CryptoUtil::writeDeviceId(String & device_id, const CryptoSlot device_id_slot)
{
  byte device_id_bytes[CERT_COMPRESSED_CERT_SLOT_LENGTH] = {0};

  device_id.getBytes(device_id_bytes, sizeof(device_id_bytes));
  
  if (!_crypto.writeSlot(static_cast<int>(device_id_slot), device_id_bytes, sizeof(device_id_bytes))) {
    return 0;
  }
  return 1;
}

int CryptoUtil::writeCert(ArduinoIoTCloudCertClass & cert, const CryptoSlot certSlot)
{
#if defined(BOARD_HAS_SE050)
  if (!_crypto.writeSlot(static_cast<int>(certSlot), cert.bytes(), cert.length())) {
    return 0;
  }
#else
  if (!_crypto.writeSlot(static_cast<int>(certSlot), cert.compressedCertSignatureAndDatesBytes(), cert.compressedCertSignatureAndDatesLength())) {
    return 0;
  }

  if (!_crypto.writeSlot(static_cast<int>(certSlot) + 1, cert.compressedCertSerialAndAuthorityKeyIdBytes(), cert.compressedCertSerialAndAuthorityKeyIdLenght())) {
    return 0;
  }
#endif
  return 1;
}

int CryptoUtil::readCert(ArduinoIoTCloudCertClass & cert, const CryptoSlot certSlot)
{
#if defined(BOARD_HAS_SE050)
  byte derBuffer[CRYPTO_CERT_BUFFER_LENGTH];
  size_t derLen;
  if (!_crypto.readBinaryObject(static_cast<int>(certSlot), derBuffer, sizeof(derBuffer), &derLen)) {
    return 0;
  }

  if (!cert.importCert(derBuffer, derLen)) {
    return 0;
  }
#else
  String deviceId;
  byte publicKey[CERT_PUBLIC_KEY_LENGTH];

  cert.begin();

  if (!readDeviceId(deviceId, CryptoSlot::DeviceId)) {
    return 0;
  }

  if (!_crypto.readSlot(static_cast<int>(certSlot), cert.compressedCertSignatureAndDatesBytes(), cert.compressedCertSignatureAndDatesLength())) {
    return 0;
  }

  if (!_crypto.readSlot(static_cast<int>(certSlot) + 1, cert.compressedCertSerialAndAuthorityKeyIdBytes(), cert.compressedCertSerialAndAuthorityKeyIdLenght())) {
    return 0;
  }

  if (!_crypto.generatePublicKey(static_cast<int>(CryptoSlot::Key), publicKey)) {
    return 0;
  }

  cert.setSubjectCommonName(deviceId);
  cert.setIssuerCountryName("US");
  cert.setIssuerOrganizationName("Arduino LLC US");
  cert.setIssuerOrganizationalUnitName("IT");
  cert.setIssuerCommonName("Arduino");

  if (!cert.setPublicKey(publicKey, CERT_PUBLIC_KEY_LENGTH)) {
    return 0;
  }

  if (!cert.buildCert()) {
    return 0;
  }

  if (!cert.signCert()) {
    return 0;
  }
#endif
  return 1;
}

#endif /* (BOARD_HAS_ECCX08) || defined(BOARD_HAS_OFFLOADED_ECCX08) || defined(BOARD_HAS_SE050) */
