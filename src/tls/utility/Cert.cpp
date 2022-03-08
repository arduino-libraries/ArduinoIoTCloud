/*
  This file is part of the ArduinoECCX08 library.
  Copyright (c) 2019 Arduino SA. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <AIoTC_Config.h>

#if defined(BOARD_HAS_ECCX08) || defined(BOARD_HAS_OFFLOADED_ECCX08) || defined(BOARD_HAS_SE050)

#include "Cert.h"

/******************************************************************************
 * DEFINE
 ******************************************************************************/

#define ASN1_INTEGER           0x02
#define ASN1_BIT_STRING        0x03
#define ASN1_NULL              0x05
#define ASN1_OBJECT_IDENTIFIER 0x06
#define ASN1_PRINTABLE_STRING  0x13
#define ASN1_SEQUENCE          0x30
#define ASN1_SET               0x31

/******************************************************************************
 * LOCAL MODULE FUNCTIONS
 ******************************************************************************/

static String base64Encode(const byte in[], unsigned int length, const char* prefix, const char* suffix) {
  static const char* CODES = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

  int b;
  String out;

  int reserveLength = 4 * ((length + 2) / 3) + ((length / 3 * 4) / 76) + strlen(prefix) + strlen(suffix);
  out.reserve(reserveLength);

  if (prefix) {
    out += prefix;
  }

  for (unsigned int i = 0; i < length; i += 3) {
    if (i > 0 && (i / 3 * 4) % 76 == 0) {
      out += '\n';
    }

    b = (in[i] & 0xFC) >> 2;
    out += CODES[b];

    b = (in[i] & 0x03) << 4;
    if (i + 1 < length) {
      b |= (in[i + 1] & 0xF0) >> 4;
      out += CODES[b];
      b = (in[i + 1] & 0x0F) << 2;
      if (i + 2 < length) {
        b |= (in[i + 2] & 0xC0) >> 6;
        out += CODES[b];
        b = in[i + 2] & 0x3F;
        out += CODES[b];
      } else {
        out += CODES[b];
        out += '=';
      }
    } else {
      out += CODES[b];
      out += "==";
    }
  }

  if (suffix) {
    out += suffix;
  }

  return out;
}

/******************************************************************************
 * CTOR/DTOR
 ******************************************************************************/

ArduinoIoTCloudCertClass::ArduinoIoTCloudCertClass()
: _certBuffer(nullptr)
, _certBufferLen(0)
, _publicKey(nullptr)
{

}

ArduinoIoTCloudCertClass::~ArduinoIoTCloudCertClass() 
{
  if (_certBuffer) {
    free(_certBuffer);
    _certBuffer = nullptr;
  }
}

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

int ArduinoIoTCloudCertClass::begin()
{
  memset(_compressedCert.data, 0x00, CERT_COMPRESSED_CERT_LENGTH);
  return 1;
}

int ArduinoIoTCloudCertClass::buildCSR()
{
  int csrInfoLen = CSRInfoLength();
  int subjectLen = issuerOrSubjectLength(_subjectData);

  _certBufferLen = getCSRSize();
  _certBuffer = (byte*)malloc(_certBufferLen);

  if (_certBuffer == nullptr) {
    return 0;
  }

  byte* out = _certBuffer;

  // header
  out += appendSequenceHeader(csrInfoLen, out);

  // version
  out += appendVersion(0x00, out);

  // subject
  out += appendSequenceHeader(subjectLen, out);
  out += appendIssuerOrSubject(_subjectData, out);

  // public key
  if (_publicKey == nullptr) {
    return 0;
  }
  out += appendPublicKey(_publicKey, out);
  
  // terminator
  *out++ = 0xa0;
  *out++ = 0x00;

  return 1;
}

