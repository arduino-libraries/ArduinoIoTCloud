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
#include "property/PropertyContainer.h"

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class ArduinoIoTCloudLPWAN : public ArduinoIoTCloudClass
{
  public:

             ArduinoIoTCloudLPWAN();
    virtual ~ArduinoIoTCloudLPWAN() { }

    virtual void update        () override;
    virtual int  connected     () override;
    virtual void printDebugInfo() override;

    virtual inline void     setThingIdOutdatedFlag()            { _thing_id_outdated = true ; }
    virtual inline void     clrThingIdOutdatedFlag()            { _thing_id_outdated = false ; }
    virtual inline bool     getThingIdOutdatedFlag()            { return _thing_id_outdated; }

    virtual inline void     setThingId (String const thing_id)  { _thing_id = thing_id; };
    virtual inline String &  getThingId ()                       { return _thing_id; };
    virtual inline bool     deviceNotAttached()                 { return _thing_id == ""; }

    int begin(ConnectionHandler& connection, bool retry = false);

    inline bool isRetryEnabled  () const { return _retryEnable; }
    inline int  getMaxRetry     () const { return _maxNumRetry; }
    inline long getIntervalRetry() const { return _intervalRetry; }

    inline void enableRetry     (bool val) { _retryEnable = val; }
    inline void setMaxRetry     (int val)  { _maxNumRetry = val; }
    inline void setIntervalRetry(long val) { _intervalRetry = val; }

    virtual Property& addInternalPropertyReal(Property& property, String name, int tag, Permission const permission, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(Property & property) = CLOUD_WINS) override;

    virtual void push() override;
    virtual bool setTimestamp(String const & prop_name, unsigned long const timestamp) override;

  private:

    enum class State
    {
      ConnectPhy,
      SyncTime,
      Connected,
    };

    String _thing_id;
    bool _thing_id_outdated;

    unsigned int _last_checked_property_index;

    PropertyContainer _thing_property_container;

    State _state;
    bool _retryEnable;
    int _maxNumRetry;
    long _intervalRetry;

    State handle_ConnectPhy();
    State handle_SyncTime();
    State handle_Connected();

    void decodePropertiesFromCloud();
    void sendPropertiesToCloud();
    int writeProperties(const byte data[], int length);
};

/******************************************************************************
 * EXTERN DECLARATION
 ******************************************************************************/

extern ArduinoIoTCloudLPWAN ArduinoCloud;

#endif