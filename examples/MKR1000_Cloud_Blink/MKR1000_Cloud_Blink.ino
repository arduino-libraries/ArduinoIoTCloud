#include <WiFi101.h>
#include <ArduinoCloudV2.h>

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;     // the WiFi radio's status
String serialString = "";        // the string used to compose network messages from the received characters

WiFiClient wifiClient;

unsigned long getTime() {
  return WiFi.getTime();
}

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  delay(7000);

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  if (!ArduinoCloud.begin(wifiClient)) {
    Serial.println("Starting Arduino Cloud failed!");
    while (true);
  }

  // attempt to connect to WiFi network:
  int attempts = 0;
  while (status != WL_CONNECTED && attempts < 6) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
    attempts++;
  }

  if (status != WL_CONNECTED) {
    Serial.println("Failed to connect to Wifi!");
  }

  // you're connected now, so print out the data:
  Serial.print("You're connected to the network");

  Serial.println();
  Serial.println("Attempting to connect to Arduino Cloud ...");

  ArduinoCloud.onGetTime(getTime);

  attempts = 0;
  while (!ArduinoCloud.connect() && attempts < 10) {
    attempts++;
  }

  if (attempts >= 3) {
    Serial.println("Failed to connect to Arduino Cloud!");
    while (1);
  }

  Serial.println("Successfully connected to Arduino Cloud :)");

  CloudSerial.begin(9600);
  CloudSerial.print("I'm ready for blinking!\n");
}

void loop() {
  ArduinoCloud.poll();

  // check if there is something waiting to be read
  if (CloudSerial.available()) {
    char character = CloudSerial.read();
    serialString += character;

    // if a \n character has been received, there should be a complete command inside serialString
    if (character == '\n') {
      manageString();
    }
  }
  else // if there is nothing to read, it could be that the last command didn't end with a '\n'. Check.
  {
    manageString();
  }

  // Just to be able to simulate MKR1000's responses through the serial monitor
  if (Serial.available()) {
    CloudSerial.write(Serial.read());
  }
}

void manageString() {
  // Don't proceed if the string is empty
  if (serialString.equals("")) return;

  // Remove whitespaces
  serialString.trim();

  // Make it uppercase;
  serialString.toUpperCase();

  if (serialString.equals("ON")) {
    digitalWrite(6, HIGH);
  }
  if (serialString.equals("OFF")) {
    digitalWrite(6, LOW);
  }

  // Send back the command you just applied. This way the Angular frontend can stay synchronized with the MKR1000 state.
  sendString(serialString);

  // Reset serialString
  serialString = "";
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