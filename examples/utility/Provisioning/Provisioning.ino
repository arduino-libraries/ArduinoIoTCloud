#include <ArduinoCloudV2.h>
#include <utility/ECC508Cert.h>
#include <utility/ECC508TLSConfig.h>

#include <ArduinoBearSSL.h>
#include <utility/ECC508.h>

const int keySlot            = 0;
const int compressedCertSlot = 10;
const int serialNumberSlot   = 11;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!ECC508.begin()) {
    Serial.println("No ECC508 present!");
    while (1);
  }

  if (!ECC508.locked()) {
    Serial.println("ECC508 is unlocked, locking ...");

    if (!ECC508.writeConfiguration(DEFAULT_ECC508_TLS_CONFIG)) {
      Serial.println("Writing ECC508 configuration failed!");
      while (1);
    }

    if (!ECC508.lock()) {
      Serial.println("Locking ECC508 configuration failed!");
      while (1);
    }

    Serial.println("ECC508 locked successfully");
    Serial.println();
  }

  if (!ECC508Cert.beginCSR(keySlot, true)) {
    Serial.println("Error starting CSR generation!");
    while (1);
  }

  ECC508Cert.setSubjectCommonName(ECC508.serialNumber());

  String csr = ECC508Cert.endCSR();

  if (!csr) {
    Serial.println("Error generating CSR!");
    while (1);
  }

  Serial.println("Generated CSR is:");
  Serial.println();
  Serial.println(csr);

  String issueYear    = promptAndReadLine("Please enter the issue year of the certificate (2000 - 2031): ");
  String issueMonth   = promptAndReadLine("Please enter the issue month of the certificate (1 - 12): ");
  String issueDay     = promptAndReadLine("Please enter the issue day of the certificate (1 - 31): ");
  String issueHour    = promptAndReadLine("Please enter the issue hour of the certificate (0 - 23): ");
  String expireYears  = promptAndReadLine("Please enter how many years the certificate is valid for (0 - 31): ");
  String serialNumber = promptAndReadLine("Please enter the certificates serial number: ");
  String signature    = promptAndReadLine("Please enter the certificates signature: ");

  serialNumber.toUpperCase();
  signature.toUpperCase();

  byte serialNumberBytes[72];
  byte signatureBytes[64];

  hexStringToBytes(serialNumber, serialNumberBytes, sizeof(serialNumberBytes));
  hexStringToBytes(signature, signatureBytes, 64);

  if (!ECC508Cert.beginStorage(compressedCertSlot, serialNumberSlot)) {
    Serial.println("Error starting ECC508 storage!");
    while (1);
  }

  ECC508Cert.setSignature(signatureBytes);
  ECC508Cert.setSerialNumber(serialNumberBytes);
  ECC508Cert.setIssueYear(issueYear.toInt());
  ECC508Cert.setIssueMonth(issueMonth.toInt());
  ECC508Cert.setIssueDay(issueDay.toInt());
  ECC508Cert.setIssueHour(issueHour.toInt());
  ECC508Cert.setExpireYears(expireYears.toInt());

  if (!ECC508Cert.endStorage()) {
    Serial.println("Error storing ECC508 compressed cert!");
    while (1);
  }

  if (!ECC508Cert.beginReconstruction(keySlot, compressedCertSlot, serialNumberSlot)) {
    Serial.println("Error starting ECC508 cert reconstruction!");
    while (1);
  }

  ECC508Cert.setIssuerCountryName("US");
  ECC508Cert.setIssuerOrganizationName("Arduino LLC US");
  ECC508Cert.setIssuerOrganizationalUnitName("IT");
  ECC508Cert.setIssuerCommonName("Arduino");

  if (!ECC508Cert.endReconstruction()) {
    Serial.println("Error reconstructing ECC508 compressed cert!");
    while (1);
  }

  Serial.println("Compressed cert = ");

  const byte* certData = ECC508Cert.bytes();
  int certLength = ECC508Cert.length();

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

void hexStringToBytes(const String& in, byte out[], int length) {
  int inLength = in.length();
  int outLength = 0;

  for (int i = 0; i < inLength && outLength < length; i += 2) {
    char highChar = in[i];
    char lowChar = in[i + 1];

    byte highByte = (highChar <= '9') ? (highChar - '0') : (highChar + 10 - 'A');
    byte lowByte = (lowChar <= '9') ? (lowChar - '0') : (lowChar + 10 - 'A');

    out[outLength++] = (highByte << 4) | lowByte;
  }
}

