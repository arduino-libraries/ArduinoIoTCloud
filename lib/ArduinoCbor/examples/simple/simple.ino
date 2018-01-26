#include <ArduinoCbor.h>

void setup() {
  Serial.begin(115200);

  while(!Serial) {
    delay(10);
  }
}


void loop() {
  CborBuffer buffer(200);
  CborObject object = CborObject(buffer);

  object.set("string", "value");
  object.set("integer", -1234);
  
  CborObject child = CborObject(buffer);
  child.set("key", "value");
  object.set("object", child);
  
  CborArray array = CborArray(buffer);
  array.add(-1234);
  object.set("array", array);

  Serial.print("string value: ");
  Serial.println(object.get("string").asString());

  Serial.print("integer value: ");
  Serial.println(object.get("integer").asInteger());

  Serial.print("child string value: ");
  Serial.println(object.get("object").asObject().get("key").asString());

  delay(1000);
}
