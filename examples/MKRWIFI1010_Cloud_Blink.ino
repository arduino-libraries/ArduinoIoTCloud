#include <ArduinoIoTCloud.h>
#include <WiFiConnectionManager.h>

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_WIFI_NAME;        // your network SSID (name)
char pass[] = SECRET_PASSWORD;    // your network password (use for WPA, or use as key for WEP)

String cloudSerialBuffer="";     // the string used to compose network messages from the received characters

ConnectionManager *ArduinoIoTPreferredConnection = new WiFiConnectionManager(SECRET_WIFI_NAME, SECRET_PASSWORD);

void setup(){
  setDebugMessageLevel(3);
  //Serial.begin(9600);
  delay(1500);
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  //CloudSerial.begin(9600); //Already done by ArduinoIotCloud.cpp (line 332)
  CloudSerial.print("I'm ready for blinking!\n");
}

void loop(){
  ArduinoCloud.update();
  // check if there is something waiting to be read
  if (CloudSerial.available()) {
    char character = CloudSerial.read();
    cloudSerialBuffer += character;
    // if a \n character has been received, there should be a complete command inside cloudSerialBuffer
    if (character == '\n') {
      handleString();
    }
  }
  else // if there is nothing to read, it could be that the last command didn't end with a '\n'. Check.
  {
    handleString();
  }
  // Just to be able to simulate the board responses through the serial monitor
  if (Serial.available()) {
    CloudSerial.write(Serial.read());
  }
}
void handleString() {
  // Don't proceed if the string is empty
  if (cloudSerialBuffer.equals("")) return;
  // Remove leading and trailing whitespaces
  cloudSerialBuffer.trim();
  // Make it uppercase;
  cloudSerialBuffer.toUpperCase();
  if (cloudSerialBuffer.equals("ON")) digitalWrite(LED_BUILTIN, HIGH);
  else if (cloudSerialBuffer.equals("OFF")) digitalWrite(LED_BUILTIN, LOW);
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

/*
OLD                        |                     NEW
"WiFi shield not present" --> "WiFi Hardware not available\nMake sure you are using a WiFi enabled board/shield", 0
"Starting Arduino Cloud..." --> 
"Attempting to connect to WPA SSID: " -->  "Connecting to \"%s\"", 2
"Failed to connect to Wifi!" --> "Connection to \"%s\" failed", 0
"You're connected to the network" --> "Connected to \"%s\"", 2
"Attempting to connect to Arduino Cloud" --> "Connecting to Arduino IoT Cloud...", 0
"Starting get time" --> "Acquiring Time from Network", 3
"Failed to connect to Arduino Cloud!" --> "Cloud Error. Retrying...", 0
"Successfully connected to Arduino Cloud :)" --> "Connected to Arduino IoT Cloud", 0




*/

/* se tutto va bene

[ 2566 ] WiFi.status(): 0

[ 2566 ] Current WiFi Firmware: 1.2.1

[ 2567 ] Connecting to "BCMI"

[ 12642 ] Connected to "BCMI"

[ 12642 ] Acquiring Time from Network
.
[ 12743 ] Network Time: 1550241011

[ 12938 ] Connecting to Arduino IoT Cloud...
Compile time: 1553472000

[ 15990 ] Connected to Arduino IoT Cloud
*/