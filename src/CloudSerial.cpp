#include "ArduinoCloudV2.h"

#include "CloudSerial.h"

CloudSerialClass::CloudSerialClass()
{
}

CloudSerialClass::~CloudSerialClass()
{
}

void CloudSerialClass::begin(int /*baud*/)
{
  _rxBuffer.clear();
}

void CloudSerialClass::end()
{
}

int CloudSerialClass::available()
{
  ArduinoCloud.poll();

  return _rxBuffer.available();
}

int CloudSerialClass::availableForWrite()
{
  ArduinoCloud.poll();

  return 1;
}

int CloudSerialClass::peek()
{
  ArduinoCloud.poll();

  return _rxBuffer.peek();
}

int CloudSerialClass::read()
{
  ArduinoCloud.poll();

  return _rxBuffer.read_char();
}

void CloudSerialClass::flush()
{
  ArduinoCloud.poll();
}

size_t CloudSerialClass::write(const uint8_t data)
{
  return write(&data, sizeof(data));
}

size_t CloudSerialClass::write(const uint8_t *buffer, size_t size)
{
  ArduinoCloud.poll();
Serial.write(buffer, size);
  if (!ArduinoCloud.writeStdout(buffer, size)) {
    return 0;
  }

  return size;
}

CloudSerialClass::operator bool()
{
  ArduinoCloud.poll();

  return ArduinoCloud.connected();
}

void CloudSerialClass::appendStdin(const uint8_t *buffer, size_t size)
{
  while (!_rxBuffer.isFull() && size--) {
    _rxBuffer.store_char(*buffer++);
  }
}

CloudSerialClass CloudSerial;
