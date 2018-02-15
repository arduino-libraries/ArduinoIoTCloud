#ifndef CLOUD_SERIAL_H
#define CLOUD_SERIAL_H

#include <Arduino.h>

#include <RingBuffer.h>

#define CLOUD_SERIAL_TX_BUFFER_SIZE 64
#define CLOUD_SERIAL_RX_BUFFER_SIZE 512

class ArduinoCloudClass;

class CloudSerialClass : public Stream
{
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
  friend class ArduinoCloudClass;

  void appendStdin(const uint8_t *buffer, size_t size);

private:
  RingBufferN<CLOUD_SERIAL_TX_BUFFER_SIZE> _txBuffer;
  RingBufferN<CLOUD_SERIAL_RX_BUFFER_SIZE> _rxBuffer;
};

extern CloudSerialClass CloudSerial;

#endif
