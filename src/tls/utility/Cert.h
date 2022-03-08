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

#ifndef ARDUINO_IOT_CLOUD_CERT_H
#define ARDUINO_IOT_CLOUD_CERT_H

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <AIoTC_Config.h>

#if defined(BOARD_HAS_ECCX08) || defined(BOARD_HAS_OFFLOADED_ECCX08) || defined(BOARD_HAS_SE050)

/******************************************************************************
 * DEFINE
 ******************************************************************************/

#define CERT_SERIAL_NUMBER_LENGTH            16
#define CERT_AUTHORITY_KEY_ID_LENGTH         20
#define CERT_PUBLIC_KEY_LENGTH               64
#define CERT_SIGNATURE_LENGTH                64
#define CERT_DATES_LENGTH                     3
#define CERT_COMPRESSED_CERT_SLOT_LENGTH     72
#define CERT_COMPRESSED_CERT_LENGTH         CERT_COMPRESSED_CERT_SLOT_LENGTH + CERT_SERIAL_NUMBER_LENGTH + CERT_AUTHORITY_KEY_ID_LENGTH

#include <Arduino.h>

class ArduinoIoTCloudCertClass {
public:
           ArduinoIoTCloudCertClass();
  virtual ~ArduinoIoTCloudCertClass();

  int begin();
  int end();

  /* APIs used only for Certificate generation*/
  void setIssueYear(int issueYear);
  void setIssueMonth(int issueMonth);
  void setIssueDay(int issueDay);
  void setIssueHour(int issueHour);
  void setExpireYears(int expireYears);
  int setSerialNumber(const uint8_t serialNumber[], int serialNumberLen);
  int setAuthorityKeyId(const uint8_t authorityKeyId[], int authorityKeyIdLen);

  inline void setIssuerCountryName(const String& countryName) { _issuerData.countryName = countryName; }
  inline void setIssuerStateProvinceName(const String& stateProvinceName) { _issuerData.stateProvinceName = stateProvinceName; }
  inline void setIssuerLocalityName(const String& localityName) { _issuerData.localityName = localityName; }
  inline void setIssuerOrganizationName(const String& organizationName) { _issuerData.organizationName = organizationName; }
  inline void setIssuerOrganizationalUnitName(const String& organizationalUnitName) { _issuerData.organizationalUnitName = organizationalUnitName; }
  inline void setIssuerCommonName(const String& commonName) { _issuerData.commonName = commonName; }

  /* APIs used for both CSR and Certificate generation */
  inline void setSubjectCountryName(const String& countryName) { _subjectData.countryName = countryName; }
  inline void setSubjectStateProvinceName(const String& stateProvinceName) { _subjectData.stateProvinceName = stateProvinceName; }
  inline void setSubjectLocalityName(const String& localityName) { _subjectData.localityName = localityName; }
  inline void setSubjectOrganizationName(const String& organizationName) { _subjectData.organizationName = organizationName; }
  inline void setSubjectOrganizationalUnitName(const String& organizationalUnitName) { _subjectData.organizationalUnitName = organizationalUnitName; }
  inline void setSubjectCommonName(const String& commonName) { _subjectData.commonName = commonName; }

  int setPublicKey(const byte* publicKey, int publicKeyLen);
  int setSignature(const byte* signature, int signatureLen);

  /* Get Buffer */
  inline byte* bytes() { return _certBuffer; }
  inline int length() { return _certBufferLen; }

#if defined(BOARD_HAS_ECCX08) || defined(BOARD_HAS_OFFLOADED_ECCX08)
  /* Get Data to create ECCX08 compressed cert */
  inline byte* compressedCertBytes() { return _compressedCert.data; }
  inline int compressedCertLenght() {return CERT_COMPRESSED_CERT_LENGTH; }
  inline byte* compressedCertSignatureAndDatesBytes() { return _compressedCert.slot.one.data; }
  inline int compressedCertSignatureAndDatesLength() {return CERT_COMPRESSED_CERT_SLOT_LENGTH; }
  inline byte* compressedCertSerialAndAuthorityKeyIdBytes() { return _compressedCert.slot.two.data; }
  inline int compressedCertSerialAndAuthorityKeyIdLenght() {return CERT_SERIAL_NUMBER_LENGTH + CERT_AUTHORITY_KEY_ID_LENGTH; }
#endif

