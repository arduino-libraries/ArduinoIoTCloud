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
#include <ArduinoIoTCloudLPWANThing.h>

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
    virtual void     setThingId (String const thing_id)  override;
    virtual String & getThingId ()                       override;
    virtual bool    deviceNotAttached()                 override;
    virtual void     setThingIdOutdatedFlag()            override;
    virtual void     clrThingIdOutdatedFlag()            override;
    virtual bool     getThingIdOutdatedFlag()            override;

    inline void     setThingIdOutdatedFlag()            { _thing_id_outdated = true ; }
    inline void     clrThingIdOutdatedFlag()            { _thing_id_outdated = false ; }
    inline bool     getThingIdOutdatedFlag()            { return _thing_id_outdated; }

    inline void     setThingId (String const thing_id)  { _thing_id = thing_id; };
    inline String &  getThingId ()                       { return _thing_id; };
    inline bool     deviceNotAttached()                 { return _thing_id == ""; }

    inline void     setTzOffset(int tz_offset)  { _tz_offset = tz_offset; }
    inline int &    getTzOffset()         { return _tz_offset; }

    inline void setTzDstUntil(unsigned int tz_dst_until) { _tz_dst_until = tz_dst_until; }
    inline unsigned int & getTzDstUntil() { return _tz_dst_until; }

    int begin(ConnectionHandler& connection, bool retry = false);

    inline bool isRetryEnabled  () const { return _retryEnable; }
    inline int  getMaxRetry     () const { return _maxNumRetry; }
    inline long getIntervalRetry() const { return _intervalRetry; }

    inline void enableRetry     (bool val) { _retryEnable = val; }
    inline void setMaxRetry     (int val)  { _maxNumRetry = val; }
    inline void setIntervalRetry(long val) { _intervalRetry = val; }


  private:

    enum class State
    {
      ConnectPhy,
      SyncTime,
      Connected,
    };

    String _thing_id;
    bool _thing_id_outdated;

    int _tz_offset;
    unsigned int _tz_dst_until;
    unsigned int _last_checked_property_index;

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