int ArduinoIoTCloudCertClass::signCSR(byte * signature)
{
  /* copy old certbuffer in a temp buffer */
  byte* tempBuffer = (byte*)malloc(_certBufferLen);

  if (tempBuffer == nullptr) {
    return 0;
  }

  memcpy(tempBuffer, _certBuffer, _certBufferLen);
  
  _certBufferLen = getCSRSignedSize(signature);
  _certBuffer = (byte*)realloc(_certBuffer, _certBufferLen);

  if (_certBuffer == nullptr) {
    return 0;
  }

  byte* out = _certBuffer;

  // header
  out += appendSequenceHeader(getCSRSize() + signatureLength(signature), out);

  // info
  memcpy(out, tempBuffer, getCSRSize());
  free(tempBuffer);
  out += getCSRSize();

  // signature
  out += appendSignature(signature, out);

  return 1;
}

String ArduinoIoTCloudCertClass::getCSRPEM()
{
  return base64Encode(_certBuffer, _certBufferLen, "-----BEGIN CERTIFICATE REQUEST-----\n", "\n-----END CERTIFICATE REQUEST-----\n");
}

int ArduinoIoTCloudCertClass::buildCert()
{
  _certBufferLen = getCertSize();
  _certBuffer = (byte*)malloc(_certBufferLen);

  if (_certBuffer == nullptr) {
    return 0;
  }
  
  uint8_t* out = _certBuffer;

  int certInfoLen = certInfoLength();

  // header
  out += appendSequenceHeader(certInfoLen, out);

  // version
  *out++ = 0xA0;
  *out++ = 0x03;
  *out++ = 0x02;
  *out++ = 0x01;
  *out++ = 0x02;

  // serial number
  out += appendSerialNumber(_compressedCert.slot.two.values.serialNumber, CERT_SERIAL_NUMBER_LENGTH, out);

  // signature type
  out += appendEcdsaWithSHA256(out);

  // issuer
  int issuerDataLen = issuerOrSubjectLength(_issuerData);
  out += appendSequenceHeader(issuerDataLen, out);
  out += appendIssuerOrSubject(_issuerData, out);

  // dates
  DateInfo dateData;
  getDateFromCompressedData(dateData);

  *out++ = ASN1_SEQUENCE;
  *out++ = 30 + ((dateData.issueYear > 2049) ? 2 : 0) + (((dateData.issueYear + dateData.expireYears) > 2049) ? 2 : 0);
  out += appendDate(dateData.issueYear, dateData.issueMonth, dateData.issueDay, dateData.issueHour, 0, 0, out);
  out += appendDate(dateData.issueYear + dateData.expireYears, dateData.issueMonth, dateData.issueDay, dateData.issueHour, 0, 0, out);

  // subject
  int subjectDataLen = issuerOrSubjectLength(_subjectData);
  out += appendSequenceHeader(subjectDataLen, out);
  out += appendIssuerOrSubject(_subjectData, out);

  // public key
  if (_publicKey == nullptr) {
    return 0;
  }
  out += appendPublicKey(_publicKey, out);

  int authorityKeyIdLen = authorityKeyIdLength(_compressedCert.slot.two.values.authorityKeyId, CERT_AUTHORITY_KEY_ID_LENGTH);
  if (authorityKeyIdLen)
  {
    out += appendAuthorityKeyId(_compressedCert.slot.two.values.authorityKeyId, CERT_AUTHORITY_KEY_ID_LENGTH, out);
  }
  else
  {
    // null sequence
    *out++ = 0xA3;
    *out++ = 0x02;
    *out++ = 0x30;
    *out++ = 0x00;
  }

  return 1;
}

int ArduinoIoTCloudCertClass::signCert(const byte * signature)
{
  /* copy old certbuffer in a temp buffer */
  byte* tempBuffer = (byte*)malloc(_certBufferLen);

  if (tempBuffer == nullptr) {
    return 0;
  }

  memcpy(tempBuffer, _certBuffer, _certBufferLen);
  
  _certBufferLen = getCertSignedSize(signature);
  _certBuffer = (byte*)realloc(_certBuffer, _certBufferLen);

  if (_certBuffer == nullptr) {
    return 0;
  }
  
  byte* out = _certBuffer;

  // header
  out +=appendSequenceHeader(getCertSize() + signatureLength(signature), out);

  // info
  memcpy(out, tempBuffer, getCertSize());
  free(tempBuffer);
  out += getCertSize();

  // signature
  out += appendSignature(signature, out);

  return 1;
}

