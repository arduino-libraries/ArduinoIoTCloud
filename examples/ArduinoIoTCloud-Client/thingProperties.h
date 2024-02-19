#if defined(BOARD_HAS_WIFI)
  WiFiClient client;
  WiFiUDP udp;
#elif defined(BOARD_HAS_GSM)
#elif defined(BOARD_HAS_LORA)
#elif defined(BOARD_HAS_NB)
#elif defined(BOARD_HAS_ETHERNET)
#else
  #error "Please check Arduino IoT Cloud supported boards list: https://github.com/arduino-libraries/ArduinoIoTCloud/#what"
#endif

#if defined(BOARD_HAS_SECRET_KEY)
  #define BOARD_ID ""
#endif

void onLedChange();

bool led;
int potentiometer;
int seconds;

void initProperties() {
#if defined(BOARD_HAS_SECRET_KEY)
  ArduinoCloud.setBoardId(BOARD_ID);
  ArduinoCloud.setSecretDeviceKey(SECRET_DEVICE_KEY);
#endif
#if defined(BOARD_HAS_WIFI) || defined(BOARD_HAS_GSM) || defined(BOARD_HAS_NB) || defined(BOARD_HAS_ETHERNET)
  ArduinoCloud.addProperty(led, Permission::Write).onUpdate(onLedChange);
  ArduinoCloud.addProperty(potentiometer, Permission::Read).publishOnChange(10);
  ArduinoCloud.addProperty(seconds, Permission::Read).publishOnChange(1);
#elif defined(BOARD_HAS_LORA)
  ArduinoCloud.addProperty(led, 1, READWRITE, ON_CHANGE, onLedChange);
  ArduinoCloud.addProperty(potentiometer, 2, READ, ON_CHANGE);
  ArduinoCloud.addProperty(seconds, 3, READ, 5 * MINUTES);
#endif
}
