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

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <ArduinoIoTCloud.h>

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class ArduinoIoTCloudLPWAN : public ArduinoIoTCloudClass {
  public:

             ArduinoIoTCloudLPWAN();
    virtual ~ArduinoIoTCloudLPWAN() { }

    virtual int  connect       () override;
    virtual bool disconnect    () override;
    virtual void update        () override;
    virtual int  connected     () override;
    virtual void printDebugInfo() override;

    ArduinoIoTConnectionStatus connectionCheck();
    int begin(ConnectionHandler& connection, bool retry = false);

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

  private:

    bool _retryEnable;
    int _maxNumRetry;
    long _intervalRetry;

    void sendPropertiesToCloud();
    int writeProperties(const byte data[], int length);
};

/******************************************************************************
 * EXTERN DECLARATION
 ******************************************************************************/

extern ArduinoIoTCloudLPWAN ArduinoCloud;

#endif