  /* Build CSR */
  int buildCSR();
  int signCSR(byte signature[]);
  String getCSRPEM();

  /* Build Certificate */
  int buildCert();
  int signCert(const byte signature[]);
  int signCert();
  String getCertPEM();

  /* Import DER buffer into CertClass*/
  int importCert(const byte certDER[], size_t derLen);

private:

  struct CertInfo {
    String countryName;
    String stateProvinceName;
    String localityName;
    String organizationName;
    String organizationalUnitName;
    String commonName;
  }_issuerData, _subjectData;

  struct DateInfo {
    int issueYear;
    int issueMonth;
    int issueDay;
    int issueHour;
    int expireYears;
  };

  union SignatureAndDateUType {
    struct __attribute__((__packed__)) SignatureAndDateType {
      byte signature[CERT_SIGNATURE_LENGTH];
      byte dates[CERT_DATES_LENGTH];
      byte unused[5];
    } values;
    byte data[CERT_COMPRESSED_CERT_SLOT_LENGTH];
  };

  union SerialNumberAndAuthorityKeyIdUType {
    struct __attribute__((__packed__)) SerialNumberAndAuthorityKeyIdType {
      byte serialNumber[CERT_SERIAL_NUMBER_LENGTH];
      byte authorityKeyId[CERT_AUTHORITY_KEY_ID_LENGTH];
    } values;
    byte data[CERT_SERIAL_NUMBER_LENGTH + CERT_AUTHORITY_KEY_ID_LENGTH];
  };

  union CompressedCertDataUType {
    struct __attribute__((__packed__)) CompressedCertDataType {
      SignatureAndDateUType one;
      SerialNumberAndAuthorityKeyIdUType two;
    }slot;
    byte data[CERT_COMPRESSED_CERT_SLOT_LENGTH + CERT_SERIAL_NUMBER_LENGTH + CERT_AUTHORITY_KEY_ID_LENGTH];
  } _compressedCert;

  /* only raw EC X Y values 64 byte */
  const byte * _publicKey;

  byte * _certBuffer;
  int    _certBufferLen;

  int versionLength();
  int issuerOrSubjectLength(const CertInfo& issuerOrSubjectData);
  int sequenceHeaderLength(int length);
  int publicKeyLength();
  int signatureLength(const byte signature[]);
  int serialNumberLength(const byte serialNumber[], int length);
  int authorityKeyIdLength(const byte authorityKeyId[], int length);
  int CSRInfoLength();
  int getCSRSize();
  int getCSRSignedSize(byte signature[]);
  int certInfoLength();
  int getCertSize();
  int getCertSignedSize(const byte signature[]);

  void getDateFromCompressedData(DateInfo& date);

  int appendSequenceHeader(int length, byte out[]);
  int appendVersion(int version, byte out[]);
  int appendName(const String& name, int type, byte out[]);
  int appendIssuerOrSubject(const CertInfo& issuerOrSubjectData, byte out[]);
  int appendPublicKey(const byte publicKey[], byte out[]);
  int appendSignature(const byte signature[], byte out[]);
  int appendSerialNumber(const byte serialNumber[], int length, byte out[]);
  int appendDate(int year, int month, int day, int hour, int minute, int second, byte out[]);
  int appendEcdsaWithSHA256(byte out[]);
  int appendAuthorityKeyId(const byte authorityKeyId[], int length, byte out[]);

};

#endif /* BOARD_HAS_ECCX08 || BOARD_HAS_OFFLOADED_ECCX08 || BOARD_HAS_SE050*/

#endif /* ARDUINO_IOT_CLOUD_CERT_H */
