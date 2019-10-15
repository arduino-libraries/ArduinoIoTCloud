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

#ifndef CLOUD_SERIAL_H
#define CLOUD_SERIAL_H

#include <Arduino.h>
#if defined(ARDUINO_ESP8266_ESP12) || defined(ARDUINO_ARCH_ESP32) || defined(ESP8266)
  #include "utility/RingBuffer.h"
#else
  #include <RingBuffer.h>
#endif

#define CLOUD_SERIAL_TX_BUFFER_SIZE 64
#define CLOUD_SERIAL_RX_BUFFER_SIZE 512

class ArduinoIoTCloudClass;

class CloudSerialClass : public Stream {
  public:
    CloudSerialClass();
    ~CloudSerialClass();

    void begin(int baud);
    void end();
    int available();
    int availableForWrite();
    int peek();
    int read();
    void flush();
    size_t write(const uint8_t data);
    using Print::write; // pull in write(str) and write(buf, size) from Print

    operator bool();

  protected:
    friend class ArduinoIoTCloudClass;

    void appendStdin(const uint8_t *buffer, size_t size);

  private:
    RingBufferN<CLOUD_SERIAL_TX_BUFFER_SIZE> _txBuffer;
    RingBufferN<CLOUD_SERIAL_RX_BUFFER_SIZE> _rxBuffer;
};

extern CloudSerialClass CloudSerial;

#endif
