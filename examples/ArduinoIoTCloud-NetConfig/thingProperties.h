#if !defined(ARDUINO_SAMD_MKRWIFI1010) && !defined(ARDUINO_SAMD_NANO_33_IOT) &&  !defined(ARDUINO_NANO_RP2040_CONNECT) \
  && !defined(ARDUINO_PORTENTA_H7_M7) && !defined(ARDUINO_NICLA_VISION) && !defined(ARDUINO_OPTA) && !defined(ARDUINO_GIGA) \
  && !defined(ARDUINO_UNOR4_WIFI) && !defined(ARDUINO_PORTENTA_C33)
#error "This example is not compatible with this board."
#endif
#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>
#include <GenericConnectionHandler.h>
#include <Arduino_NetworkConfigurator.h>
#include <configuratorAgents/agents/BLEAgent.h>
#include <configuratorAgents/agents/SerialAgent.h>

void onLedChange();

bool led;
int potentiometer;
int seconds;

GenericConnectionHandler ArduinoIoTPreferredConnection;
KVStore kvStore;
NetworkConfiguratorClass NetworkConfigurator(ArduinoIoTPreferredConnection);
BLEAgentClass BLEAgent;
SerialAgentClass SerialAgent;

void initProperties() {
  NetworkConfigurator.addAgent(BLEAgent);
  NetworkConfigurator.addAgent(SerialAgent);
  NetworkConfigurator.setStorage(kvStore);

  /* For changing the default reset pin uncomment and set your preferred pin.
   * Use DISABLE_PIN for disabling the reset procedure.
   * The pin must be in the list of digital pins usable for interrupts.
   * Please refer to the Arduino documentation for more details:
   * https://docs.arduino.cc/language-reference/en/functions/external-interrupts/attachInterrupt/
   */
  //NetworkConfigurator.setReconfigurePin(your_pin);

  /* Otherwise if you need to monitor the pin status changes
   * you can set a custom callback function that is fired on every change
   */
  //NetworkConfigurator.setPinChangedCallback(your_callback);

  ArduinoCloud.setConfigurator(NetworkConfigurator);

  ArduinoCloud.addProperty(led, Permission::Write).onUpdate(onLedChange);
  ArduinoCloud.addProperty(potentiometer, Permission::Read).publishOnChange(10);
  ArduinoCloud.addProperty(seconds, Permission::Read).publishOnChange(1);

}
