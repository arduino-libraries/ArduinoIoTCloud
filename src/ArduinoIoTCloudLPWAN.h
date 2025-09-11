/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2019 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef ARDUINO_IOT_CLOUD_LPWAN_H
#define ARDUINO_IOT_CLOUD_LPWAN_H

/******************************************************************************
  INCLUDE
 ******************************************************************************/

#include <ArduinoIoTCloud.h>

/******************************************************************************
  CLASS DECLARATION
 ******************************************************************************/

class ArduinoIoTCloudLPWAN : public ArduinoIoTCloudClass
{
  public:

             ArduinoIoTCloudLPWAN();
    virtual ~ArduinoIoTCloudLPWAN() { }

    virtual void update        () override;
    virtual int  connected     () override;
    virtual void printDebugInfo() override;

    int begin(ConnectionHandler& connection, bool retry = false);

    inline bool isRetryEnabled  () const { return _retryEnable; }
    inline int  getMaxRetry     () const { return _maxNumRetry; }
    inline long getIntervalRetry() const { return _intervalRetry; }

    inline void enableRetry     (bool val) { _retryEnable = val; }
    inline void setMaxRetry     (int val)  { _maxNumRetry = val; }
    inline void setIntervalRetry(long val) { _intervalRetry = val; }

    inline PropertyContainer &getThingPropertyContainer() { return _thing_property_container; }


  private:

    enum class State
    {
      ConnectPhy,
      SyncTime,
      Connected,
    };

    State _state;
    bool _retryEnable;
    int _maxNumRetry;
    long _intervalRetry;

    PropertyContainer _thing_property_container;
    unsigned int _last_checked_property_index;

    State handle_ConnectPhy();
    State handle_SyncTime();
    State handle_Connected();

    void decodePropertiesFromCloud();
    void sendPropertiesToCloud();
    int writeProperties(const byte data[], int length);
};

/******************************************************************************
  EXTERN DECLARATION
 ******************************************************************************/

extern ArduinoIoTCloudLPWAN ArduinoCloud;

#endif
