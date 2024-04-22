/*
  This sketch demonstrates how to exchange data between your board and the Arduino IoT Cloud.

  * Connect a potentiometer (or other analog sensor) to A0.
  * When the potentiometer (or sensor) value changes the data is sent to the Cloud.
  * When you flip the switch in the Cloud dashboard the onboard LED lights gets turned ON or OFF.

  IMPORTANT:
  This sketch works with WiFi, GSM, NB, Ethernet and Lora enabled boards supported by Arduino IoT Cloud.
  On a LoRa board, if it is configured as a class A device (default and preferred option),
  values from Cloud dashboard are received only after a value is sent to Cloud.

  The full list of compatible boards can be found here:
   - https://github.com/arduino-libraries/ArduinoIoTCloud#what
*/

#include "thingProperties.h"

unsigned long publishMillis = 0;
unsigned long connectMillis = 0;

void setup() {
  /* Initialize serial and wait up to 5 seconds for port to open */
  Serial.begin(9600);

  /* Configure LED pin as an output */
  pinMode(LED_BUILTIN, OUTPUT);

  /* This function takes care of connecting your sketch variables to the ArduinoIoTCloud object */
  initProperties();

  /* Initialize Arduino IoT Cloud library */
  ArduinoCloud.begin(ArduinoIoTPreferredConnection, true, "iot.arduino.cc");

  setDebugMessageLevel(5);
  ArduinoCloud.printDebugInfo();

  /* Initialize AWS Client */
  ArduinoBearSSL.onGetTime(getTime);
  sslClientAWS.setEccSlot(AWS_SLOT, AWS_CERTIFICATE);

  mqttClientAWS.setId("ArduinoAWSClient");
  mqttClientAWS.onMessage(onMessageReceived);
  mqttClientAWS.setConnectionTimeout(10 * 1000);
  mqttClientAWS.setKeepAliveInterval(30 * 1000);
  mqttClientAWS.setCleanSession(false);
}

void loop() {
  ArduinoCloud.update();
  potentiometer = analogRead(A0);
  seconds = millis() / 1000;

  if (!ArduinoCloud.connected()) {
    return;
  }

  if (AWSIoTPreferredConnection.check() != NetworkConnectionState::CONNECTED) {
    return;
  }

  if (!mqttClientAWS.connected()) {
    if (millis() - connectMillis > 5000) {
      connectMillis = millis();
      // MQTT client is disconnected, connect
      if (!connectMQTT()) {
        return;
      }
    } else {
      return;
    }
  }

  // poll for new MQTT messages and send keep alive
  mqttClientAWS.poll();

  // publish a message roughly every 5 seconds.
  if (millis() - publishMillis > 5000) {
    publishMillis = millis();

    publishMessage();
  }
}

/*
 * 'onLedChange' is called when the "led" property of your Thing changes
 */
void onLedChange() {
  Serial.print("LED set to ");
  Serial.println(led);
  digitalWrite(LED_BUILTIN, led);
}

void onMessageReceived(int messageSize)
{
  // we received a message, print out the topic and contents
  Serial.print("Received a message with topic '");
  Serial.print(mqttClientAWS.messageTopic());
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  for (int i = 0; i < messageSize; i++) {
    const char c = mqttClientAWS.read();
    Serial.print(c);
  }
  Serial.println();
}

int connectMQTT() {
  Serial.print("Attempting to connect to MQTT broker: ");
  Serial.print(AWS_BROKER);
  Serial.println(" ");

  if (!mqttClientAWS.connect(AWS_BROKER, 8883)) {
    // failed, retry
    Serial.print(".");
    return 0;
  }
  Serial.println();

  Serial.println("You're connected to the MQTT broker");
  Serial.println();

  // subscribe to a topic
  mqttClientAWS.subscribe("arduino/incoming");
  return 1;
}

void publishMessage() {
  Serial.println("Publishing message");

  // send message, the Print interface can be used to set the message contents
  mqttClientAWS.beginMessage("arduino/outgoing");
  mqttClientAWS.print("hello ");
  mqttClientAWS.print(millis());
  mqttClientAWS.endMessage();
}
