`ArduinoIoTCloud`
=================

[![Check Arduino status](https://github.com/arduino-libraries/ArduinoIoTCloud/actions/workflows/check-arduino.yml/badge.svg)](https://github.com/arduino-libraries/ArduinoIoTCloud/actions/workflows/check-arduino.yml)
[![Compile Examples](https://github.com/arduino-libraries/ArduinoIoTCloud/workflows/Compile%20Examples/badge.svg)](https://github.com/arduino-libraries/ArduinoIoTCloud/actions?workflow=Compile+Examples)
[![Spell Check](https://github.com/arduino-libraries/ArduinoIoTCloud/workflows/Spell%20Check/badge.svg)](https://github.com/arduino-libraries/ArduinoIoTCloud/actions?workflow=Spell+Check)
[![Unit Tests](https://github.com/arduino-libraries/ArduinoIoTCloud/workflows/Unit%20Tests/badge.svg)](https://github.com/arduino-libraries/ArduinoIoTCloud/actions?workflow=Unit+Tests)
[![codecov](https://codecov.io/gh/arduino-libraries/ArduinoIoTCloud/branch/master/graph/badge.svg)](https://codecov.io/gh/arduino-libraries/ArduinoIoTCloud)

### What?
The `ArduinoIoTCloud` library is the central element of the firmware enabling certain Arduino boards to connect to the [Arduino IoT Cloud](https://www.arduino.cc/en/IoT/HomePage). The following boards are supported:

* **WiFi**: [`MKR 1000`](https://store.arduino.cc/arduino-mkr1000-wifi), [`MKR WiFi 1010`](https://store.arduino.cc/arduino-mkr-wifi-1010), [`Nano 33 IoT`](https://store.arduino.cc/arduino-nano-33-iot), [`Portenta H7`](https://store.arduino.cc/portenta-h7), [`Nano RP2040 Connect`](https://store.arduino.cc/products/arduino-nano-rp2040-connect), [`Nicla Vision`](https://store.arduino.cc/products/nicla-vision), [`OPTA WiFi`](https://store.arduino.cc/products/opta-wifi), [`GIGA R1 WiFi`](https://store.arduino.cc/products/giga-r1-wifi), [`Portenta C33`](https://store.arduino.cc/products/portenta-c33), [`UNO R4 WiFi`](https://store.arduino.cc/products/uno-r4-wifi), [`Nano ESP32`](https://store.arduino.cc/products/nano-esp32), [`ESP8266`](https://github.com/esp8266/Arduino/releases/tag/2.5.0), [`ESP32`](https://github.com/espressif/arduino-esp32/releases/tag/2.0.5)
* **GSM**: [`MKR GSM 1400`](https://store.arduino.cc/arduino-mkr-gsm-1400-1415)
* **5G**: [`MKR NB 1500`](https://store.arduino.cc/arduino-mkr-nb-1500-1413)
* **LoRa**: [`MKR WAN 1300/1310`](https://store.arduino.cc/mkr-wan-1310)
* **Ethernet**: [`Portenta H7`](https://store.arduino.cc/products/portenta-h7) + [`Vision Shield Ethernet`](https://store.arduino.cc/products/arduino-portenta-vision-shield-ethernet), [`Max Carrier`](https://store.arduino.cc/products/portenta-max-carrier), [`Breakout`](https://store.arduino.cc/products/arduino-portenta-breakout), [`Portenta Machine Control`](https://store.arduino.cc/products/arduino-portenta-machine-control), [`OPTA WiFi`](https://store.arduino.cc/products/opta-wifi), [`OPTA RS485`](https://store.arduino.cc/products/opta-rs485), [`OPTA Lite`](https://store.arduino.cc/products/opta-lite), [`Portenta C33`](https://store.arduino.cc/products/portenta-c33) + [`Vision Shield Ethernet`](https://store.arduino.cc/products/arduino-portenta-vision-shield-ethernet)

### How?
1) Register your Arduino IoT Cloud capable board via [Arduino IoT Cloud](https://create.arduino.cc/iot) (Devices Section).
2) Create a new logical representation known as a [Thing](https://create.arduino.cc/iot/things).

### Arduino IoT Cloud Components

- **Devices**: Physical objects built around a board (e.g. `MKR WiFi 1010`). This is the hardware which runs the sketch, reads sensors, controls actuators and communicates with the Arduino IoT Cloud.

- **Things**: Logical representation of a connected object. They embody inherent properties of the object, with as little reference to the actual hardware or code used to implement them. Each Thing is represented by a collection of _Properties_ (e.g., temperature, light, pressure...).

- **Properties**: Qualities defining the characteristics of a system. A _Property_ can be defined as *read-only* (`READ`) to indicate that Arduino IoT Cloud can read the data, but cannot change the value of such _Property_. On the other end, it may be designated to be **read-and-write** (`READWRITE`), allowing Arduino IoT Cloud to remotely change the property’s value and trigger an event notification on the device. It is also possible to mark properties as *write-only* (`WRITE`) which means the cloud can write to this property but not read its value (this limits data transmission for properties which are used to trigger events only).

### How-to-use
* `thingProperties.h`
```C++
void onLedChange();
/* ... */
bool led;
int seconds;
/* ... */
void initProperties() {
  ArduinoCloud.setThingId("ARDUINO-THING-ID");
  ArduinoCloud.addProperty(led, WRITE, ON_CHANGE, onLedChange);
  ArduinoCloud.addProperty(seconds, READ, ON_CHANGE);
}
/* ... */
WiFiConnectionHandler ArduinoIoTPreferredConnection("SECRET_SSID", "SECRET_PASS");
```

* `MyCloudApplication.ino`
```C++
#include "thingProperties.h"

void setup() {
  Serial.begin(9600);
  while(!Serial) { }
  pinMode(LED_BUILTIN, OUTPUT);

  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
}

void loop() {
  ArduinoCloud.update();
  seconds = millis() / 1000;
}

void onLedChange() {
  digitalWrite(LED_BUILTIN, led);
}
```

### FAQ
#### Watchdog
The [1.0.0](https://github.com/arduino-libraries/ArduinoIoTCloud/releases/tag/1.0.0) release of this library adds watchdog functionality to all ATSAMD21G18 based cloud connected boards. A watchdog is simply an electronic timer counting down from a preset start value which, upon reaching zero, triggers a reset of the microcontroller. It can be used to automatically recover from temporary hardware faults or unrecoverable software errors. In order to avoid the watchdog from reaching zero the countdown timer needs to be regularly re-set to its start value. This is happening within `ArduinoCloud.update()` which is periodically called at the start of the `loop()` function. Although the watchdog is automatically enabled it can be disabled by setting the second parameter of `ArduinoCloud.begin(...)` to `false`: 
```C++
ArduinoCloud.begin(ArduinoIoTPreferredConnection, false).
```

Whatchdog is enabled by default using the following boards: [`MKR 1000`](https://store.arduino.cc/arduino-mkr1000-wifi), [`MKR WiFi 1010`](https://store.arduino.cc/arduino-mkr-wifi-1010), [`MKR GSM 1400`](https://store.arduino.cc/arduino-mkr-gsm-1400-1415), [`MKR NB 1500`](https://store.arduino.cc/arduino-mkr-nb-1500-1413), [`MKR WAN 1300/1310`](https://store.arduino.cc/mkr-wan-1310), [`Nano 33 IoT`](https://store.arduino.cc/arduino-nano-33-iot), [`Portenta H7`](https://store.arduino.cc/portenta-h7), [`Nano RP2040 Connect`](https://store.arduino.cc/products/arduino-nano-rp2040-connect), [`Nicla Vision`](https://store.arduino.cc/products/nicla-vision), [`OPTA WiFi`](https://store.arduino.cc/products/opta-wifi), [`OPTA RS485`](https://store.arduino.cc/products/opta-rs485), [`OPTA Lite`](https://store.arduino.cc/products/opta-lite), [`GIGA R1 WiFi`](https://store.arduino.cc/products/giga-r1-wifi)

#### Device can not subscribe to `THING_ID`
```
ArduinoIoTCloudTCP::handle_SubscribeMqttTopics could not subscribe to /a/t/xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx/e/i
```
In this case either the device has not been associated with the thing within the Arduino IoT Cloud GUI configuration or there's a typo in the thing ID.

#### OTA
OTA is supported by the following boards:
[`MKR WiFi 1010`](https://store.arduino.cc/arduino-mkr-wifi-1010), [`Nano 33 IoT`](https://store.arduino.cc/arduino-nano-33-iot), [`Portenta H7`](https://store.arduino.cc/portenta-h7), [`Nano RP2040 Connect`](https://store.arduino.cc/products/arduino-nano-rp2040-connect), [`Nicla Vision`](https://store.arduino.cc/products/nicla-vision), [`OPTA WiFi`](https://store.arduino.cc/products/opta-wifi), [`OPTA RS485`](https://store.arduino.cc/products/opta-rs485), [`OPTA Lite`](https://store.arduino.cc/products/opta-lite), [`GIGA R1 WiFi`](https://store.arduino.cc/products/giga-r1-wifi), [`Nano ESP32`](https://store.arduino.cc/products/nano-esp32), [`ESP32`](https://github.com/espressif/arduino-esp32/releases/tag/2.0.5)

#### Authentication
Boards can authenticate to the ArduinoIoTCloud servers using 3 methods:

 * `DEVICE_LOGIN_NAME` and `DEVICE_KEY`. This values are defined in the `thingProperties.h` file and included in the Sketch. Boards that are using this method are: [`UNO R4 WiFi`](https://store.arduino.cc/products/uno-r4-wifi), [`Nano ESP32`](https://store.arduino.cc/products/nano-esp32), [`ESP8266`](https://github.com/esp8266/Arduino/releases/tag/2.5.0), [`ESP32`](https://github.com/espressif/arduino-esp32/releases/tag/2.0.5)

 * `DEVICE_CERTIFICATE` and `PRIVATE_KEY`. This values are stored inside the board secure element during the device provisioning phase. Boards that are using this method are: [`MKR 1000`](https://store.arduino.cc/arduino-mkr1000-wifi), [`MKR WiFi 1010`](https://store.arduino.cc/arduino-mkr-wifi-1010), [`MKR GSM 1400`](https://store.arduino.cc/arduino-mkr-gsm-1400-1415), [`MKR NB 1500`](https://store.arduino.cc/arduino-mkr-nb-1500-1413), [`Nano 33 IoT`](https://store.arduino.cc/arduino-nano-33-iot), [`Portenta H7`](https://store.arduino.cc/portenta-h7), [`Nano RP2040 Connect`](https://store.arduino.cc/products/arduino-nano-rp2040-connect), [`Nicla Vision`](https://store.arduino.cc/products/nicla-vision), [`OPTA WiFi`](https://store.arduino.cc/products/opta-wifi), [`OPTA RS485`](https://store.arduino.cc/products/opta-rs485), [`OPTA Lite`](https://store.arduino.cc/products/opta-lite), [`GIGA R1 WiFi`](https://store.arduino.cc/products/giga-r1-wifi), [`Portenta C33`](https://store.arduino.cc/products/portenta-c33)

 * `APP_EUI` and `APP_KEY`. This values are defined in the `thingProperties.h` file and included in the Sketch. Boards that are using this method are: [`MKR WAN 1300/1310`](https://store.arduino.cc/mkr-wan-1310)

 