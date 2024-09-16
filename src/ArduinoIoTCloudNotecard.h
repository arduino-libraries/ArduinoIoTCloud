/*
  This file is part of the ArduinoIoTCloud library.

  Copyright 2024 Blues (http://www.blues.com/)

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef ARDUINO_IOT_CLOUD_NOTECARD_H
#define ARDUINO_IOT_CLOUD_NOTECARD_H

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include "ArduinoIoTCloud.h"
#include "ArduinoIoTCloudThing.h"
#include "ArduinoIoTCloudDevice.h"

/******************************************************************************
 * DEFINES
 ******************************************************************************/

#define USE_LIGHT_PAYLOADS (false)

/******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/******************************************************************************
 * TYPEDEF
 ******************************************************************************/

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

/**
 * @brief The ArduinoIoTCloudNotecard class
 *
 * This class is used to connect to the Arduino IoT Cloud using a Notecard.
 */
class ArduinoIoTCloudNotecard : public ArduinoIoTCloudClass
{
  public:
             ArduinoIoTCloudNotecard();
    virtual ~ArduinoIoTCloudNotecard() { }

    virtual void update        () override;
    virtual int  connected     () override;
    virtual void printDebugInfo() override;

    /**
     * @brief Begin the connection to the Arduino IoT Cloud.
     *
     * @param connection The connection handler to use.
     * @param interrupt_pin The interrupt pin to use for the Notecard.
     *
     * @note The interrupt pin is optional and only required if you want to
     * eliminate the need to poll the Notecard for new data. The pin only needs
     * to be specified, and will otherwise be configured by the library.
     *
     * @return 1 on success, 0 on failure.
     */
    int begin(ConnectionHandler &connection, int interrupt_pin = -1);

    /**
     * @brief Set the Notecard polling interval.
     *
     * The interval at which the Notecard is polled (via I2C/UART) for new data.
     * This is not a network transaction, but a local polling of the Notecard.
     *
     * @param interval_ms The interval in milliseconds.
     * @par
     * - Minimum: 250ms
     * - Default: 1000ms
     *
     * @note The Notecard poll interval is ignored if an interrupt pin is
     * provided to the `begin()` function.
     */
    inline void setNotecardPollingInterval(uint32_t interval_ms) { _notecard_polling_interval_ms = ((interval_ms < 250) ? 250 : interval_ms); }

  private:

    enum class State
    {
      ConnectPhy,
      SyncTime,
      Connected,
      Disconnect,
    };

    State _state;
    TimedAttempt _connection_attempt;
    MessageStream _message_stream;
    ArduinoCloudThing _thing;
    ArduinoCloudDevice _device;

    // Notecard member variables
    uint32_t _notecard_last_poll_ms;
    uint32_t _notecard_polling_interval_ms;
    int _interrupt_pin;
    volatile bool _data_available;

    inline virtual PropertyContainer &getThingPropertyContainer() override { return _thing.getPropertyContainer(); }

    State handle_ConnectPhy();
    State handle_SyncTime();
    State handle_Connected();
    State handle_Disconnect();

    void attachThing(String thingId);
    bool available (void);
    void detachThing();
    void fetchIncomingBytes(uint8_t *buf, size_t &len);
    void pollNotecard(void);
    void processCommand(const uint8_t *buf, size_t len);
    void processMessage(const uint8_t *buf, size_t len);
    void sendMessage(Message * msg);
    void sendCommandMsgToCloud(Message * msg_);
    void sendThingPropertyContainerToCloud(void);

    friend void ISR_dataAvailable (void);
};

/******************************************************************************
 * EXTERN DECLARATION
 ******************************************************************************/

extern ArduinoIoTCloudNotecard ArduinoCloud;

#endif // ARDUINO_IOT_CLOUD_NOTECARD_H
