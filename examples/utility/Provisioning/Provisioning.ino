#include <Arduino_SecureElement.h>
#include <utility/SElementArduinoCloud.h>
#include <utility/SElementArduinoCloudCertificate.h>
#include <utility/SElementArduinoCloudDeviceId.h>
#include <utility/SElementCSR.h>

#ifdef ARDUINO_SAMD_MKR1000
#include <WiFi101.h>
#define LATEST_WIFI_FIRMWARE_VERSION WIFI_FIRMWARE_LATEST_MODEL_B
#endif
#if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT) || defined(ARDUINO_NANO_RP2040_CONNECT)
#include <WiFiNINA.h>
#define LATEST_WIFI_FIRMWARE_VERSION WIFI_FIRMWARE_LATEST_VERSION
#endif
#if defined(ARDUINO_UNOR4_WIFI)
#include <WiFiS3.h>
#define LATEST_WIFI_FIRMWARE_VERSION WIFI_FIRMWARE_LATEST_VERSION
#endif

String promptAndReadLine(const char* prompt, const unsigned int timeout = 0);

void setup() {
  Serial.begin(9600);
  while (!Serial);

  SecureElement secureElement;

  if (!secureElement.begin()) {
    Serial.println("No crypto present!");
    while (1);
  }

  if (!secureElement.locked()) {
    /* WARNING: This string is parsed from IoTCloud frontend */
    String lockConfirm = promptAndReadLine("Your crypto is unlocked, would you like to lock it (y/N): ");
    lockConfirm.toLowerCase();

    if (lockConfirm != "y") {
      Serial.println("That's all folks");
      while (1);
    }

    if (!secureElement.writeConfiguration()) {
      /* WARNING: This string is parsed from IoTCloud frontend */
      Serial.println("Writing crypto configuration failed!");
      while (1);
    }

    if (!secureElement.lock()) {
      /* WARNING: This string is parsed from IoTCloud frontend */
      Serial.println("Locking crypto configuration failed!");
      while (1);
    }

    Serial.println("crypto locked successfully");
    Serial.println();
  }

  /* WARNING: This string is parsed from IoTCloud frontend */
  String csrConfirm = promptAndReadLine("Would you like to generate a new private key and CSR (y/N): ", 5000);
  csrConfirm.toLowerCase();

  if (csrConfirm != "y") {
    Serial.println("That's all folks");
    while (1);
  }

  ECP256Certificate Certificate;

  if (!Certificate.begin()) {
    Serial.println("Error starting CSR generation!");
    while (1);
  }

  /* WARNING: This string is parsed from IoTCloud frontend */
  String deviceId = promptAndReadLine("Please enter the device id: ");
  Certificate.setSubjectCommonName(deviceId);

  if (!SElementCSR::build(secureElement, Certificate, (int)SElementArduinoCloudSlot::Key, true)) {
    /* WARNING: This string is parsed from IoTCloud frontend */
    Serial.println("Error generating CSR!");
    while (1);
  }

  String csr = Certificate.getCSRPEM();

  if (!csr) {
    /* WARNING: This string is parsed from IoTCloud frontend */
    Serial.println("Error generating CSR!");
    while (1);
  }

  Serial.println("Generated CSR is:");
  Serial.println();
  /* WARNING: This string is parsed from IoTCloud frontend */
  Serial.println(csr);

  String issueYear              = promptAndReadLine("Please enter the issue year of the certificate (2000 - 2031): ");
  String issueMonth             = promptAndReadLine("Please enter the issue month of the certificate (1 - 12): ");
  String issueDay               = promptAndReadLine("Please enter the issue day of the certificate (1 - 31): ");
  String issueHour              = promptAndReadLine("Please enter the issue hour of the certificate (0 - 23): ");
  String expireYears            = promptAndReadLine("Please enter how many years the certificate is valid for (0 - 31): ");
  String serialNumber           = promptAndReadLine("Please enter the certificates serial number: ");
  String authorityKeyIdentifier = promptAndReadLine("Please enter the certificates authority key identifier: ");
  String signature              = promptAndReadLine("Please enter the certificates signature: ");

  byte serialNumberBytes[ECP256_CERT_SERIAL_NUMBER_LENGTH];
  byte authorityKeyIdentifierBytes[ECP256_CERT_AUTHORITY_KEY_ID_LENGTH];
  byte signatureBytes[ECP256_CERT_SIGNATURE_LENGTH];

  hexStringToBytes(serialNumber, serialNumberBytes, sizeof(serialNumberBytes));
  hexStringToBytes(authorityKeyIdentifier, authorityKeyIdentifierBytes, sizeof(authorityKeyIdentifierBytes));
  hexStringToBytes(signature, signatureBytes, sizeof(signatureBytes));

  if (!SElementArduinoCloudDeviceId::write(secureElement, deviceId, SElementArduinoCloudSlot::DeviceId)) {
    Serial.println("Error storing device ID!");
    while (1);
  }

  if (!Certificate.begin()) {
    /* WARNING: This string is parsed from IoTCloud frontend */
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

  if (!SElementArduinoCloudCertificate::build(secureElement, Certificate, static_cast<int>(SElementArduinoCloudSlot::Key))) {
    Serial.println("Error building cert!");
    while (1);
  }

  if (!SElementArduinoCloudCertificate::write(secureElement, Certificate, SElementArduinoCloudSlot::CompressedCertificate)) {
    Serial.println("Error storing cert!");
    while (1);
  }

  /* WARNING: This string is parsed from IoTCloud frontend */
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


  String cert = Certificate.getCertPEM();
  if (!cert) {
    Serial.println("Error generating cert!");
    while (1);
  }
  Serial.println("Cert PEM = ");
  Serial.println();
  Serial.println(cert);


#ifdef LATEST_WIFI_FIRMWARE_VERSION
  Serial.println("Checking firmware of WiFi module...");
  Serial.println();
  String fv = WiFi.firmwareVersion();
  /* WARNING: This string is parsed from IoTCloud frontend */
  Serial.print("Current firmware version: ");
  /* WARNING: This string is parsed from IoTCloud frontend */
  Serial.println(fv);

  String latestFv = LATEST_WIFI_FIRMWARE_VERSION;
  if (fv >= latestFv) {
    /* WARNING: This string is parsed from IoTCloud frontend */
    Serial.println("Latest firmware version correctly installed.");
  } else {
    /* WARNING: This string is parsed from IoTCloud frontend */
    String latestFvStr = "The firmware is not up to date. Latest version available: " + latestFv;
    Serial.println(latestFvStr);
  }
#else
  Serial.println();
  /* WARNING: This string is parsed from IoTCloud frontend */
  Serial.println("Program finished.");
#endif
}

void loop() {
}

String promptAndReadLine(const char* prompt, const unsigned int timeout) {
  String s = "";
  while(1) {
    Serial.print(prompt);
    s = readLine(timeout);
    if (s.length() > 0) {
      break;
    }
  }
  Serial.println(s);

  return s;
}

bool isExpired(const unsigned int start, const unsigned int timeout) {
  if (timeout) {
    return (millis() - start) > timeout;
  } else {
    return false;
  }
}

String readLine(const unsigned int timeout) {
  String line;
  const unsigned int start = millis();
  while (!isExpired(start, timeout)) {
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
