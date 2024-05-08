/*
  Self Provisioning on Arduino IoT Cloud

  This sketch demonstrates how to automate the provisioning
  of the Arduino boards MKR WiFi 1010 and Nano 33 IoT to
  the Arduino IoT Cloud, using the Arduino Cloud API.

  Circuit:
   - MKR WiFi 1010 board
   - Nano 33 IoT board

  created 14 August 2020
  by Luigi Gubello

  This example code is in the public domain.
*/

#include "arduino_secrets.h"
#include <Arduino_SecureElement.h>
#include <utility/SElementCSR.h>
#include <utility/SElementArduinoCloud.h>
#include <utility/SElementArduinoCloudCertificate.h>
#include <Arduino_JSON.h>

const bool DEBUG = true;

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
char client_id[] = SECRET_CLIENT_ID;
char secret_id[] = SECRET_SECRET_ID;

#if defined(ARDUINO_SAMD_NANO_33_IOT)
    #include <WiFiNINA.h>
    char board_type[] = "nano_33_iot"; // Nano 33 IoT
    char board_fqbn[] = "arduino:samd:nano_33_iot"; // Nano 33 IoT
#elif defined(ARDUINO_SAMD_MKRWIFI1010)
    #include <WiFiNINA.h>
    char board_type[] = "mkrwifi1010"; // MKR WiFi 1010
    char board_fqbn[] = "arduino:samd:mkrwifi1010"; // MKR WiFi 1010
#elif defined(ARDUINO_NANO_RP2040_CONNECT)
    #include <WiFiNINA.h>
    char board_type[] = "nanorp2040connect"; // Nano RP2040 Connect
    char board_fqbn[] = "arduino:mbed_nano:nanorp2040connect"; // Nano RP2040 Connect
#elif defined(ARDUINO_PORTENTA_H7_M7)
    #include <WiFi.h>
    char board_type[] = "envie_m7"; // Portenta H7
    char board_fqbn[] = "arduino:mbed_portenta:envie_m7"; // Portenta H7
#elif defined(ARDUINO_NICLA_VISION)
    #include <WiFi.h>
    char board_type[] = "nicla_vision"; // Nicla Vision
    char board_fqbn[] = "arduino:mbed_nicla:nicla_vision"; // Nicla Vision
#elif defined(ARDUINO_GIGA)
    #include <WiFi.h>
    char board_type[] = "giga"; // Giga R1 WiFi
    char board_fqbn[] = "arduino:mbed_giga:giga"; // Giga R1 WiFi
#elif defined(ARDUINO_OPTA)
    #include <WiFi.h>
    char board_type[] = "opta"; // Opta
    char board_fqbn[] = "arduino:mbed_opta:opta"; // Opta
#elif defined(ARDUINO_PORTENTA_C33)
    #include <WiFiC3.h>
    #include <WiFiSSLClient.h>
    char board_type[] = "portenta_c33"; // Portenta C33
    char board_fqbn[] = "arduino:renesas_portenta:portenta_c33"; // Portenta C33
#elif defined(ARDUINO_UNOR4_WIFI)
    #include <WiFiS3.h>
    char board_type[] = "unor4wifi"; // UNO R4 WiFi
    char board_fqbn[] = "arduino:renesas_uno:unor4wifi"; // UNO R4 WiFI
#else
    char board_type[] = "unsupported"; // Not supported boards
    char board_fqbn[] = "";
#endif

/// Board Info
String board_name = "NewDevice_";
String ArduinoID;

String Arduino_Token;
String deviceId;

String not_before;
String serialNumber;
String authorityKeyIdentifier;
String signature;

char server[] = "api2.arduino.cc";  // server address

WiFiSSLClient client;
int status = WL_IDLE_STATUS;

