#if !defined(ARDUINO_SAMD_MKRWIFI1010) && !defined(ARDUINO_SAMD_NANO_33_IOT) &&  !defined(ARDUINO_NANO_RP2040_CONNECT) \
  && !defined(ARDUINO_PORTENTA_H7_M7) && !defined(ARDUINO_NICLA_VISION) && !defined(ARDUINO_OPTA) && !defined(ARDUINO_GIGA) \
  && !defined(ARDUINO_UNOR4_WIFI) && !defined(ARDUINO_PORTENTA_C33)
#error "This example is not compatible with this board."
#endif
#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>
#include "ConfiguratorAgents/agents/BLE/BLEAgent.h"
#include "ConfiguratorAgents/agents/Serial/SerialAgent.h"

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
  ArduinoCloud.setConfigurator(NetworkConfigurator);
#if defined(ARDUINO_OPTA)
  ArduinoCloud.setReconfigurePin(BTN_USER, INPUT);
#endif

  ArduinoCloud.addProperty(led, Permission::Write).onUpdate(onLedChange);
  ArduinoCloud.addProperty(potentiometer, Permission::Read).publishOnChange(10);
  ArduinoCloud.addProperty(seconds, Permission::Read).publishOnChange(1);

}
