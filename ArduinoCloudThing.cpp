#include <Arduino.h>
#include <ArduinoCloudThing.h>

ArduinoCloudThing::ArduinoCloudThing() {

}

/*
 * begin() should prepare the environment
 * connect 
 */

static constexpr char* PLACEHOLDER = "placeholder";

void ArduinoCloudThing::begin(Client &client) {
    this->client = new MQTT::Client<ArduinoCloudThing, Network, Timer, MQTT_BUFFER_SIZE, 0>(network);
    this->network.setClient(&client);
    this->options = MQTTPacket_connectData_initializer;
    this->client->defaultMessageHandler.attach<ArduinoCloudThing>(this, &ArduinoCloudThing::callback);

    // using WiFi client and ECC508 connect to server
    while (!connect()) {
        Serial.println("Not connected");
        delay(500);
    }
}

bool ArduinoCloudThing::connect() {

#ifdef TESTING
    return true;
#endif

    if(!network.connect(PLACEHOLDER, 0xDEADBEEF)) {
        return false;
    }

    options.clientID.cstring = PLACEHOLDER;
    options.username.cstring = PLACEHOLDER;
    options.password.cstring = PLACEHOLDER;
    options.keepAliveInterval = 10;
    options.willFlag = 0x1;
    options.will.topicName.cstring = PLACEHOLDER;
    options.will.message.cstring = PLACEHOLDER;
    options.will.retained = 0x1;

    if (client->connect(options) != 0) {
        // set status to ON
        status = ON;
        addProperty(status, WRITE);
        // execute first poll() to syncronize the "manifest"
        poll();
        // subscribe to "general" topic
        client->subscribe("placeholder", MQTT::QOS0, NULL);
        return true;
    }

    return false;
}

void ArduinoCloudThing::publish() {

    bool retained = false;

    CborDynamicOutput output;
    CborWriter writer(output);

    writer.writeTag(1);

    for (int i = 0; i < list.size(); i++) {
        ArduinoCloudPropertyGeneric *p = list.get(i);
        p->append(writer);
    }

    unsigned char *buf = output.getData();

    decodeCBORData(buf, output.getSize());

    MQTT::Message message;
    message.qos = MQTT::QOS0;
    message.retained = retained;
    message.dup = false;
    message.payload = (void*)buf;
    message.payloadlen = output.getSize();
    client->publish(topic, message);
}

// Reconnect to the mqtt broker
int ArduinoCloudThing::poll() {

#ifndef TESTING
    if (!client->isConnected()){
        if (!connect()){
            return 0;
        }
    }
    if(!network.connected() && client->isConnected()) {
      client->disconnect();
    }
#endif

    // check if backing storage and cloud has diverged
    int diff_in = 0;
    int diff_out = 1;
    checkNewData(&diff_in , &diff_out);
    if (diff_out > 0) {
        publish();
    }
    if (diff_in > 0) {
        update();
    }
    client->yield();
    return diff_in;
}

void ArduinoCloudThing::checkNewData(int* new_data_in, int* new_data_out) {

}

void ArduinoCloudThing::addPropertyReal(int& property, String name, permissionType permission) {
    ArduinoCloudProperty<int> *thing = new ArduinoCloudProperty<int>(property, name, permission);
    list.add(thing);
}

void ArduinoCloudThing::addPropertyReal(bool& property, String name, permissionType permission) {
    ArduinoCloudProperty<bool> *thing = new ArduinoCloudProperty<bool>(property, name, permission);
    list.add(thing);
}

void ArduinoCloudThing::addPropertyReal(float& property, String name, permissionType permission) {
    ArduinoCloudProperty<float> *thing = new ArduinoCloudProperty<float>(property, name, permission);
    list.add(thing);
}


ArduinoCloudThing ArduinoCloudThing::callback(MQTT::MessageData& messageData) {
    MQTT::Message &message = messageData.message;
    // null terminate topic to create String object

    // unwrap message into a structure
    uint8_t * payload = (uint8_t*)message.payload;
    decodeCBORData(payload, message.payloadlen);

    return *this;
}

void ArduinoCloudThing::decodeCBORData(uint8_t * payload, size_t length) {
    list_shadow.clear();

    CborInput input(payload, length);

    CborReader reader(input);
    CborPropertyListener listener(list_shadow);
    reader.SetListener(listener);
    reader.Run();
}

void CborPropertyListener::OnInteger(int32_t value) {
    printf("integer: %d\n", value);
}

void CborPropertyListener::OnBytes(unsigned char *data, unsigned int size) {
    printf("bytes with size: %d", size);
}

void CborPropertyListener::OnString(String &str) {
    printf("string: '%.*s'\n", (int)str.length(), str.c_str());
}

void CborPropertyListener::OnArray(unsigned int size) {
    printf("array: %d\n", size);
}

void CborPropertyListener::OnMap(unsigned int size) {
    printf("map: %d\n", size);
}

void CborPropertyListener::OnTag(unsigned int tag) {
    printf("tag: %d\n", tag);
}

void CborPropertyListener::OnSpecial(unsigned int code) {
    printf("special: %d\n", code);
}

void CborPropertyListener::OnError(const char *error) {
    printf("error: %s\n", error);
}