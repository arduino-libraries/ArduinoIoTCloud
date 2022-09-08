#include <ArduinoIoTCloud.h>
#include "ECCX08TLSConfig.h"

const bool DEBUG = true;

ArduinoIoTCloudCertClass Certificate;
CryptoUtil Crypto;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!Crypto.begin()) {
    Serial.println("No crypto present!");
    while (1);
  }

  if (!Crypto.locked()) {
    String lockConfirm = promptAndReadLine("Your crypto is unlocked, would you like to lock it (y/N): ");
    lockConfirm.toLowerCase();

    if (lockConfirm != "y") {
      Serial.println("That's all folks");
      while (1);
    }

    if (!Crypto.writeConfiguration(DEFAULT_ECCX08_TLS_CONFIG)) {
      Serial.println("Writing crypto configuration failed!");
      while (1);
    }

    if (!Crypto.lock()) {
      Serial.println("Locking crypto configuration failed!");
      while (1);
    }

    Serial.println("crypto locked successfully");
    Serial.println();
  }

  String csrConfirm = promptAndReadLine("Would you like to generate a new private key and CSR (y/N): ");
  csrConfirm.toLowerCase();

  if (csrConfirm != "y") {
    Serial.println("That's all folks");
    while (1);
  }

  if (!Certificate.begin()) {
    Serial.println("Error starting CSR generation!");
    while (1);
  }

  String deviceId = promptAndReadLine("Please enter the device ID: ");
  Certificate.setSubjectCommonName(deviceId);

  if (!Crypto.buildCSR(Certificate, CryptoSlot::Key, true)) {
    Serial.println("Error generating CSR!");
    while (1);
  }

  String csr = Certificate.getCSRPEM();

  if (!csr) {
    Serial.println("Error generating CSR!");
    while (1);
  }

  Serial.println("Generated CSR is:");
  Serial.println();
  Serial.println(csr);

  String issueYear              = promptAndReadLine("Please enter the issue year of the certificate (2000 - 2031): ");
  String issueMonth             = promptAndReadLine("Please enter the issue month of the certificate (1 - 12): ");
  String issueDay               = promptAndReadLine("Please enter the issue day of the certificate (1 - 31): ");
  String issueHour              = promptAndReadLine("Please enter the issue hour of the certificate (0 - 23): ");
  String expireYears            = promptAndReadLine("Please enter how many years the certificate is valid for (0 - 31): ");
  String serialNumber           = promptAndReadLine("Please enter the certificates serial number: ");
  String authorityKeyIdentifier = promptAndReadLine("Please enter the certificates authority key identifier: ");
  String signature              = promptAndReadLine("Please enter the certificates signature: ");

  byte serialNumberBytes[CERT_SERIAL_NUMBER_LENGTH];
  byte authorityKeyIdentifierBytes[CERT_AUTHORITY_KEY_ID_LENGTH];
  byte signatureBytes[CERT_SIGNATURE_LENGTH];

  hexStringToBytes(serialNumber, serialNumberBytes, sizeof(serialNumberBytes));
  hexStringToBytes(authorityKeyIdentifier, authorityKeyIdentifierBytes, sizeof(authorityKeyIdentifierBytes));
  hexStringToBytes(signature, signatureBytes, sizeof(signatureBytes));

  if (!Crypto.writeDeviceId(deviceId, CryptoSlot::DeviceId)) {
    Serial.println("Error storing device ID!");
    while (1);
  }

  if (!Certificate.begin()) {
    Serial.println("Error starting crypto storage!");
    while (1);
  }

  Certificate.setSubjectCommonName(deviceId);
  Certificate.setIssuerCountryName("US");
  Certificate.setIssuerOrganizationName("Arduino LLC US");
  Certificate.setIssuerOrganizationalUnitName("IT");
  Certificate.setIssuerCommonName("Arduino");
  Certificate.setSignature(signatureBytes, sizeof(signatureBytes));
  Certificate.setAuthorityKeyId(authorityKeyIdentifierBytes, sizeof(authorityKeyIdentifierBytes));
  Certificate.setSerialNumber(serialNumberBytes, sizeof(serialNumberBytes));
  Certificate.setIssueYear(issueYear.toInt());
  Certificate.setIssueMonth(issueMonth.toInt());
  Certificate.setIssueDay(issueDay.toInt());
  Certificate.setIssueHour(issueHour.toInt());
  Certificate.setExpireYears(expireYears.toInt());

  if (!Crypto.buildCert(Certificate, CryptoSlot::Key)) {
    Serial.println("Error building cert!");
    while (1);
  }
  
  if (!Crypto.writeCert(Certificate, CryptoSlot::CompressedCertificate)) {
    Serial.println("Error storing cert!");
    while (1);
  }

  if (!DEBUG) {
    return;
  }

  Serial.println("Compressed cert = ");

  const byte* certData = Certificate.bytes();
  int certLength = Certificate.length();

  for (int i = 0; i < certLength; i++) {
    byte b = certData[i];

    if (b < 16) {
      Serial.print('0');
    }
    Serial.print(b, HEX);
  }
  Serial.println();
}

void loop() {
}

String promptAndReadLine(const char* prompt) {
  Serial.print(prompt);
  String s = readLine();
  Serial.println(s);

  return s;
}

String readLine() {
  String line;

  while (1) {
    if (Serial.available()) {
      char c = Serial.read();

      if (c == '\r') {
        // ignore
      } else if (c == '\n') {
        break;
      }

      line += c;
    }
  }

  line.trim();

  return line;
}

void hexStringToBytes(String& in, byte out[], int length) {
  int inLength = in.length();
  in.toUpperCase();
  int outLength = 0;

  for (int i = 0; i < inLength && outLength < length; i += 2) {
    char highChar = in[i];
    char lowChar = in[i + 1];

    byte highByte = (highChar <= '9') ? (highChar - '0') : (highChar + 10 - 'A');
    byte lowByte = (lowChar <= '9') ? (lowChar - '0') : (lowChar + 10 - 'A');

    out[outLength++] = (highByte << 4) | (lowByte & 0xF);
  }
}
