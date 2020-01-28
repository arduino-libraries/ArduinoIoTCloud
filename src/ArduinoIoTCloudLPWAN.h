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

#ifndef ARDUINO_IOT_CLOUD_LPWAN_H
#define ARDUINO_IOT_CLOUD_LPWAN_H

#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

static uint8_t const DEFAULT_CBOR_LORA_MSG_SIZE = 255;

class ArduinoIoTCloudLPWAN : public ArduinoIoTCloudClass {
  public:
    ArduinoIoTCloudLPWAN();
    ~ArduinoIoTCloudLPWAN();
    int connect();
    bool disconnect();
    int connected();
    inline void update() {
      update(NULL);
    }
    inline void update(int const reconnectionMaxRetries, int const reconnectionTimeoutMs) __attribute__((deprecated)) {
      update(NULL);
    }
    void update(CallbackFunc onSyncCompleteCallback) __attribute__((deprecated));
    void connectionCheck();
    void printDebugInfo();
    int begin(LPWANConnectionHandler& connection, bool retry = false);
    inline LPWANConnectionHandler* getConnection() {
      return _connection;
    }
    bool isRetryEnabled() {
      return _retryEnable;
    }

    void enableRetry(bool val) {
      _retryEnable = val;
    }

    int getMaxRetry() {
      return _maxNumRetry;
    }

    void setMaxRetry(int val) {
      _maxNumRetry = val;
    }

    long getIntervalRetry() {
      return _intervalRetry;
    }

    void setIntervalRetry(long val) {
      _intervalRetry = val;
    }

  protected:
    int writeStdout(const byte data[], int length);
    int writeProperties(const byte data[], int length);
    int writeShadowOut(const byte data[], int length);

  private:
    LPWANConnectionHandler* _connection;
    void sendPropertiesToCloud();
    bool _retryEnable;
    int _maxNumRetry;
    long _intervalRetry;

};

extern ArduinoIoTCloudLPWAN ArduinoCloud;


#endif