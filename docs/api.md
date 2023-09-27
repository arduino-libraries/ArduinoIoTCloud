## ArduinoCloud Class (Base)

### `ArduinoCloud (Class)`

#### Description

`ArduinoIoTCloud` is a class for interfacing with the Arduino IoT Cloud. It provides an interface for managing IoT Cloud connectivity, updating data, and handling events.

Depending on what type of connection is used (TCP/LPWAN), either the `ArduinoIoTCloudTCP` or `ArduinoIoTCloudLPWAN` base classes are initialized. This specification is done in the Arduino IoT Cloud's "Thing" interface, and is reflected in the automatically generated `thingProperties.h` file. 

The above classes are inherited from the `ArduinoIoTCloudClass` base class, with methods used depending on what connection is used.

#### Syntax

```
ArduinoCloud.method()
```

### `push()`

#### Description

Pushes data to the IoT Cloud.

#### Syntax

```
ArduinoCloud.push()
```

#### Parameters

None.

#### Returns

Nothing.

### `setTimestamp()`

Sets a timestamp for a cloud variable.

#### Syntax

```
ArduinoCloud.setTimestamp(cloudVariable, timestamp)
```

#### Parameters

- `String` - property name
- `unsigned long const` - timestamp

### `setThingId()`

Sets the Thing ID.

#### Syntax

```
ArduinoCloud.setThingId(thingId)
```

#### Parameters
- `String` - your Thing's ID. Obtainable in the IoT Cloud interface. 

### `String & getThingId()`

Gets the Thing ID.

#### Syntax

#### Parameters

### `setDeviceId()`

Sets the Device ID.

#### Syntax

```
ArduinoCloud.setDeviceId(deviceId)
```

#### Parameters

- `String` - Device ID.

### `getDeviceId()`

Gets the Device ID.

#### Syntax

```
ArduinoCloud.getDeviceId()
```

#### Parameters

None

#### Returns

- `String` - Device ID.

### `setThingIdOutdatedFlag()`

Sets a flag indicating that the Thing ID is outdated.

#### Syntax

```
ArduinoCloud.setThingIdOutdatedFlag()
```

#### Parameters
None.

#### Returns
Nothing.

### `clrThingIdOutdatedFlag()`

Clears the flag indicating that the Thing ID is outdated.

#### Syntax

```
ArduinoCloud.clrThingIdOutdatedFlag()
```

#### Parameters
None.

#### Returns
Nothing.

### `getThingIdOutdatedFlag()`

Gets the flag indicating whether the Thing ID is outdated.

#### Syntax

```
ArduinoCloud.getThingOutdatedFlag()
```

#### Parameters
None.

#### Returns
- Boolean (true/false)

### `deviceNotAttached()`

Checks if the device is not attached.

#### Syntax

```
ArduinoCloud.deviceNotAttached()
```

#### Parameters
None.

#### Returns

- Boolean (true/false)

### `getConnection()`

Gets the connection handler used.

#### Syntax

```
ArduinoCloud.getConnection()
```

#### Parameters

### `getInternalTime()`

Gets the internal time.

#### Syntax

```
ArduinoCloud.getInternalTime()
```

#### Parameters
None.

#### Returns
- `unsigned long` - internal time

### `getLocalTime()`

Gets the local time (based on your time zone).

#### Syntax

```
ArduinoCloud.getLocalTime()
```

#### Parameters
None

#### Returns
- `unsigned long` - local time


### `updateInternalTimezoneInfo()`

Updates the internal timezone information.

#### Syntax

#### Parameters

### `addCallback(ArduinoIoTCloudEvent const event, OnCloudEventCallback callback)`

Adds a callback function for IoT Cloud events.

#### Syntax

```
ArduinoCloud.addCallback(ArduinoIoTCloudEvent::CONNECT, doThisOnConnect);
ArduinoCloud.addCallback(ArduinoIoTCloudEvent::SYNC, doThisOnSync);
ArduinoCloud.addCallback(ArduinoIoTCloudEvent::DISCONNECT, doThisOnDisconnect);
```

#### Parameters
- Event (CONNECT, SYNC, DISCONNECT)
- Callback function

#### Example 

