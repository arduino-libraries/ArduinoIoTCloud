#include <WiFi101.h>
#include <ArduinoCloudV2.h>

#include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;     // the WiFi radio's status

WiFiClient wifiClient;

const char thingId[] = "blah";

unsigned long getTime() {
  return WiFi.getTime();
}

int position;

void onPositionUpdate() {
  Serial.print("New position value: ");
  Serial.println(position);
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

  if (!ArduinoCloud.begin(wifiClient, thingId)) {
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
    delay(10000);
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

  ArduinoCloud.Thing.addProperty(position, READ).publishEvery(10*SECONDS).onUpdate(onPositionUpdate);

  CloudSerial.begin(9600);
}

void loop() {
  ArduinoCloud.poll();

  if (CloudSerial.available()) {
    Serial.write(CloudSerial.read());
  }

  if (Serial.available()) {
    CloudSerial.write(Serial.read());
  }
}