SecureElement secureElement;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (board_type == "unsupported") {
    Serial.println("Sorry, this sketch only works on Nano 33 IoT and MKR 1010 WiFi");
    while (1) { ; }
  }

  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);

    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
    delay(3000);
  }

  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  while (!secureElement.begin()) {
    Serial.println("No secureElement present!");
    delay(100);
  }

  if (!secureElement.locked()) {

    if (!secureElement.writeConfiguration()) {
      Serial.println("Writing secureElement configuration failed!");
      Serial.println("Stopping Provisioning");
      while (1);
    }

    if (!secureElement.lock()) {
      Serial.println("Locking secureElement configuration failed!");
      Serial.println("Stopping Provisioning");
      while (1);
    }

    Serial.println("secureElement locked successfully");
    Serial.println();
  }

  //Random number for device name
  board_name += String(secureElement.random(65535));
  Serial.print("Device Name: ");
  Serial.println(board_name);
  //Board Serial Number
  ArduinoID = ArduinoSerialNumber();
  Serial.print("SN: ");
  Serial.println(ArduinoID);
  // Create Arduino Token
  ArduinoToken(client_id, secret_id);
  Serial.print("Bearer Token: ");
  Serial.println(Arduino_Token);
  // Generating Device UUID
  BoardUuid(board_name, board_type, board_fqbn, ArduinoID, Arduino_Token);
  Serial.print("Device UUID:");
  Serial.println(deviceId);
  
  delay(2000);

  // Configure WiFi firmware version
  String fv = WiFi.firmwareVersion();
  WiFiFirmwareVersion(fv, deviceId, Arduino_Token);
  Serial.print("WiFi Firmware Version: ");
  Serial.println(fv);

  ECP256Certificate Certificate;

  while (!Certificate.begin()) {
    Serial.println("Error starting CSR generation!");
    while (1);
  }

  Certificate.setSubjectCommonName(deviceId);

  if (!SElementCSR::build(secureElement, Certificate, static_cast<int>(SElementArduinoCloudSlot::Key), true)) {
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
  Serial.println(csr);

  // Downloading Arduino cert
  ArduinoCertificate(Arduino_Token, deviceId, csr);
  
  String issueYear              = not_before.substring(0,4);
  Serial.print("Year: ");
  Serial.println(issueYear);
  String issueMonth             = not_before.substring(5,7);
  Serial.print("Month: ");
  Serial.println(issueMonth);
  String issueDay               = not_before.substring(8,10);
  Serial.print("Day: ");
  Serial.println(issueDay);
  String issueHour              = not_before.substring(11,13);
  Serial.print("Hour: ");
  Serial.println(issueHour);
  String expireYears            = "31";
  Serial.print("Certificate Serial: ");
  Serial.println(serialNumber);
  Serial.print("Certificate Authority Key: ");
  Serial.println(authorityKeyIdentifier);
  Serial.print("Certificate Signature: ");
  Serial.println(signature);

  byte deviceIdBytes[72];
  byte serialNumberBytes[16];
  byte authorityKeyIdentifierBytes[20];
  byte signatureBytes[64];

  deviceId.getBytes(deviceIdBytes, sizeof(deviceIdBytes));
  hexStringToBytes(serialNumber, serialNumberBytes, sizeof(serialNumberBytes));
  hexStringToBytes(authorityKeyIdentifier, authorityKeyIdentifierBytes, sizeof(authorityKeyIdentifierBytes));
  hexStringToBytes(signature, signatureBytes, sizeof(signatureBytes));

  if (!secureElement.writeSlot(static_cast<int>(SElementArduinoCloudSlot::DeviceId), deviceIdBytes, sizeof(deviceIdBytes))) {
    Serial.println("Error storing device id!");
    while (1);
  }

  if (!Certificate.begin()) {
    Serial.println("Error starting secureElement storage!");
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
    Serial.println("Error building secureElement compressed cert!");
    while (1);
  }

  if (!SElementArduinoCloudCertificate::write(secureElement, Certificate, SElementArduinoCloudSlot::CompressedCertificate)) {
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

#if defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_RENESAS)

static void utox8(uint32_t val, uint8_t* s) {
  for (int i = 0; i < 16; i=i+2) {
    int d = val & 0XF;
    val = (val >> 4);

    s[15 - i -1] = d > 9 ? 'A' + d - 10 : '0' + d;
    s[15 - i] = '\0';
  }
}
#endif

#ifdef ARDUINO_ARCH_SAMD

uint8_t getUniqueSerialNumber(uint8_t* name) {
  utox8(*(volatile uint32_t*)(0x0080A00C), &name[0]);
  utox8(*(volatile uint32_t*)(0x0080A040), &name[16]);
  utox8(*(volatile uint32_t*)(0x0080A044), &name[32]);
  utox8(*(volatile uint32_t*)(0x0080A048), &name[48]);
  return 64;
}

#endif

#ifdef ARDUINO_ARCH_RENESAS
uint8_t getUniqueSerialNumber(uint8_t* name) {
  const bsp_unique_id_t* t = R_BSP_UniqueIdGet();
  utox8(t->unique_id_words[0], &name[0]);
  utox8(t->unique_id_words[1], &name[16]);
  utox8(t->unique_id_words[2], &name[32]);
  utox8(t->unique_id_words[3], &name[48]);
  return 64;
}
#endif

String ArduinoSerialNumber() {

  uint8_t uniqueSerialNumber[64 + 1] = {0};
  char BoardUniqueID[32 + 1] = {0};

  int uniqueSerialNumberLength = getUniqueSerialNumber(uniqueSerialNumber);
  for(int i = 0, k = 0; i < uniqueSerialNumberLength; i = i+2, k++) {
    BoardUniqueID[k] = uniqueSerialNumber[i];
  }

  String serialNumber = String(BoardUniqueID);
  serialNumber.toUpperCase();
  return serialNumber;

}

void WiFiFirmwareVersion(String fv, String deviceId, String token) {
  Serial.println("Configuring WiFi firmware version...");
  String PostData = "{\"wifi_fw_version\":\"";
  PostData += fv;
  PostData += "\"}";

  if (client.connect(server, 443)) {
    client.print("POST /iot/v2/devices/");
    client.print(deviceId);
    client.println(" HTTP/1.1");
    client.println("Host: api2.arduino.cc");
    client.println("Connection: close");
    client.println("Content-Type: application/json;charset=UTF-8");
    client.print("Authorization: Bearer ");
    client.println(token);
    client.print("Content-Length: ");
    client.println(PostData.length());
    client.println();
    client.println(PostData);
  }
  client.stop();
}

void ArduinoToken(String client_id, String client_secret) {
  Serial.println("Creating Bearer Token...");
  String PostData = "grant_type=client_credentials&client_id=";
  PostData += client_id;
  PostData += "&client_secret=";
  PostData += secret_id;
  PostData += "&audience=https://api2.arduino.cc/iot";
  
  if (client.connect(server, 443)) {
    client.println("POST /iot/v1/clients/token HTTP/1.1");
    client.println("Host: api2.arduino.cc");
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(PostData.length());
    client.println();
    client.println(PostData);
  }

  while (!client.available()) {
    Serial.println("No client");
    delay(2000);
  }

  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println("Invalid response");
    Serial.println("Error during token generation");
    while (1) {
      ;
    }
  }

  char tokenResponse[1024];
  int intIndex = 0;
  while (client.available()) {
    tokenResponse[intIndex] = client.read();
    if (tokenResponse[intIndex] == -1) {
      break;
    }
    delay(1);
    intIndex++;
  }
  JSONVar myObject = JSON.parse(tokenResponse);
  if (myObject.hasOwnProperty("access_token")) {
    Arduino_Token += (const char*) myObject["access_token"];
  }
  client.stop();
}

void BoardUuid(String board_name, String board_type, String board_fqbn, String board_serial, String user_token) {
  Serial.println("Adding board to IoT Cloud...");
  String PostData = "{\"fqbn\":\"";
  PostData += board_fqbn;
  PostData += "\",\"name\":\"";
  PostData += board_name;
  PostData += "\",\"type\":\"";
  PostData += board_type;
  PostData += "\",\"serial\":\"";
  PostData += board_serial;
  PostData += "\"}";

  if (client.connect(server, 443)) {
    client.println("PUT /iot/v2/devices HTTP/1.1");
    client.println("Host: api2.arduino.cc");
    client.println("Connection: close");
    client.println("Content-Type: application/json;charset=UTF-8");
    client.print("Authorization: Bearer ");
    client.println(user_token);
    client.print("Content-Length: ");
    client.println(PostData.length());
    client.println();
    client.println(PostData);
  }

  while (!client.available()) {
    Serial.println("No client");
    delay(2000);
  }

  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println("Invalid response");
    Serial.println("Error during Device UUID generation");
    while (1) {
      ;
    }
  }
  
  char deviceResponse[512];
  int intIndex = 0;
  while (client.available()) {
    deviceResponse[intIndex] = client.read();
    if (deviceResponse[intIndex] == -1) {
      break;
    }
    delay(1);
    intIndex++;
  }
  JSONVar myObject = JSON.parse(deviceResponse);
  if (myObject.hasOwnProperty("id")) {
    deviceId += (const char*) myObject["id"];
  }
  client.stop();
}

