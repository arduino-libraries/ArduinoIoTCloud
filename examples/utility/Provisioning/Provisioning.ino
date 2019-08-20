#include <ArduinoIoTCloud.h>
#include <utility/ECCX08Cert.h>
#include <utility/ECCX08TLSConfig.h>

#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>

const bool DEBUG = true;
const int keySlot                                   = 0;
const int compressedCertSlot                        = 10;
const int serialNumberAndAuthorityKeyIdentifierSlot = 11;
const int deviceIdSlot                              = 12;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!ECCX08.begin()) {
    Serial.println("No ECCX08 present!");
    while (1);
  }

  if (!ECCX08.locked()) {
    String lockConfirm = promptAndReadLine("Your ECCX08 is unlocked, would you like to lock it (y/N): ");
    lockConfirm.toLowerCase();

    if (lockConfirm != "y") {
      Serial.println("That's all folks");
      while (1);
    }

    if (!ECCX08.writeConfiguration(DEFAULT_ECCX08_TLS_CONFIG)) {
      Serial.println("Writing ECCX08 configuration failed!");
      while (1);
    }

    if (!ECCX08.lock()) {
      Serial.println("Locking ECCX08 configuration failed!");
      while (1);
    }

    Serial.println("ECCX08 locked successfully");
    Serial.println();
  }

  String csrConfirm = promptAndReadLine("Would you like to generate a new private key and CSR (y/N): ");
  csrConfirm.toLowerCase();

  if (csrConfirm != "y") {
    Serial.println("That's all folks");
    while (1);
  }

  if (!ECCX08Cert.beginCSR(keySlot, true)) {
    Serial.println("Error starting CSR generation!");
    while (1);
  }

  String deviceId = promptAndReadLine("Please enter the device id: ");
  ECCX08Cert.setSubjectCommonName(deviceId);

  String csr = ECCX08Cert.endCSR();

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

  byte deviceIdBytes[72];
  byte serialNumberBytes[16];
  byte authorityKeyIdentifierBytes[20];
  byte signatureBytes[64];

  deviceId.getBytes(deviceIdBytes, sizeof(deviceIdBytes));
  hexStringToBytes(serialNumber, serialNumberBytes, sizeof(serialNumberBytes));
  hexStringToBytes(authorityKeyIdentifier, authorityKeyIdentifierBytes, sizeof(authorityKeyIdentifierBytes));
  hexStringToBytes(signature, signatureBytes, sizeof(signatureBytes));

  if (!ECCX08.writeSlot(deviceIdSlot, deviceIdBytes, sizeof(deviceIdBytes))) {
    Serial.println("Error storing device id!");
    while (1);
  }

  if (!ECCX08Cert.beginStorage(compressedCertSlot, serialNumberAndAuthorityKeyIdentifierSlot)) {
    Serial.println("Error starting ECCX08 storage!");
    while (1);
  }

  ECCX08Cert.setSignature(signatureBytes);
  ECCX08Cert.setAuthorityKeyIdentifier(authorityKeyIdentifierBytes);
  ECCX08Cert.setSerialNumber(serialNumberBytes);
  ECCX08Cert.setIssueYear(issueYear.toInt());
  ECCX08Cert.setIssueMonth(issueMonth.toInt());
  ECCX08Cert.setIssueDay(issueDay.toInt());
  ECCX08Cert.setIssueHour(issueHour.toInt());
  ECCX08Cert.setExpireYears(expireYears.toInt());

  if (!ECCX08Cert.endStorage()) {
    Serial.println("Error storing ECCX08 compressed cert!");
    while (1);
  }

  if (!ECCX08Cert.beginReconstruction(keySlot, compressedCertSlot, serialNumberAndAuthorityKeyIdentifierSlot)) {
    Serial.println("Error starting ECCX08 cert reconstruction!");
    while (1);
  }

  ECCX08Cert.setIssuerCountryName("US");
  ECCX08Cert.setIssuerOrganizationName("Arduino LLC US");
  ECCX08Cert.setIssuerOrganizationalUnitName("IT");
  ECCX08Cert.setIssuerCommonName("Arduino");

  if (!ECCX08Cert.endReconstruction()) {
    Serial.println("Error reconstructing ECCX08 compressed cert!");
    while (1);
  }

  if (!DEBUG) {
    return;
  }

  Serial.println("Compressed cert = ");

  const byte* certData = ECCX08Cert.bytes();
  int certLength = ECCX08Cert.length();

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
