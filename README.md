`ArduinoIoTCloud`
=================

[![Check Arduino status](https://github.com/arduino-libraries/ArduinoIoTCloud/actions/workflows/check-arduino.yml/badge.svg)](https://github.com/arduino-libraries/ArduinoIoTCloud/actions/workflows/check-arduino.yml)
[![Compile Examples](https://github.com/arduino-libraries/ArduinoIoTCloud/workflows/Compile%20Examples/badge.svg)](https://github.com/arduino-libraries/ArduinoIoTCloud/actions?workflow=Compile+Examples)
[![Spell Check](https://github.com/arduino-libraries/ArduinoIoTCloud/workflows/Spell%20Check/badge.svg)](https://github.com/arduino-libraries/ArduinoIoTCloud/actions?workflow=Spell+Check)
[![Unit Tests](https://github.com/arduino-libraries/ArduinoIoTCloud/workflows/Unit%20Tests/badge.svg)](https://github.com/arduino-libraries/ArduinoIoTCloud/actions?workflow=Unit+Tests)
[![codecov](https://codecov.io/gh/arduino-libraries/ArduinoIoTCloud/branch/master/graph/badge.svg)](https://codecov.io/gh/arduino-libraries/ArduinoIoTCloud)

### What?
The `ArduinoIoTCloud` library is the central element of the firmware enabling certain Arduino boards to connect to the [Arduino IoT Cloud](https://www.arduino.cc/en/IoT/HomePage). The following boards are supported:

* **WiFi**: [`MKR 1000`](https://store.arduino.cc/arduino-mkr1000-wifi), [`MKR WiFi 1010`](https://store.arduino.cc/arduino-mkr-wifi-1010), [`Nano 33 IoT`](https://store.arduino.cc/arduino-nano-33-iot), [`Portenta H7`](https://store.arduino.cc/portenta-h7), [`Nano RP2040 Connect`](https://store.arduino.cc/products/arduino-nano-rp2040-connect), [`Nicla Vision`](https://store.arduino.cc/products/nicla-vision), [`ESP8266`](https://github.com/esp8266/Arduino/releases/tag/2.5.0), [`ESP32`](https://github.com/espressif/arduino-esp32/releases/tag/1.0.6)
* **GSM**: [`MKR GSM 1400`](https://store.arduino.cc/arduino-mkr-gsm-1400-1415)
* **5G**: [`MKR NB 1500`](https://store.arduino.cc/arduino-mkr-nb-1500-1413)
* **LoRa**: [`MKR WAN 1300/1310`](https://store.arduino.cc/mkr-wan-1310)

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

#### Device can not subscribe to `THING_ID`
```
ArduinoIoTCloudTCP::handle_SubscribeMqttTopics could not subscribe to /a/t/xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx/e/i
```
In this case either the device has not been associated with the thing within the Arduino IoT Cloud GUI configuration or there's a typo in the thing ID.

#### OTA
OTA is supported by the following boards:
[`MKR WiFi 1010`](https://store.arduino.cc/arduino-mkr-wifi-1010), [`Nano 33 IoT`](https://store.arduino.cc/arduino-nano-33-iot), [`Portenta H7`](https://store.arduino.cc/portenta-h7), [`Nano RP2040 Connect`](https://store.arduino.cc/products/arduino-nano-rp2040-connect), [`Nicla Vision`](https://store.arduino.cc/products/nicla-vision)

#### ESP Boards
Support for ESP boards is obtained through a third-party core with some differences and limitations compared to Arduino boards.

- **Authentication scheme**: Board authentication is done through `DEVICE_LOGIN_NAME` and `DEVICE_KEY`, both values are included in the `thingProperties.h` file.
- **RTC**: RTC support is not included thus each `ArduinoCould.update()` call will lead to an NTP request introducing delay in your `loop()` function. The scheduler widget will not work correctly if connection is lost after configuration.
- **Watchdog**: Watchdog support is not included.
- **OTA**: OTA support is not included