int ArduinoIoTCloudCertClass::importCert(const byte certDER[], size_t derLen)
{
  _certBufferLen = derLen;
  _certBuffer = (byte*)malloc(_certBufferLen);

  if (_certBuffer == nullptr) {
    return 0;
  }

  memcpy(_certBuffer, certDER, _certBufferLen);

  return 1;
}

int ArduinoIoTCloudCertClass::signCert()
{
  return signCert(_compressedCert.slot.one.values.signature);
}

String ArduinoIoTCloudCertClass::getCertPEM()
{
  return base64Encode(_certBuffer, _certBufferLen, "-----BEGIN CERTIFICATE-----\n", "\n-----END CERTIFICATE-----\n");
}

void ArduinoIoTCloudCertClass::getDateFromCompressedData(DateInfo& date) {
  date.issueYear = (_compressedCert.slot.one.values.dates[0] >> 3) + 2000;
  date.issueMonth = ((_compressedCert.slot.one.values.dates[0] & 0x07) << 1) | (_compressedCert.slot.one.values.dates[1] >> 7);
  date.issueDay = (_compressedCert.slot.one.values.dates[1] & 0x7c) >> 2;
  date.issueHour = ((_compressedCert.slot.one.values.dates[1] & 0x03) << 3) | (_compressedCert.slot.one.values.dates[2] >> 5);
  date.expireYears = (_compressedCert.slot.one.values.dates[2] & 0x1f);
}

void ArduinoIoTCloudCertClass::setIssueYear(int issueYear) {
  _compressedCert.slot.one.values.dates[0] &= 0x07;
  _compressedCert.slot.one.values.dates[0] |= (issueYear - 2000) << 3;
}

void ArduinoIoTCloudCertClass::setIssueMonth(int issueMonth) {
  _compressedCert.slot.one.values.dates[0] &= 0xf8;
  _compressedCert.slot.one.values.dates[0] |= issueMonth >> 1;

  _compressedCert.slot.one.values.dates[1] &= 0x7f;
  _compressedCert.slot.one.values.dates[1] |= issueMonth << 7;
}

void ArduinoIoTCloudCertClass::setIssueDay(int issueDay) {
  _compressedCert.slot.one.values.dates[1] &= 0x83;
  _compressedCert.slot.one.values.dates[1] |= issueDay << 2;
}

void ArduinoIoTCloudCertClass::setIssueHour(int issueHour) {
  _compressedCert.slot.one.values.dates[2] &= 0x1f;
  _compressedCert.slot.one.values.dates[2] |= issueHour << 5;

  _compressedCert.slot.one.values.dates[1] &= 0xfc;
  _compressedCert.slot.one.values.dates[1] |= issueHour >> 3;
}

void ArduinoIoTCloudCertClass::setExpireYears(int expireYears) {
  _compressedCert.slot.one.values.dates[2] &= 0xe0;
  _compressedCert.slot.one.values.dates[2] |= expireYears;
}

int ArduinoIoTCloudCertClass::setSerialNumber(const uint8_t serialNumber[], int serialNumberLen) {
  if (serialNumberLen == CERT_SERIAL_NUMBER_LENGTH) {
    memcpy(_compressedCert.slot.two.values.serialNumber, serialNumber, CERT_SERIAL_NUMBER_LENGTH);
    return 1;
  }
  return 0;
}

