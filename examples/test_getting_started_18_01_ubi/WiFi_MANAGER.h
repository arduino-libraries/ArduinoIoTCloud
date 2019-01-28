#ifdef BOARD_HAS_WIFI

#ifndef ARDUINO_SAMD_MKR1000
#include <WiFiNINA.h>
#else
#include <WiFi101.h>
#endif

extern char ssid[];
extern char pass[];

// WiFi client declaration
WiFiClient wifiClient;

// specific WiFi module getTime method
unsigned long getTime() {
  return WiFi.getTime();
}

void initConnection() {
  int attempts = 0;

  // check for the presence of the shield:
  if (WiFi.status() == NETWORK_HARDWARE_ERROR) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }
  // attempt to connect to WiFi network:

  while (status != NETWORK_CONNECTED && attempts++ < NETWORK_LAYER_CONNECTION_RETRIES) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(NETWORK_LAYER_CONNECTION_TIMEOUT);
  }

  if (status != NETWORK_CONNECTED) {
    Serial.println("Failed to connect to Wifi!");
    while (true);
  }

  // you're connected now, so print out the data:
  Serial.println("You're connected to the WiFi network");

  // begin of the ArduinoIoTCloud
  if (!ArduinoCloud.begin(wifiClient)) {
    Serial.println("Starting Arduino Cloud failed!");
    while (true);
  }

  // onGetTime callback assignment.
  ArduinoCloud.onGetTime(getTime);

  Serial.println("Attempting to connect to Arduino Cloud");
  attempts = 0;
  while (!ArduinoCloud.connect() && attempts++ < ARDUINO_IOT_CLOUD_CONNECTION_RETRIES) {
    Serial.print(".");
    delay(ARDUINO_IOT_CLOUD_CONNECTION_TIMEOUT);
  }

  if (attempts >= ARDUINO_IOT_CLOUD_CONNECTION_RETRIES) {
    Serial.println("\nFailed to connect to Arduino Cloud!");
    while (1);
  }

  Serial.println("\nSuccessfully connected to Arduino Cloud :)");

  CloudSerial.begin(9600);
}


// check network connection, if it is disconnected, re-connect it.
void checkNetworkConnection() {
  if (millis() - lastNetworkCheck > NETWORK_CONNECTION_INTERVAL) {
    Serial.print("<<Network Status: ");

    // Do nothing if wifi is connected
    if (WiFi.status() == NETWORK_CONNECTED) {
      // uopdate cheking time
      Serial.println("CONNECTED");
      lastNetworkCheck = millis();
      return;
    }
    // Check Wifi status, until it is connected (blocking!!!)
    while (WiFi.status() != NETWORK_CONNECTED) {
      Serial.print("..Reconnection to connect to WPA SSID: ");
      Serial.println(ssid);
      status = WiFi.begin(ssid, pass);
      // wait 10 seconds for connection:
      delay(NETWORK_CONNECTION_TIMEOUT);
    }

    Serial.println("..Reconnected to the Nework!");
    // Call the reconnect method to clean up the ArduinoCloud connection
    while (!ArduinoCloud.reconnect(wifiClient)) {
      delay(ARDUINO_IOT_CLOUD_CONNECTION_TIMEOUT);
    }

    Serial.println("..Reconnected to the Cloud!");

    delay(500);
    // uopdate cheking time
    lastNetworkCheck = millis();
  }
}

#endif
