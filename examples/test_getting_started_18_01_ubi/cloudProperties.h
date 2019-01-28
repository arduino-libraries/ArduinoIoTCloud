#include <GSMConnectionManager.h>

ConnectionManager *ArduinoIoTPreferredConnection = new GSMConnectionManager(
  SECRET_GSMPIN, SECRET_GPRSAPN, SECRET_GPRSLOGIN, SECRET_GPRSPWD);

void onLedSwitchChange();
void onIntValueChange();

bool ledSwitch;
int intValue;

void initProperties() {
  ArduinoCloud.setThingId(THING_ID);
  ArduinoCloud.addProperty(ledSwitch, READWRITE, ON_CHANGE, onLedSwitchChange);
  ArduinoCloud.addProperty(intValue, READWRITE, ON_CHANGE, onIntValueChange);
}
