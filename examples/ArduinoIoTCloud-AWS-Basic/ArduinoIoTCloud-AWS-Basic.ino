#include "arduino_secrets.h"
/*
  This sketch demonstrates how to connect to ArduinoIoTCloud and AWS IoT core.

  The full list of compatible boards can be found here:
   - https://github.com/arduino-libraries/ArduinoIoTCloud#what
*/

#include "thingProperties.h"
#include "aws_secrets.h"

Client& getDefaultClient() {
  switch(ArduinoIoTPreferredConnection.getInterface()) {

#ifdef BOARD_HAS_WIFI
    case NetworkAdapter::WIFI:
    static WiFiClient wclient;
    return wclient;
#endif

#ifdef BOARD_HAS_ETHERNET
    case NetworkAdapter::ETHERNET:
    static EthernetClient eclient;
    return eclient;
#endif

    default:
    Serial.println("Error: could not create default AWS client");
    break;
  }
}

unsigned long publishMillis = 0;
unsigned long connectMillis = 0;

BearSSLClient sslClientAWS(getDefaultClient());
MqttClient mqttClientAWS(sslClientAWS);

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
