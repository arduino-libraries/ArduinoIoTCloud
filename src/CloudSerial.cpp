#include "ArduinoIoTCloud.h"
#include "CloudSerial.h"

CloudSerialClass::CloudSerialClass()
{
}

CloudSerialClass::~CloudSerialClass()
{
}

void CloudSerialClass::begin(int /*baud*/)
{
  _txBuffer.clear();
  _rxBuffer.clear();
}

void CloudSerialClass::end()
{
}

int CloudSerialClass::available()
{
  ArduinoCloud.update();

  return _rxBuffer.available();
}

int CloudSerialClass::availableForWrite()
{
  ArduinoCloud.update();

  return _txBuffer.availableForStore();
}

int CloudSerialClass::peek()
{
  ArduinoCloud.update();

  return _rxBuffer.peek();
}

int CloudSerialClass::read()
{
  ArduinoCloud.update();

  return _rxBuffer.read_char();
}

void CloudSerialClass::flush()
{
  ArduinoCloud.update();

  byte out[CLOUD_SERIAL_TX_BUFFER_SIZE];
  int length = 0;

  while (_txBuffer.available()) {
    out[length++] = _txBuffer.read_char();
  }

  ArduinoCloud.writeStdout(out, length);
}

size_t CloudSerialClass::write(const uint8_t data)
{
  _txBuffer.store_char(data);

  if (_txBuffer.isFull() || data == '\n') {
    flush();
  }

  return 1;
}

CloudSerialClass::operator bool()
{
  ArduinoCloud.update();

  return ArduinoCloud.connected();
}

void CloudSerialClass::appendStdin(const uint8_t *buffer, size_t size)
{
  while (!_rxBuffer.isFull() && size--) {
    _rxBuffer.store_char(*buffer++);
  }
}

CloudSerialClass CloudSerial;
