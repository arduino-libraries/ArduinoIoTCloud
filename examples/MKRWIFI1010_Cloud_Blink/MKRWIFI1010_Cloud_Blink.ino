#include <ArduinoIoTCloud.h>
#include <ConnectionManager.h>

#if defined(BOARD_HAS_WIFI)
  #include <WiFiConnectionManager.h>
#elif defined(BOARD_HAS_GSM)
  #include <GSMConnectionManager.h>
#else
  #error "Arduino IoT Cloud currently only supports MKR1000, MKR WiFi 1010 and MKR GSM 1400"
#endif

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_WIFI_NAME; // your network SSID (name)
char pass[] = SECRET_PASSWORD; // your network password (use for WPA, or use as key for WEP)

String cloudSerialBuffer = ""; // the string used to compose network messages from the received characters
// handles connection to the network
#if defined(BOARD_HAS_WIFI)
  ConnectionManager * ArduinoIoTPreferredConnection = new WiFiConnectionManager(SECRET_WIFI_NAME, SECRET_PASSWORD);
#elif defined(BOARD_HAS_GSM)
  ConnectionManager * ArduinoIoTPreferredConnection = new GSMConnectionManager(SECRET_PIN, SECRET_APN, SECRET_LOGIN, SECRET_PASS);
#endif

void setup() {
  setDebugMessageLevel(3); // used to set a level of granularity in information output [0...4]
  Serial.begin(9600);
  while (!Serial); // waits for the serial to become available
  ArduinoCloud.begin(ArduinoIoTPreferredConnection); // initialize a connection to the Arduino IoT Cloud
  while (ArduinoCloud.connected()); // needed to wait for the initialization of CloudSerial
  CloudSerial.print("I'm ready for blinking!\n");
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  ArduinoCloud.update();
  // check if there is something waiting to be read
  if (CloudSerial.available()) {
    char character = CloudSerial.read();
    cloudSerialBuffer += character;
    // if a \n character has been received, there should be a complete command inside cloudSerialBuffer
    if (character == '\n') {
      handleString();
    }
  } else { // if there is nothing to read, it could be that the last command didn't end with a '\n'. Check.
    handleString();
  }
  // Just to be able to simulate the board responses through the serial monitor
  if (Serial.available()) {
    CloudSerial.write(Serial.read());
  }
}
void handleString() {
  // Don't proceed if the string is empty
  if (cloudSerialBuffer.equals("")) {
    return;
  }
  // Remove leading and trailing whitespaces
  cloudSerialBuffer.trim();
  // Make it uppercase;
  cloudSerialBuffer.toUpperCase();
  if (cloudSerialBuffer.equals("ON")) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else if (cloudSerialBuffer.equals("OFF")) {
    digitalWrite(LED_BUILTIN, LOW);
  }
  sendString(cloudSerialBuffer);
  // Reset cloudSerialBuffer
  cloudSerialBuffer = "";
}
// sendString sends a string to the Arduino Cloud.
void sendString(String stringToSend) {
  // send the characters one at a time
  char lastSentChar = 0;
  for (int i = 0; i < stringToSend.length(); i++) {
    lastSentChar = stringToSend.charAt(i);
    CloudSerial.write(lastSentChar);
  }
  // if the last sent character wasn't a '\n' add it
  if (lastSentChar != '\n') {
    CloudSerial.write('\n');
  }
}