int ArduinoIoTCloudCertClass::setAuthorityKeyId(const uint8_t authorityKeyId[], int authorityKeyIdLen) {
  if (authorityKeyIdLen == CERT_AUTHORITY_KEY_ID_LENGTH) {
    memcpy(_compressedCert.slot.two.values.authorityKeyId, authorityKeyId, CERT_AUTHORITY_KEY_ID_LENGTH);
    return 1;
  }
  return 0;
}

int ArduinoIoTCloudCertClass::setPublicKey(const byte* publicKey, int publicKeyLen) {
  if (publicKeyLen == CERT_PUBLIC_KEY_LENGTH) {
    _publicKey = publicKey;
    return 1;
  }
  return 0;
}

int ArduinoIoTCloudCertClass::setSignature(const byte* signature, int signatureLen) {
  if (signatureLen == CERT_SIGNATURE_LENGTH) {
    memcpy(_compressedCert.slot.one.values.signature, signature, CERT_SIGNATURE_LENGTH);
    return 1;
  }
  return 0;
}

/******************************************************************************
 * PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

int ArduinoIoTCloudCertClass::versionLength()
{
  return 3;
}

int ArduinoIoTCloudCertClass::issuerOrSubjectLength(const CertInfo& issuerOrSubjectData)
{
  int length                       = 0;
  int countryNameLength            = issuerOrSubjectData.countryName.length();
  int stateProvinceNameLength      = issuerOrSubjectData.stateProvinceName.length();
  int localityNameLength           = issuerOrSubjectData.localityName.length();
  int organizationNameLength       = issuerOrSubjectData.organizationName.length();
  int organizationalUnitNameLength = issuerOrSubjectData.organizationalUnitName.length();
  int commonNameLength             = issuerOrSubjectData.commonName.length();

  if (countryNameLength) {
    length += (11 + countryNameLength);
  }

  if (stateProvinceNameLength) {
    length += (11 + stateProvinceNameLength);
  }

  if (localityNameLength) {
    length += (11 + localityNameLength);
  }

  if (organizationNameLength) {
    length += (11 + organizationNameLength);
  }

  if (organizationalUnitNameLength) {
    length += (11 + organizationalUnitNameLength);
  }

  if (commonNameLength) {
    length += (11 + commonNameLength);
  }

  return length;
}

int ArduinoIoTCloudCertClass::sequenceHeaderLength(int length)
{
  if (length > 255) {
    return 4;
  } else if (length > 127) {
    return 3;
  } else {
    return 2;
  }
}

int ArduinoIoTCloudCertClass::publicKeyLength()
{
  return (2 + 2 + 9 + 10 + 4 + 64);
}

int ArduinoIoTCloudCertClass::signatureLength(const byte signature[])
{
  const byte* r = &signature[0];
  const byte* s = &signature[32];

  int rLength = 32;
  int sLength = 32;

  while (*r == 0x00 && rLength) {
    r++;
    rLength--;
  }

  if (*r & 0x80) {
    rLength++;
  }

  while (*s == 0x00 && sLength) {
    s++;
    sLength--;
  }

  if (*s & 0x80) {
    sLength++;
  }

  return (21 + rLength + sLength);
}

int ArduinoIoTCloudCertClass::serialNumberLength(const byte serialNumber[], int length)
{
  while (*serialNumber == 0 && length) {
    serialNumber++;
    length--;
  }

  if (*serialNumber & 0x80) {
    length++;
  }

  return (2 + length);
}

int ArduinoIoTCloudCertClass::authorityKeyIdLength(const byte authorityKeyId[], int length) {
  bool set = false;

  // check if the authority key identifier is non-zero
  for (int i = 0; i < length; i++) {
    if (authorityKeyId[i] != 0) {
      set = true;
      break;
    }
  }

  return (set ? (length + 17) : 0);
}

int ArduinoIoTCloudCertClass::CSRInfoLength()
{
  int versionLen = versionLength();
  int subjectLen = issuerOrSubjectLength(_subjectData);
  int subjectHeaderLen = sequenceHeaderLength(subjectLen);
  int publicKeyLen = publicKeyLength();

  int csrInfoLen = versionLen + subjectHeaderLen + subjectLen + publicKeyLen + 2;

  return csrInfoLen;
}

int ArduinoIoTCloudCertClass::getCSRSize()
{
  int csrInfoLen = CSRInfoLength();
  int csrInfoHeaderLen = sequenceHeaderLength(csrInfoLen);

  return (csrInfoLen + csrInfoHeaderLen);
}

int ArduinoIoTCloudCertClass::getCSRSignedSize(byte * signature)
{
  int signatureLen = signatureLength(signature);
  int csrLen = getCSRSize() + signatureLen;
  return sequenceHeaderLength(csrLen) + csrLen;
}

int ArduinoIoTCloudCertClass::certInfoLength()
{
  int datesSizeLen = 30;
  DateInfo dates;

  getDateFromCompressedData(dates);

  if (dates.issueYear > 2049) {
    // two more bytes for GeneralizedTime
    datesSizeLen += 2;
  }

  if ((dates.issueYear + dates.expireYears) > 2049) {
    // two more bytes for GeneralizedTime
    datesSizeLen += 2;
  }

  int serialNumberLen = serialNumberLength(_compressedCert.slot.two.values.serialNumber, CERT_SERIAL_NUMBER_LENGTH);

  int issuerLen = issuerOrSubjectLength(_issuerData);

  int issuerHeaderLen = sequenceHeaderLength(issuerLen);

  int subjectLen = issuerOrSubjectLength(_subjectData);

  int subjectHeaderLen = sequenceHeaderLength(subjectLen);

  int publicKeyLen = publicKeyLength();
  
  int certInfoLen = 5 + serialNumberLen + 12 + issuerHeaderLen + issuerLen + (datesSizeLen + 2) +
                    subjectHeaderLen + subjectLen + publicKeyLen;

  int authorityKeyIdLen = authorityKeyIdLength(_compressedCert.slot.two.values.authorityKeyId, CERT_AUTHORITY_KEY_ID_LENGTH);

  if (authorityKeyIdLen)
  {
    certInfoLen += authorityKeyIdLen;
  }
  else
  {
    certInfoLen += 4;
  }

  return certInfoLen;
}

int ArduinoIoTCloudCertClass::getCertSize()
{
  int certInfoLen = certInfoLength();
  int certInfoHeaderLen = sequenceHeaderLength(certInfoLen);

  return (certInfoLen + certInfoHeaderLen);
}

int ArduinoIoTCloudCertClass::getCertSignedSize(const byte * signature)
{
  int signatureLen = signatureLength(signature);
  int certLen = getCertSize() + signatureLen;
  return sequenceHeaderLength(certLen) + certLen;
}

int ArduinoIoTCloudCertClass::appendSequenceHeader(int length, byte out[])
{
  *out++ = ASN1_SEQUENCE;
  if (length > 255) {
    *out++ = 0x82;
    *out++ = (length >> 8) & 0xff;
  } else if (length > 127) {
    *out++ = 0x81;
  }
  *out++ = (length) & 0xff;

  if (length > 255) {
    return 4;
  } else if (length > 127) {
    return 3;
  } else {
    return 2;
  }
}

int ArduinoIoTCloudCertClass::appendVersion(int version, byte out[])
{
  out[0] = ASN1_INTEGER;
  out[1] = 0x01;
  out[2] = version;

  return versionLength();
}

int ArduinoIoTCloudCertClass::appendName(const String& name, int type, byte out[])
{
  int nameLength = name.length();

  *out++ = ASN1_SET;
  *out++ = nameLength + 9;

  *out++ = ASN1_SEQUENCE;
  *out++ = nameLength + 7;

  *out++ = ASN1_OBJECT_IDENTIFIER;
  *out++ = 0x03;
  *out++ = 0x55;
  *out++ = 0x04;
  *out++ = type;

  *out++ = ASN1_PRINTABLE_STRING;
  *out++ = nameLength;
  memcpy(out, name.c_str(), nameLength);

  return (nameLength + 11);
}

int ArduinoIoTCloudCertClass::appendIssuerOrSubject(const CertInfo& issuerOrSubjectData, byte out[])
{
  if (issuerOrSubjectData.countryName.length() > 0) {
    out += appendName(issuerOrSubjectData.countryName, 0x06, out);
  }

  if (issuerOrSubjectData.stateProvinceName.length() > 0) {
    out += appendName(issuerOrSubjectData.stateProvinceName, 0x08, out);
  }

  if (issuerOrSubjectData.localityName.length() > 0) {
    out += appendName(issuerOrSubjectData.localityName, 0x07, out);
  }

  if (issuerOrSubjectData.organizationName.length() > 0) {
    out += appendName(issuerOrSubjectData.organizationName, 0x0a, out);
  }

  if (issuerOrSubjectData.organizationalUnitName.length() > 0) {
    out += appendName(issuerOrSubjectData.organizationalUnitName, 0x0b, out);
  }

  if (issuerOrSubjectData.commonName.length() > 0) {
    out += appendName(issuerOrSubjectData.commonName, 0x03, out);
  }

  return issuerOrSubjectLength(issuerOrSubjectData);
}

int  ArduinoIoTCloudCertClass::appendPublicKey(const byte publicKey[], byte out[])
{
  int subjectPublicKeyDataLength = 2 + 9 + 10 + 4 + 64;

  // subject public key
  *out++ = ASN1_SEQUENCE;
  *out++ = (subjectPublicKeyDataLength) & 0xff;

  *out++ = ASN1_SEQUENCE;
  *out++ = 0x13;

  // EC public key
  *out++ = ASN1_OBJECT_IDENTIFIER;
  *out++ = 0x07;
  *out++ = 0x2a;
  *out++ = 0x86;
  *out++ = 0x48;
  *out++ = 0xce;
  *out++ = 0x3d;
  *out++ = 0x02;
  *out++ = 0x01;

  // PRIME 256 v1
  *out++ = ASN1_OBJECT_IDENTIFIER;
  *out++ = 0x08;
  *out++ = 0x2a;
  *out++ = 0x86;
  *out++ = 0x48;
  *out++ = 0xce;
  *out++ = 0x3d;
  *out++ = 0x03;
  *out++ = 0x01;
  *out++ = 0x07;

  *out++ = 0x03;
  *out++ = 0x42;
  *out++ = 0x00;
  *out++ = 0x04;

  memcpy(out, publicKey, 64);

  return publicKeyLength();
}

int ArduinoIoTCloudCertClass::appendSignature(const byte signature[], byte out[])
{
  // signature algorithm
  *out++ = ASN1_SEQUENCE;
  *out++ = 0x0a;
  *out++ = ASN1_OBJECT_IDENTIFIER;
  *out++ = 0x08;

  // ECDSA with SHA256
  *out++ = 0x2a;
  *out++ = 0x86;
  *out++ = 0x48;
  *out++ = 0xce;
  *out++ = 0x3d;
  *out++ = 0x04;
  *out++ = 0x03;
  *out++ = 0x02;

  const byte* r = &signature[0];
  const byte* s = &signature[32];

  int rLength = 32;
  int sLength = 32;

  while (*r == 0 && rLength) {
    r++;
    rLength--;
  }

  while (*s == 0 && sLength) {
    s++;
    sLength--;
  }

  if (*r & 0x80) {
    rLength++;
  }

  if (*s & 0x80) {
    sLength++;
  }

  *out++ = ASN1_BIT_STRING;
  *out++ = (rLength + sLength + 7);
  *out++ = 0;

  *out++ = ASN1_SEQUENCE;
  *out++ = (rLength + sLength + 4);

  *out++ = ASN1_INTEGER;
  *out++ = rLength;
  if ((*r & 0x80) && rLength) {
    *out++ = 0;
    rLength--;
  }
  memcpy(out, r, rLength);
  out += rLength;

  *out++ = ASN1_INTEGER;
  *out++ = sLength;
  if ((*s & 0x80) && sLength) {
    *out++ = 0;
    sLength--;
  }
  memcpy(out, s, sLength);
  out += rLength;

  return signatureLength(signature);
}

int ArduinoIoTCloudCertClass::appendSerialNumber(const byte serialNumber[], int length, byte out[])
{
  while (*serialNumber == 0 && length) {
    serialNumber++;
    length--;
  }

  if (*serialNumber & 0x80) {
    length++;  
  }

  *out++ = ASN1_INTEGER;
  *out++ = length;

  if (*serialNumber & 0x80) {
    *out++ = 0x00;
    length--;
  }

  memcpy(out, serialNumber, length);
  
  if (*serialNumber & 0x80) {
    length++;
  }

  return (2 + length);
}

int ArduinoIoTCloudCertClass::appendDate(int year, int month, int day, int hour, int minute, int second, byte out[])
{
  bool useGeneralizedTime = (year > 2049);

  if (useGeneralizedTime) {
    *out++ = 0x18;
    *out++ = 0x0f;
    *out++ = '0' + (year / 1000);
    *out++ = '0' + ((year % 1000) / 100);
    *out++ = '0' + ((year % 100) / 10);
    *out++ = '0' + (year % 10);
  } else {
    year -= 2000;

    *out++ = 0x17;
    *out++ = 0x0d;
    *out++ = '0' + (year / 10);
    *out++ = '0' + (year % 10);
  }
  *out++ = '0' + (month / 10);
  *out++ = '0' + (month % 10);
  *out++ = '0' + (day / 10);
  *out++ = '0' + (day % 10);
  *out++ = '0' + (hour / 10);
  *out++ = '0' + (hour % 10);
  *out++ = '0' + (minute / 10);
  *out++ = '0' + (minute % 10);
  *out++ = '0' + (second / 10);
  *out++ = '0' + (second % 10);
  *out++ = 0x5a; // UTC

  return (useGeneralizedTime ? 17 : 15);
}

int ArduinoIoTCloudCertClass::appendEcdsaWithSHA256(byte out[])
{
  *out++ = ASN1_SEQUENCE;
  *out++ = 0x0A;
  *out++ = ASN1_OBJECT_IDENTIFIER;
  *out++ = 0x08;
  *out++ = 0x2A;
  *out++ = 0x86;
  *out++ = 0x48;
  *out++ = 0xCE;
  *out++ = 0x3D;
  *out++ = 0x04;
  *out++ = 0x03;
  *out++ = 0x02;

  return 12;
}

int ArduinoIoTCloudCertClass::appendAuthorityKeyId(const byte authorityKeyId[], int length, byte out[]) {
  // [3]
  *out++ = 0xa3;
  *out++ = 0x23;

  // sequence
  *out++ = ASN1_SEQUENCE;
  *out++ = 0x21;

  // sequence
  *out++ = ASN1_SEQUENCE;
  *out++ = 0x1f;

  // 2.5.29.35 authorityKeyIdentifier(X.509 extension)
  *out++ = 0x06;
  *out++ = 0x03;
  *out++ = 0x55;
  *out++ = 0x1d;
  *out++ = 0x23;

  // octet string
  *out++ = 0x04;
  *out++ = 0x18;

  // sequence
  *out++ = ASN1_SEQUENCE;
  *out++ = 0x16;

  *out++ = 0x80;
  *out++ = 0x14;

  memcpy(out, authorityKeyId, length);

  return length + 17;
}

#endif /* (BOARD_HAS_ECCX08) || defined(BOARD_HAS_OFFLOADED_ECCX08) || defined(BOARD_HAS_SE050) */
