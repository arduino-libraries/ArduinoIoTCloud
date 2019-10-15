/* This sketch is used during the getting started tutorial when
   initialising a Arduino cloud-enabled board with the Arduino
   cloud for the first time.

   This sketch is compatible with:
     - MKR GSM 1400
*/

#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

#include "arduino_secrets.h"

String cloudSerialBuffer = ""; // the string used to compose network messages from the received characters
// handles connection to the network
GSMConnectionHandler ArduinoIoTPreferredConnection(SECRET_PIN, SECRET_APN, SECRET_USER_NAME, SECRET_PASSWORD);

void setup() {
  setDebugMessageLevel(3); // used to set a level of granularity in information output [0...4]
  Serial.begin(9600);
  while (!Serial); // waits for the serial to become available
  ArduinoCloud.begin(ArduinoIoTPreferredConnection); // initialize a connection to the Arduino IoT Cloud
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  ArduinoCloud.update();
  // check if there is something waiting to be read
  if (ArduinoCloud.connected()) {
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
  for (unsigned int i = 0; i < stringToSend.length(); i++) {
    lastSentChar = stringToSend.charAt(i);
    CloudSerial.write(lastSentChar);
  }
  // if the last sent character wasn't a '\n' add it
  if (lastSentChar != '\n') {
    CloudSerial.write('\n');
  }
}
