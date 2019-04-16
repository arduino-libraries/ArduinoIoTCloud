# ArduinoIoTCloud
[![Build Status](https://travis-ci.org/arduino-libraries/ArduinoIoTCloud.svg?branch=master)](https://travis-ci.org/arduino-libraries/ArduinoIoTCloud)

This library facilitates interactions between boards featuring a cryptography co-processor and the Arduino IoT Cloud service. It includes a ConnectionManager to handle connection/disconnection/reconnection flows and provides means to interface local sketch variables with cloud based Thing properties, enabling synchronization and on-change callbacks.
Currently supported boards: MKR1000 (WiFi101) and MKR 1010 (WiFiNINA). Support for MKRGSM is nearing completion and more cryptography enabled boards are following.

## Arduino IoT Cloud

[Arduino IoT Cloud](https://create.arduino.cc/iot) is a service which facilitates connections between cloud-based applications and the world around you. More information [is available here](https://www.arduino.cc/en/IoT/HomePage) and a detailed tutorial can be found on [Arduino Project Hub](https://create.arduino.cc/projecthub/133030/iot-cloud-getting-started-c93255).
The platform is currently in public beta, we appreciate any feedback provided.


### Arduino IoT Cloud Components

- **Devices**: Physical objects built around a board (e.g. MKR WiFi 1010). This is the hardware which runs the sketch, reads sensors, controls actuators and communicates with the Arduino IoT Cloud.

- **Things**: Logical representation of a connected object. They embody inherent properties of the object, with as little reference to the actual hardware or code used to implement them. Each Thing is represented by a collection of _Properties_ (e.g., temperature, light, pressure...).

- **Properties**: Qualities defining the characteristics of a system. A _Property_ can be defined as *read-only* (RO) to indicate that Arduino IoT Cloud can read the data, but cannot change the value of such _Property_. On the other end it may be designated to be **read-and-write** (RW), allowing Arduino IoT Cloud to remotely change the property’s value and trigger an event notification on the device to be handled via code.

- **Events**: When a physical event is triggered on the _Device_, Arduino IoT Cloud is made aware of it thanks to application messages. It might, for example, be notified that a proximity sensor detected someone or something outside a door.

- **Software**: An Arduino Create sketch is automatically generated by Arduino IoT Cloud when setting up a new thing: this simplifies starting efforts. Because the connection to the cloud is handled by the library, the user can focus on implementing the last bits of code required to handle the change of variables linked to properties.
The introductory tutorial linked above explains this in an easy and comprehensive way.

## ArduinoIoTCloud library

The library is made of multiple classes:
- `ArduinoIoTCloud` is the main class. It's responsible for the connection to the MQTT Broker and to Arduino IoT Cloud.
This library has multiple `begin(...)` methods allowing you to take more control of its behavior when it comes to network **Client** or to use a `ConnectionManager`

- `ConnectionManager` is an abstract Class defining methods to be implemented in derived classes, such as `WiFiConnectionManager`, `GSMConnectionManager` and so on. The right `ConnectionManager` is chosen on a board basis during compilation.

- `WiFiConnectionManager` handles connection, network time retrieval, disconnection, and reconnection to Internet for WiFi equipped boards (**MKR1000**, **MKR WIFI 1010** and upcoming implementations).

- `GSMConnectionManager` handles connection, network time retrieval, disconnection, and reconnection to Internet for GSM equipped boards (**MKR GSM 1400**)


- `CloudSerial` is similar to [Serial](https://www.arduino.cc/reference/en/language/functions/communication/serial/), but used in combination with the cloud, allowing the user to send and receive custom messages using Arduino IoT Cloud as a channel.


### ConnectionManager

**Connection Manager** is configured via a series of compiler directives, including the correct implementation of the class based on which board is selected.

### How to use it
- Instantiate the class with `ConnectionManager *ArduinoIoTPreferredConnection = new WiFiConnectionManager(SECRET_SSID, SECRET_PASS);` if you are using a WiFi board, otherwise replace **WiFi** with **GSM** or any future implementation.

- The `check()` method does all the work. It uses a finite state machine and is responsible for connection and reconnection to a network. The method is designed to be non-blocking by using time (milliseconds) to perform its tasks.

- `getTime()` returns different implementations of the `getTIme()` method based on the board used. Time is retrieved from an NTP server and is required for the SSL connection to the cloud.

- `&getClient()` returns a reference an instance of the `Client` class used to connect to the network.

- `getStatus()` returns the network connection status. The different states are defined in an `enum`

- `debugMessage(char *_msg, uint8_t _debugLevel, bool _timestamp = true, bool _newline = true)` is the method used to print debug messages on the physical serial. This helps providing troubleshooting information should anything go wrong.

- The `setDebugMessageLevel(int _debugLevel)` method is used to set a level of granularity in information output. Every debug message comes with a level which goes from 0 to 4. A higher level means more verbosity. Debug messages with level higher than `_debugLevel` will not be shown. The lowest level has a higher importance and is usually used for errors, which are always printed. Passing -1 as a parameter will disable logging entirely, **errors will also be ignored**.

### ArduinoIoTCloud

- The `begin(ConnectionManager *connection = ArduinoIoTPreferredConnection, String brokerAddress = "mqtts-sa.iot.arduino.cc")` method is used to initialize a connection to the Arduino IoT Cloud through MQTT. Accepts an instance of `ConnectionManager` and returns 0 in case of failure or 1 if object is successfully instantiated.

- `begin(Client& net, String brokerAddress = "mqtts-sa.iot.arduino.cc")` if connection is managed manually and not via a `ConnectionManager` a reference to a `Client` object can be used.

- The `connect()` method is used to connect to the MQTT broker.

- `disconnect()` closes the connection to the MQTT Client.

- The `update()` method can be called periodically in the loop of the `.ino` file. If a `ConnectionManager` is implemented it checks network connections. It also makes sure that a connection with the MQTT broker is active and tries to reconnect otherwise. During `update()` data from the Cloud is retrieved and changed values are posted to the proper MQTT topic.

- `connected()` simply returns the current status of the MQTT connection.

- `reconnect(Client&)` cleans up the existing MQTT connection, creates a new one and initializes it by calling `connect()`

- `setThingId(String const thing_id)` sets the **THING_ID** to properly configure the MQTT topic.

- `getThingId()` returns the **THING_ID**.

- `connectionCheck()` invokes the `check()` method from a **ConnectionManager** if it is implemented. Mainly it implements a state machine and is responsible for the connection to Arduino IoT Cloud.
