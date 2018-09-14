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
  ArduinoCloud.poll();

  return _rxBuffer.available();
}

int CloudSerialClass::availableForWrite()
{
  ArduinoCloud.poll();

  return _txBuffer.availableForStore();
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