void ArduinoCertificate(String user_token, String DeviceUuid, String csr) {
  Serial.println("Downloading certificate...");
  String url = "/iot/v2/devices/";
  url += DeviceUuid;
  url += "/certs";

  csr.replace("\n", "\\n");
  String PostData = "{\"ca\":\"Arduino\",\"csr\":\"";
  PostData += csr;
  PostData += "\",\"enabled\":true}";

  if (client.connect(server, 443)) {
    client.print("PUT ");
    client.print(url);
    client.println(" HTTP/1.1");
    client.println("Host: api2.arduino.cc");
    client.println("Connection: close");
    client.println("Content-Type: application/json;charset=UTF-8");
    client.print("Authorization: Bearer ");
    client.println(user_token);
    client.print("Content-Length: ");
    client.println(PostData.length());
    client.println();
    client.println(PostData);
  }

  while (!client.available()) {
    ;
  }

  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println("Invalid response");
    Serial.println("Error during certificate generation");
    while (1) {
      ;
    }
  }
  
  char certResponse[4096];
  int intIndex = 0;
  while (client.available()) {
    certResponse[intIndex] = client.read();
    if (certResponse[intIndex] == -1) {
      break;
    }
    delay(1);
    intIndex++;
  }
  char* p = strstr(certResponse, "{");
  JSONVar myObject = JSON.parse(p);
  String certZip = JSON.stringify(myObject["compressed"]);
  JSONVar myCert = JSON.parse(certZip);
  if (myCert.hasOwnProperty("not_before") &&
      myCert.hasOwnProperty("serial") &&
      myCert.hasOwnProperty("authority_key_identifier") &&
      myCert.hasOwnProperty("signature_asn1_x") &&
      myCert.hasOwnProperty("signature_asn1_x")) {
    not_before += (const char*) myCert["not_before"];
    serialNumber += (const char*) myCert["serial"];
    authorityKeyIdentifier += (const char*) myCert["authority_key_identifier"];
    signature += (const char*) myCert["signature_asn1_x"];
    signature += (const char*) myCert["signature_asn1_y"];
  } else {
    Serial.println("Error parsing cloud certificate");
    while (1) {
      ;
    }
  }
}
