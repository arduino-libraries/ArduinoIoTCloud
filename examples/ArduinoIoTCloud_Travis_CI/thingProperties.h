/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

#if defined(BOARD_HAS_WIFI)
#elif defined(BOARD_HAS_GSM)
#elif defined(BOARD_HAS_LORA)
#elif defined(BOARD_HAS_NB)
#else
  #error "Arduino IoT Cloud currently only supports MKR1000, MKR WiFi 1010, MKR WAN 1300/1310, MKR NB 1500 and MKR GSM 1400"
#endif

/******************************************************************************
   DEFINES
 ******************************************************************************/

#define THING_ID "ARDUINO_IOT_CLOUD_THING_ID"

/******************************************************************************
   GLOBAL CONSTANTS
 ******************************************************************************/

int   const MIN_DELTA_INT_PROPERTY   = 5;
float const MIN_DELTA_FLOAT_PROPERTY = 10.0f;

/******************************************************************************
   GLOBAL VARIABLES
 ******************************************************************************/

bool bool_property_1;
bool bool_property_2;

int int_property_1;
int int_property_2;
int int_property_3;
int int_property_4;
int int_property_5;
int int_property_6;

float float_property_1;
float float_property_2;
float float_property_3;
float float_property_4;

String str_property_1;
String str_property_2;
String str_property_3;
String str_property_4;
String str_property_5;
String str_property_6;
String str_property_7;
String str_property_8;


#if defined(BOARD_HAS_WIFI)
  WiFiConnectionHandler ArduinoIoTPreferredConnection(SECRET_SSID, SECRET_PASS);
#elif defined(BOARD_HAS_GSM)
  GSMConnectionHandler ArduinoIoTPreferredConnection(SECRET_PIN, SECRET_APN, SECRET_LOGIN, SECRET_PASS);
#elif defined(BOARD_HAS_LORA)
  LoRaConnectionHandler ArduinoIoTPreferredConnection(SECRET_APP_EUI, SECRET_APP_KEY, EU868);
#elif defined(BOARD_HAS_NB)
  NBConnectionHandler ArduinoIoTPreferredConnection(SECRET_PIN, SECRET_APN, SECRET_LOGIN, SECRET_PASS);
#endif

/******************************************************************************
   PROTOTYPES
 ******************************************************************************/

void onBoolPropertyChange();
void onIntPropertyChange();
void onFloatPropertyChange();
void onStringPropertyChange();

/******************************************************************************
   FUNCTIONS
 ******************************************************************************/
#if defined(BOARD_HAS_WIFI) || defined(BOARD_HAS_GSM) || defined (BOARD_HAS_NB)
void initProperties() {
  ArduinoCloud.setThingId(THING_ID);

  ArduinoCloud.addProperty(bool_property_1,  READWRITE, 1 * SECONDS);
  ArduinoCloud.addProperty(int_property_1,   READ,      2 * MINUTES);
  ArduinoCloud.addProperty(float_property_1, WRITE,     3 * HOURS);
  ArduinoCloud.addProperty(str_property_1,   READWRITE, 4 * DAYS);

  ArduinoCloud.addProperty(bool_property_2,  Permission::ReadWrite).publishEvery(1 * SECONDS);
  ArduinoCloud.addProperty(int_property_2,   Permission::Read).publishEvery(1 * MINUTES);
  ArduinoCloud.addProperty(float_property_2, Permission::Write).publishEvery(3 * HOURS);
  ArduinoCloud.addProperty(str_property_2,   Permission::ReadWrite).publishEvery(4 * DAYS);

  ArduinoCloud.addProperty(int_property_3,   READWRITE, ON_CHANGE);                       /* Default 'minDelta' = 0 */
  ArduinoCloud.addProperty(int_property_4,   READWRITE, ON_CHANGE, onIntPropertyChange);  /* Default 'minDelta' = 0 */
  ArduinoCloud.addProperty(int_property_5,   READWRITE, ON_CHANGE, 0 /* onIntPropertyChange */, MIN_DELTA_INT_PROPERTY);
  ArduinoCloud.addProperty(int_property_6,   READWRITE, ON_CHANGE, onIntPropertyChange,         MIN_DELTA_INT_PROPERTY);

  ArduinoCloud.addProperty(float_property_3, Permission::ReadWrite).publishOnChange(MIN_DELTA_FLOAT_PROPERTY);
  ArduinoCloud.addProperty(float_property_4, Permission::ReadWrite).publishOnChange(MIN_DELTA_FLOAT_PROPERTY).onUpdate(onFloatPropertyChange);

  ArduinoCloud.addProperty(str_property_3, READWRITE, 1 * SECONDS, 0 /* onStringPropertyChange */, 0.0 /* 'minDelta' */, MOST_RECENT_WINS);
  ArduinoCloud.addProperty(str_property_4, READWRITE, 1 * SECONDS, 0 /* onStringPropertyChange */, 0.0 /* 'minDelta' */, CLOUD_WINS);
  ArduinoCloud.addProperty(str_property_5, READWRITE, 1 * SECONDS, 0 /* onStringPropertyChange */, 0.0 /* 'minDelta' */, DEVICE_WINS);

  ArduinoCloud.addProperty(str_property_6, Permission::ReadWrite).publishEvery(1 * SECONDS).onSync(MOST_RECENT_WINS);
  ArduinoCloud.addProperty(str_property_7, Permission::ReadWrite).publishEvery(1 * SECONDS).onSync(CLOUD_WINS);
  ArduinoCloud.addProperty(str_property_8, Permission::ReadWrite).publishEvery(1 * SECONDS).onSync(DEVICE_WINS);
}

