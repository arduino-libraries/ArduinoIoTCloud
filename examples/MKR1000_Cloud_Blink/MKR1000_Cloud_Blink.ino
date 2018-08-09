#include <WiFi101.h>
#include <ArduinoCloudV2.h>

#include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;     // the WiFi radio's status

WiFiClient wifiClient;

unsigned long getTime() {
  return WiFi.getTime();
}

// Thing properties
int position;
bool valid = true;
float ratio = 2.47;
String welcome = "ciao";

// Last time when the WiFi connection was checked
unsigned long lastMillis = 0;

void onPositionUpdate() {
  Serial.print("New position value: ");
  Serial.println(position);
}

void onWelcomeUpdate() {
  Serial.print("New state value: ");
  Serial.println(welcome);
}

void onValidUpdate() {
  Serial.print("New valid value: ");
  Serial.println(valid);
}

void onRatioUpdate() {
  Serial.print("New ratio value: ");
  Serial.println(ratio);
}

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

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
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(5000);
  }

  // you're connected now, so print out the data:
  Serial.print("You're connected to the network");

  Serial.println();
  Serial.println("Attempting to connect to Arduino Cloud ...");

  ArduinoCloud.onGetTime(getTime);
  if (!ArduinoCloud.connect()) {
    Serial.println("Failed to connect to Arduino Cloud!");
    while (1);
  }

  Serial.println("Successfully connected to Arduino Cloud :)");
  ArduinoCloud.addProperty(welcome, READWRITE, ON_CHANGE, onWelcomeUpdate);
  ArduinoCloud.addProperty(position, READWRITE, ON_CHANGE, onPositionUpdate);
  ArduinoCloud.addProperty(valid, READWRITE, ON_CHANGE, onValidUpdate);
  ArduinoCloud.addProperty(ratio, READWRITE, ON_CHANGE, onRatioUpdate);

  CloudSerial.begin(9600);
  lastMillis = millis();
}

void loop() {
  ArduinoCloud.poll();
  Serial.println("loop updated");
 /* 
  Serial.println(".");
  welcome += "!";
  ratio += 0.4355;
  valid = !valid;
  position += 1;
*/
  if (millis() - lastMillis > 20000) {
    Serial.println("..Check WiFi status..");
    bool error = false;
    // Check Wifi status
    while (WiFi.status() != WL_CONNECTED) {
      error = true;
      Serial.print("..Reconnection to connect to WPA SSID: ");
      Serial.println(ssid);
      status = WiFi.begin(ssid, pass);
      // wait 10 seconds for connection:
      delay(2000);
    }
    if(error) {
      Serial.println("..Reconnected to the Nework!");
     // Call the reconnect method to clean up the ArduinoCloud connection
     ArduinoCloud.reconnect(wifiClient);
    }
    delay(500);
    lastMillis = millis();
  }

  delay(2000);
}
