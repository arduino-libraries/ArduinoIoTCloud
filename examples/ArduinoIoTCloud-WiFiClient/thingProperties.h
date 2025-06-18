#include <ArduinoIoTCloud.h>
#include "arduino_secrets.h"

#if !defined(HAS_TCP)
  #error  "Please check Arduino IoT Cloud supported boards list: https://github.com/arduino-libraries/ArduinoIoTCloud/#what"
#endif

#if !defined(BOARD_HAS_SECURE_ELEMENT)
  #define BOARD_ID "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
#endif

void onLedChange();

bool led;
int potentiometer;
int seconds;

void initProperties() {
  ArduinoCloud.addProperty(led, Permission::Write).onUpdate(onLedChange);
  ArduinoCloud.addProperty(potentiometer, Permission::Read).publishOnChange(10);
  ArduinoCloud.addProperty(seconds, Permission::Read).publishOnChange(1);

#if !defined(BOARD_HAS_SECURE_ELEMENT)
  ArduinoCloud.setBoardId(BOARD_ID);
  ArduinoCloud.setSecretDeviceKey(SECRET_DEVICE_KEY);
#endif
}