#elif defined(BOARD_HAS_LORA)
void initProperties() {
  ArduinoCloud.setThingId(THING_ID);

  ArduinoCloud.addProperty(bool_property_1,  1,   READWRITE, 1 * SECONDS);
  ArduinoCloud.addProperty(int_property_1,   2,   READ,      2 * MINUTES);
  ArduinoCloud.addProperty(float_property_1, 3,   WRITE,     3 * HOURS);
  ArduinoCloud.addProperty(str_property_1,   4,   READWRITE, 4 * DAYS);

  ArduinoCloud.addProperty(bool_property_2,  5,   Permission::ReadWrite).publishEvery(1 * SECONDS);
  ArduinoCloud.addProperty(int_property_2,   6,   Permission::Read).publishEvery(1 * MINUTES);
  ArduinoCloud.addProperty(float_property_2, 7,   Permission::Write).publishEvery(3 * HOURS);
  ArduinoCloud.addProperty(str_property_2,   8,   Permission::ReadWrite).publishEvery(4 * DAYS);

  ArduinoCloud.addProperty(int_property_3,   9,   READWRITE, ON_CHANGE);                       /* Default 'minDelta' = 0 */
  ArduinoCloud.addProperty(int_property_4,  10,   READWRITE, ON_CHANGE, onIntPropertyChange);  /* Default 'minDelta' = 0 */
  ArduinoCloud.addProperty(int_property_5,  11,   READWRITE, ON_CHANGE, 0 /* onIntPropertyChange */, MIN_DELTA_INT_PROPERTY);
  ArduinoCloud.addProperty(int_property_6,  12,   READWRITE, ON_CHANGE, onIntPropertyChange,         MIN_DELTA_INT_PROPERTY);

  ArduinoCloud.addProperty(float_property_3, 13, Permission::ReadWrite).publishOnChange(MIN_DELTA_FLOAT_PROPERTY);
  ArduinoCloud.addProperty(float_property_4, 14, Permission::ReadWrite).publishOnChange(MIN_DELTA_FLOAT_PROPERTY).onUpdate(onFloatPropertyChange);

  ArduinoCloud.addProperty(str_property_3, 15, READWRITE, 1 * SECONDS, 0 /* onStringPropertyChange */, 0.0 /* 'minDelta' */, MOST_RECENT_WINS);
  ArduinoCloud.addProperty(str_property_4, 16, READWRITE, 1 * SECONDS, 0 /* onStringPropertyChange */, 0.0 /* 'minDelta' */, CLOUD_WINS);
  ArduinoCloud.addProperty(str_property_5, 17, READWRITE, 1 * SECONDS, 0 /* onStringPropertyChange */, 0.0 /* 'minDelta' */, DEVICE_WINS);

  ArduinoCloud.addProperty(str_property_6, 18, Permission::ReadWrite).publishEvery(1 * SECONDS).onSync(MOST_RECENT_WINS);
  ArduinoCloud.addProperty(str_property_7, 19, Permission::ReadWrite).publishEvery(1 * SECONDS).onSync(CLOUD_WINS);
  ArduinoCloud.addProperty(str_property_8, 20, Permission::ReadWrite).publishEvery(1 * SECONDS).onSync(DEVICE_WINS);
}
#endif
