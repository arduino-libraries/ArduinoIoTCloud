#ifndef _ECCX08_CERT_H_
#define _ECCC08_CERT_H_

#include <Arduino.h>

class ECCX08CertClass {

public:
  ECCX08CertClass();
  virtual ~ECCX08CertClass();

  int beginCSR(int keySlot, bool newPrivateKey = true);
  String endCSR();

  int beginStorage(int compressedCertSlot, int serialNumberSlot);
  void setSignature(byte signature[]);
  void setIssueYear(int issueYear);
  void setIssueMonth(int issueMonth);
  void setIssueDay(int issueDay);
  void setIssueHour(int issueHour);
  void setExpireYears(int expireYears);
  void setSerialNumber(byte serialNumber[]);
  int endStorage();

  int beginReconstruction(int keySlot, int compressedCertSlot, int serialNumberSlot);
  int endReconstruction();

  byte* bytes();
  int length();

  void setIssuerCountryName(const String& countryName);
  void setIssuerStateProvinceName(const String& stateProvinceName);
  void setIssuerLocalityName(const String& localityName);
  void setIssuerOrganizationName(const String& organizationName);
  void setIssuerOrganizationalUnitName(const String& organizationalUnitName);
  void setIssuerCommonName(const String& commonName);

  void setSubjectCountryName(const String& countryName);
  void setSubjectStateProvinceName(const String& stateProvinceName);
  void setSubjectLocalityName(const String& localityName);
  void setSubjectOrganizationName(const String& organizationName);
  void setSubjectOrganizationalUnitName(const String& organizationalUnitName);
  void setSubjectCommonName(const String& commonName);

private:
  int versionLength();

  int issuerOrSubjectLength(const String& countryName,
                            const String& stateProvinceName,
                            const String& localityName,
                            const String& organizationName,
                            const String& organizationalUnitName,
                            const String& commonName);

  int publicKeyLength();

  int signatureLength(const byte signature[]);

  int serialNumberLength(const byte serialNumber[]);

  int sequenceHeaderLength(int length);

  void appendVersion(int version, byte out[]);

  void appendIssuerOrSubject(const String& countryName,
                             const String& stateProvinceName,
                             const String& localityName,
                             const String& organizationName,
                             const String& organizationalUnitName,
                             const String& commonName,
                             byte out[]);

  void appendPublicKey(const byte publicKey[], byte out[]);

  void appendSignature(const byte signature[], byte out[]);

  void appendSerialNumber(const byte serialNumber[], byte out[]);

  int appendName(const String& name, int type, byte out[]);

  void appendSequenceHeader(int length, byte out[]);

  int appendDate(int year, int month, int day, int hour, int minute, int second, byte out[]);

  int appendEcdsaWithSHA256(byte out[]);

private:
  int _keySlot;
  int _compressedCertSlot;
  int _serialNumberSlot;

  String _issuerCountryName;
  String _issuerStateProvinceName;
  String _issuerLocalityName;
  String _issuerOrganizationName;
  String _issuerOrganizationalUnitName;
  String _issuerCommonName;

  String _subjectCountryName;
  String _subjectStateProvinceName;
  String _subjectLocalityName;
  String _subjectOrganizationName;
  String _subjectOrganizationalUnitName;
  String _subjectCommonName;

  byte _temp[144];
  byte* _bytes;
  int _length;
};

extern ECCX08CertClass ECCX08Cert;

#endif
