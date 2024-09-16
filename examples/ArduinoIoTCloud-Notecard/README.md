Notecard Connectivity
=====================

The Notecard is a wireless, secure abstraction for device connectivity, that can
be used to enable _ANY*_ device with I2C, or UART, to connect to the Arduino IoT
Cloud via cellular, LoRa, satellite or Wi-Fi!

As a result, your existing device architecture can now have first class support
in the Arduino IoT Cloud, by using a Notecard as a secure communication channel.

> \*_While any device with I2C/UART may use the Notecard, the Arduino IoT Cloud
> library is not supported by the AVR toolchain. Therefore, devices based on the
> AVR architecture cannot access the Arduino IoT Cloud via the Notecard._
>
> _However, any device (including AVR), may use the Notecard library to send data
> to Notehub, then that data may be routed to any endpoint of your choosing. See the
> [Notecard Routing Guide](https://dev.blues.io/guides-and-tutorials/routing-data-to-cloud)
> for more information..._

Wireless Connectivity Options
-----------------------------

- [Cellular](https://shop.blues.com/collections/notecard/products/notecard-cellular)
- [Cellular + Wi-Fi](https://shop.blues.com/collections/notecard/products/notecard-cell-wifi)
- [Wi-Fi](https://shop.blues.com/collections/notecard/products/wifi-notecard)
- [LoRa](https://shop.blues.com/collections/notecard/products/notecard-lora)
- [Satellite](https://shop.blues.com/products/starnote)

How it Works
------------

**Architecture Diagram:**

```
--------                ------------                  -----------           -----------
|      |                |          |                  |         |           |         |
| Host |                |          |      Secure      |         |           | Arduino |
| MCU  |---<I2C/UART>---| Notecard | ( ( Wireless ) ) | Notehub |---<TLS>---|   IoT   |
|      |                |          |     Protocol     |         |           |  Cloud  |
|______|                |__________|                  |_________|           |_________|
```

Getting Started
---------------

### Setup a Notehub Account

Using the Notecard only requires a couple of easy steps:

1. [Purchase a Notecard](https://shop.blues.com/collections/notecard) (and
[Notecarrier](https://shop.blues.com/collections/notecarrier)) that fits the
needs of your device.
   > _**NOTE:** We recommend starting with our [Dev Kit](https://shop.blues.com/products/blues-global-starter-kit)
   > if you are unsure._
1. [Setup a Notehub account](https://dev.blues.io/quickstart/notecard-quickstart/notecard-and-notecarrier-f/#set-up-notehub).
   > _**NOTE:** Notehub accounts are free (no credit card required)._
1. [Create a project on your Notehub account](https://dev.blues.io/quickstart/notecard-quickstart/notecard-and-notecarrier-f/#create-a-notehub-project).
1. In `thingProperties.h`, replace "com.domain.you:product" (from
`NOTECARD_PRODUCT_UID`) with the ProductUID of your new Notehub project.

### Power-up the Device

1. [Connect the Notecard to your Host MCU](https://dev.blues.io/quickstart/notecard-quickstart/notecard-and-notecarrier-f/#connect-your-notecard-and-notecarrier)
1. Flash the `ArduinoIoTCloud-Notecard` example sketch to your device. You
should see the device reporting itself as online in your [Notehub Project](https://notehub.io).

### Associate Notecard to Arduino IoT Cloud

1. Create a "MANUAL Device" in the Arduino IoT Cloud, then [add environment
variables for the "Device ID" and "Secret Key" to your Notecard in Notehub](https://dev.blues.io/guides-and-tutorials/notecard-guides/understanding-environment-variables/#setting-a-notehub-device-variable).
   - `_sn`: \<Device ID\>
   - `_secret_key`: \<Secret Key\>

### More Information

For more information about the Notecard and Notehub in general, please see our
[Quickstart Guide](https://dev.blues.io/quickstart/) for a general overview of
how the Notecard and Notehub are designed to work.
