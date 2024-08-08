/*
   This file is part of ArduinoIoTCloud.

   Copyright 2024 Blues (http://www.blues.com/)

   This software is released under the GNU General Public License version 3,
   which covers the main part of arduino-cli.
   The terms of this license can be found at:
   https://www.gnu.org/licenses/gpl-3.0.en.html

   You can be released from the requirements of the above licenses by purchasing
   a commercial license. Buying such a license is mandatory if you want to
   modify or otherwise use the software for commercial activities involving the
   Arduino software without disclosing the source code of your own applications.
   To purchase a commercial license, send an email to license@arduino.cc.
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

#if OTA_ENABLED
typedef bool (*onOTARequestCallbackFunc)(void);
#endif /* OTA_ENABLED */

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
    inline virtual PropertyContainer &getThingPropertyContainer() override { return _thing.getPropertyContainer(); }

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
     * The interval at which the Notecard is polled for new data.
     *
     * @param interval_ms The interval in milliseconds.
     * @par
     * - Minimum: 250ms
     * - Default: 1000ms
     *
     * @note The Notecard poll interval is ignored if an interrupt pin is
     * provided to the `begin()` function.
     */
    inline void setNotecardPollInterval(uint32_t interval_ms) { _notecard_poll_interval_ms = ((interval_ms < 250) ? 250 : interval_ms); }

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
    uint32_t _notecard_poll_interval_ms;
    int _interrupt_pin;
    volatile bool _data_available;

#if OTA_ENABLED
    // OTA member variables
    bool _ota_cap;
    int _ota_error;
    String _ota_img_sha256;
    String _ota_url;
    bool _ota_req;
    bool _ask_user_before_executing_ota;
    onOTARequestCallbackFunc _get_ota_confirmation;
#endif /* OTA_ENABLED */

    State handle_ConnectPhy();
    State handle_SyncTime();
    State handle_Connected();
    State handle_Disconnect();

    void attachThing(String thingId);
    bool available (void);
#if OTA_ENABLED
    void checkOTARequest(void);
#endif /* OTA_ENABLED */
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