See the [ArduinoIoTCloud-Callbacks](https://github.com/arduino-libraries/ArduinoIoTCloud/blob/master/examples/ArduinoIoTCloud-Callbacks/ArduinoIoTCloud-Callbacks.ino) example.

<!-- TCP documentation begins here -->

## ArduinoCloud Class (TCP)

### `begin()` (TCP)

#### Description

Initialises the library with the connection handler specified in `thingProperties.h`. This is automatically generated based on the type of device.

#### Syntax

```
ArduinoCloud.begin()
```

#### Parameters

The `ArduinoIoTPreferredConnection` object is created automatically inside a `thingProperties.h` file when configuring a Thing and depends on what type of device you use. The connection handler classes are not part of the Arduino IoT Cloud, but exists inside the [Arduino_ConnectionHandler](https://github.com/arduino-libraries/Arduino_ConnectionHandler) library (listed as a dependency).

For example, using the `WiFiConnectionHandler`, we use the `SSID`, `PASS` parameters which are defined in the `arduino_secrets.h` file, also generated based on what you input in the Arduino IoT Cloud interface.

```
WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);
```

#### Returns
Nothing.

### `update()` (TCP)

#### Description

**Note:** for LoRa devices, the `update()` function is implemented differently. See 

Responsible for updating the IoT Cloud connection. 
- First it feeds a watchdog timer, making sure it is not stuck anywhere,
- then sends variable updates to the IoT Cloud (via MQTT client)
- finally, it checks for new data from the IoT Cloud (via the MQTT client)

Any IoT Cloud sketch needs to continuously call the `update()` function, as it will otherwise time out and reset. The `update()` is by default placed inside of the `loop()`.

**Note:** do not use the `delay()` function in the sketch, as the watchdog timer will cause the board to reset and attempt to reconnect.

#### Syntax

```
ArduinoCloud.update()
```

#### Parameters
None.

#### Returns
Nothing.


### `connected()` (TCP)

#### Description

Checks the connection status and returns an integer the connection status (int).

#### Syntax

```
ArduinoCloud.connected()
```

#### Parameters
None.

#### Returns
- `int` - status of connection

### `printDebugInfo()` (TCP)

#### Description

Print any available debug information.

#### Parameters
None

#### Returns
Nothing.

### `setBoardId()`

#### Description

This method is only enabled if you are using an ESP32/ESP8266 board, and sets the board/device id.

#### Syntax

```
ArduinoCloud.setBoardId(DEVICE_LOGIN_NAME);
```

### `setSecretDeviceKey()`

#### Description

This method is only enabled if you are using an ESP32/ESP8266 based board, and sets the secret device key for the device. The secret key is only obtainable from the Arduino IoT Cloud during device configuration.

#### Syntax

```
ArduinoCloud.setSecretDeviceKey(DEVICE_KEY);
```

### `getBrokerAddress()`

#### Description

Get the MQTT broker address used.

#### Syntax

```
ArduinoCloud.getBrokerAddress()
```

#### Parameters
None.

#### Return
- `String` - the MQTT broker address.

### `getBrokerPort()`

#### Description

Get the MQTT broker port used.

#### Syntax

```
ArduinoCloud.getBrokerPort()
```

#### Parameters
None.

#### Return
- `int` - the MQTT broker port.

<!-- LPWAN documentation begins here -->

## ArduinoCloud Class (LPWAN)

### `begin()` (LPWAN)

### Description
This function initializes the ArduinoIoTCloudLPWAN library with the specified connection handler and retry settings.

#### Parameters
- `ConnectionHandler` - object containing `APPEUI`, `APPKEY` and frequency band (e.g. `_lora_band::EU868`)
- `bool` - a boolean value that decides whether retry functionality is enabled (`true`) or disabled (`false`)

#### Returns

- `int` - returns `1` on successful initialization.

### `update()` (LPWAN)

#### Description

**Note:** for TCP devices, the `update()` function is implemented differently. See [ArduinoCloud Class (TCP)](). 

Responsible for updating the IoT Cloud connection. 
- First it feeds a watchdog timer, making sure it is not stuck anywhere,
- then sends variable updates to the IoT Cloud (via MQTT client)
- finally, it checks for new data from the IoT Cloud (via the MQTT client)

Any IoT Cloud sketch needs to continuously call the `update()` function, as it will otherwise time out and reset. The `update()` is by default placed inside of the `loop()`.

**Note:** do not use the `delay()` function in the sketch, as the watchdog timer will cause the board to reset and attempt to reconnect.

#### Syntax

```
ArduinoCloud.update()
```

#### Parameters
None.

#### Returns
Nothing.


### `connected()` (LPWAN)

#### Description

Checks the connection status and returns an integer the connection status (int).

#### Syntax

```
ArduinoCloud.connected()
```

#### Parameters
None.

#### Returns
- `int` - status of connection

### `printDebugInfo()` (LPWAN)

#### Description

Print any available debug information.

#### Parameters
None.

#### Returns
Nothing.

## `isRetryEnabled()`

#### Description
This method is used to check whether retry functionality is enabled.

#### Parameters
None.

#### Returns
- `bool` - returns `true` if retry functionality is enabled, otherwise returns `false`.


### `getMaxRetry()`

#### Description
This method is used to retrieve the maximum number of retry attempts.

#### Parameters
None.

#### Returns
- `int` - returns the maximum number of retry attempts as an integer.

### `getIntervalRetry()`

#### Description
This method is used to retrieve the interval in milliseconds between the retry attempts.

#### Parameters
None.

#### Returns
- `long` - returns the interval between retry attempts in milliseconds. 

### `enableRetry()`

#### Description
This method is used to enable or disable retry functionality.

#### Parameters
- `bool` - if `true`, retry functionality is enabled; if `false`, retry functionality is disabled.

#### Returns
Nothing.

### `setMaxRetry()`

Used to set the maximum number of retry attempts.

#### Parameters
- `int` - the maximum number of retry attempts to set.

#### Returns
Nothing.

### `setIntervalRetry()`

#### Description
Sets the interval in milliseconds between retry attempts.

#### Parameters
- `long` - the interval between retry attempts in milliseconds to set.

#### Returns
Nothing.