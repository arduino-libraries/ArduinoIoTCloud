/*
   This file is part of ArduinoIoTCloud.

   Copyright 2019 ARDUINO SA (http://www.arduino.cc/)

   This software is released under the GNU General Public License version 3,
   which covers the main part of arduino-cli.
   The terms of this license can be found at:
   https://www.gnu.org/licenses/gpl-3.0.en.html

   You can be released from the requirements of the above licenses by purchasing
   a commercial license. Buying such a license is mandatory if you want to modify or
   otherwise use the software for commercial activities involving the Arduino
   software without disclosing the source code of your own applications. To purchase
   a commercial license, send an email to license@arduino.cc.
*/
#include "ArduinoIoTCloud_Defines.h"
#ifndef HAS_LORA

#include "ArduinoIoTCloud.h"
#include "CloudSerial.h"

CloudSerialClass::CloudSerialClass() {
}

CloudSerialClass::~CloudSerialClass() {
}

void CloudSerialClass::begin(int /*baud*/) {
  _txBuffer.clear();
  _rxBuffer.clear();
}

void CloudSerialClass::end() {
}

int CloudSerialClass::available() {

  return _rxBuffer.available();
}

int CloudSerialClass::availableForWrite() {

  return _txBuffer.availableForStore();
}

int CloudSerialClass::peek() {

  return _rxBuffer.peek();
}

int CloudSerialClass::read() {

  return _rxBuffer.read_char();
}

void CloudSerialClass::flush() {

  byte out[CLOUD_SERIAL_TX_BUFFER_SIZE];
  int length = 0;

  while (_txBuffer.available()) {
    out[length++] = _txBuffer.read_char();
  }

  ArduinoCloud.writeStdout(out, length);
}

size_t CloudSerialClass::write(const uint8_t data) {
  _txBuffer.store_char(data);

  if (_txBuffer.isFull() || data == '\n') {
    flush();
  }

  return 1;
}

CloudSerialClass::operator bool() {

  return ArduinoCloud.connected();
}

void CloudSerialClass::appendStdin(const uint8_t *buffer, size_t size) {
  while (!_rxBuffer.isFull() && size--) {
    _rxBuffer.store_char(*buffer++);
  }
}

CloudSerialClass CloudSerial;

#endif