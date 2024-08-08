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

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include "AIoTC_Config.h"

#if defined(USE_NOTECARD)

#include "ArduinoIoTCloudNotecard.h"

#include <algorithm>
#include <functional>

#include <Arduino.h>
#include <Arduino_NotecardConnectionHandler.h>

#include "cbor/CBOREncoder.h"
#include "cbor/MessageDecoder.h"
#include "cbor/MessageEncoder.h"

#if OTA_ENABLED
#include "ota/OTA.h"
#endif /* OTA_ENABLED */

/******************************************************************************
 * CONSTANTS
 ******************************************************************************/

static size_t const CBOR_NOTE_MSG_MAX_SIZE = 255;
static size_t const DEFAULT_READ_INTERVAL_MS = 1000;   // 1 second
static size_t const FAILSAFE_READ_INTERVAL_MS = 15000; // 15 seconds

/******************************************************************************
 * LOCAL MODULE FUNCTIONS
 ******************************************************************************/

unsigned long getTime()
{
  return ArduinoCloud.getInternalTime();
}

void ISR_dataAvailable(void)
{
  ArduinoCloud._data_available = true;
}

/******************************************************************************
 * CTOR/DTOR
 ******************************************************************************/

ArduinoIoTCloudNotecard::ArduinoIoTCloudNotecard()
:
  _state{State::ConnectPhy}
  ,_connection_attempt(0,0)
  ,_message_stream(std::bind(&ArduinoIoTCloudNotecard::sendMessage, this, std::placeholders::_1))
  ,_thing(&_message_stream)
  ,_device(&_message_stream)
  ,_notecard_last_poll_ms{static_cast<uint32_t>(-DEFAULT_READ_INTERVAL_MS)}
  ,_notecard_poll_interval_ms{DEFAULT_READ_INTERVAL_MS}
  ,_interrupt_pin{-1}
  ,_data_available{false}
#if OTA_ENABLED
  ,_ota_cap{false}
  ,_ota_error{static_cast<int>(OTAError::None)}
  ,_ota_img_sha256{"Inv."}
  ,_ota_url{""}
  ,_ota_req{false}
  ,_ask_user_before_executing_ota{false}
  ,_get_ota_confirmation{nullptr}
#endif /* OTA_ENABLED */
{

}

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

int ArduinoIoTCloudNotecard::begin(ConnectionHandler &connection_, int interrupt_pin_)
{
  _connection = &connection_;
  NotecardConnectionHandler *notecard_connection = reinterpret_cast<NotecardConnectionHandler *>(_connection);

  // Configure the interrupt pin
  if (interrupt_pin_ >= 0) {
    _interrupt_pin = interrupt_pin_;
    ::pinMode(_interrupt_pin, INPUT);
    ::attachInterrupt(digitalPinToInterrupt(_interrupt_pin), ISR_dataAvailable, RISING);
    notecard_connection->enableHardwareInterrupts();
  }

  // Initialize the connection to the Notecard
  if (NetworkConnectionState::ERROR == _connection->check()) {
    DEBUG_ERROR("ArduinoIoTCloudNotecard::%s encountered fatal connection error!", __FUNCTION__);
    return 0; // (false -> failure)
  }

  // Pull the Arduino IoT Cloud Device ID from the Notecard
  setDeviceId(notecard_connection->getDeviceId());

#if  OTA_ENABLED
  /* Setup OTA TLS client */
  _otaClient.begin(connection);
#endif

  // Begin the Notecard time service
  _time_service.begin(&connection_);

  /* Setup retry timers */
  _connection_attempt.begin(AIOT_CONFIG_RECONNECTION_RETRY_DELAY_ms, AIOT_CONFIG_MAX_RECONNECTION_RETRY_DELAY_ms);

  // Configure the Device and Thing property containers
  _thing.begin();
  _device.begin();

  return 1; // (true -> success)
}

int ArduinoIoTCloudNotecard::connected()
{
  return (_connection->check() == NetworkConnectionState::CONNECTED);
}

void ArduinoIoTCloudNotecard::printDebugInfo()
{
  // Print the debug information
  NetworkConnectionState conn_state = _connection->check();
  DEBUG_INFO("***** Arduino IoT Cloud Notecard - configuration info *****");
  DEBUG_INFO("Notecard UID: %s", reinterpret_cast<NotecardConnectionHandler *>(_connection)->getNotecardUid().c_str());
  DEBUG_INFO("Arduino Device ID: %s", getDeviceId().c_str());
  if (NetworkConnectionState::CONNECTED == conn_state)
  {
    DEBUG_INFO("Arduino Thing ID: %s", getThingId().c_str());
  }
  else
  {
    DEBUG_INFO("Arduino Thing ID: awaiting connection...");
  }
}

void ArduinoIoTCloudNotecard::update()
{
  /* Run through the state machine. */
  State next_state = _state;
  switch (_state)
  {
  case State::ConnectPhy:           next_state = handle_ConnectPhy();           break;
  case State::SyncTime:             next_state = handle_SyncTime();             break;
  case State::Connected:            next_state = handle_Connected();            break;
  case State::Disconnect:           next_state = handle_Disconnect();           break;
  }
  _state = next_state;

#if OTA_ENABLED
  if (State::Connected == _state) {
    checkOTARequest();
  }
#endif /* OTA_ENABLED */
}

/******************************************************************************
 * PRIVATE STATE MACHINE FUNCTIONS
 ******************************************************************************/

ArduinoIoTCloudNotecard::State ArduinoIoTCloudNotecard::handle_ConnectPhy()
{
  if (_connection->check() == NetworkConnectionState::CONNECTED
  && (!_connection_attempt.isRetry() || (_connection_attempt.isRetry() && _connection_attempt.isExpired())))
    return State::SyncTime;
  else
    return State::ConnectPhy;
}

ArduinoIoTCloudNotecard::State ArduinoIoTCloudNotecard::handle_SyncTime()
{
  const uint32_t current_time = ArduinoCloud.getInternalTime();
  if (TimeServiceClass::isTimeValid(current_time))
  {
    DEBUG_VERBOSE("ArduinoIoTCloudNotecard::%s internal clock configured to posix timestamp %d", __FUNCTION__, current_time);
    return State::Connected;
  }

  DEBUG_ERROR("ArduinoIoTCloudNotecard::%s could not get valid time. Retrying now.", __FUNCTION__);
  return State::ConnectPhy;
}

ArduinoIoTCloudNotecard::State ArduinoIoTCloudNotecard::handle_Connected()
{
  if (!connected() || !_thing.connected() || !_device.connected())
  {
    return State::Disconnect;
  }

  /* Poll Notecard for new messages */
  pollNotecard();

  /* Call CloudDevice process to get configuration */
  _device.update();

#if  OTA_ENABLED
  if(_get_ota_confirmation != nullptr &&
      _ota.getState() == OTACloudProcessInterface::State::OtaAvailable &&
      _get_ota_confirmation()) {
    _ota.approveOta();
  }

  _ota.update();
#endif // OTA_ENABLED


  if (_device.isAttached()) {
    /* Call CloudThing process to synchronize properties */
    _thing.update();
  }

  return State::Connected;
}

ArduinoIoTCloudNotecard::State ArduinoIoTCloudNotecard::handle_Disconnect()
{
  if (!connected()) {
    DEBUG_ERROR("ArduinoIoTCloudNotecard::%s connection to Notehub lost", __FUNCTION__);
  }

  // Reset the Thing and Device property containers
  Message message = { ResetCmdId };
  _thing.handleMessage(&message);
  _device.handleMessage(&message);

  DEBUG_INFO("Disconnected from Arduino IoT Cloud");

  // Execute the user-defined disconnect callback
  execCloudEventCallback(ArduinoIoTCloudEvent::DISCONNECT);

  /* Setup timer for broker connection and restart */
  _connection_attempt.begin(AIOT_CONFIG_RECONNECTION_RETRY_DELAY_ms, AIOT_CONFIG_MAX_RECONNECTION_RETRY_DELAY_ms);
  return State::ConnectPhy;
}

/******************************************************************************
 * PRIVATE SUPPORT FUNCTIONS
 ******************************************************************************/

void ArduinoIoTCloudNotecard::attachThing(String thingId)
{
  _thing_id = thingId;

  Message message;
  message = { DeviceAttachedCmdId };
  _device.handleMessage(&message);

  DEBUG_INFO("Connected to Arduino IoT Cloud");
  DEBUG_INFO("Thing ID: %s", getThingId().c_str());
  execCloudEventCallback(ArduinoIoTCloudEvent::CONNECT);
}

bool ArduinoIoTCloudNotecard::available(void)
{
  bool result;

  const bool interrupts_enabled = (_interrupt_pin >= 0);
  const uint32_t now_ms = ::millis();

  bool check_data = ((now_ms - _notecard_last_poll_ms) > _notecard_poll_interval_ms);
  if (interrupts_enabled) {
    check_data = (_data_available || ((now_ms - _notecard_last_poll_ms) > FAILSAFE_READ_INTERVAL_MS));
  }

  if (check_data) {
    result = _connection->available();
    _data_available = ::digitalRead(_interrupt_pin);
    _notecard_last_poll_ms = now_ms;
  } else {
    result = false;
  }

  return result;
}

#if OTA_ENABLED
void ArduinoIoTCloudNotecard::checkOTARequest(void) {
  /* Request a OTA download if the hidden property
  * OTA request has been set.
  */

  if (_ota_req)
  {
    bool const ota_execution_allowed_by_user = (_get_ota_confirmation != nullptr && _get_ota_confirmation());
    bool const perform_ota_now = ota_execution_allowed_by_user || !_ask_user_before_executing_ota;
    if (perform_ota_now) {
      /* Clear the error flag. */
      _ota_error = static_cast<int>(OTAError::None);
      _ota_error = 0;
      /* Clear the request flag. */
      _ota_req = false;
      /* Transmit the cleared request flags to the cloud. */
      sendDevicePropertyToCloud("OTA_REQ");
      /* Call member function to handle OTA request. */
      _ota_error = OTA::onRequest(_ota_url, _connection->getInterface());
      DEBUG_WARNING("OTA request received. OTA is not currently supported by Notecard.");
      /* If something fails send the OTA error to the cloud */
      sendDevicePropertyToCloud("OTA_ERROR");
    }
  }

  /* Check if we have received the OTA_URL property and provide
  * echo to the cloud.
  */
  sendDevicePropertyToCloud("OTA_URL");
}
#endif /* OTA_ENABLED */

void ArduinoIoTCloudNotecard::detachThing()
{
  Message message;
  message = { DeviceDetachedCmdId };
  _device.handleMessage(&message);

  _thing_id = "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx";
  DEBUG_INFO("Disconnected from Arduino IoT Cloud");
  execCloudEventCallback(ArduinoIoTCloudEvent::DISCONNECT);
}

void ArduinoIoTCloudNotecard::fetchIncomingBytes(uint8_t *buf, size_t &len)
{
  size_t bytes_received = 0;
  for (;
       _connection->available() && (bytes_received < len);
       ++bytes_received)
  {
    buf[bytes_received] = _connection->read();
  }
  len = bytes_received;
}

void ArduinoIoTCloudNotecard::pollNotecard(void)
{
  /* Decode available data. */
  if (available()) {
    size_t note_len = CBOR_NOTE_MSG_MAX_SIZE;
    uint8_t note_buf[CBOR_NOTE_MSG_MAX_SIZE];
    fetchIncomingBytes(note_buf, note_len);
    processMessage(note_buf, note_len);
  }
}

void ArduinoIoTCloudNotecard::processCommand(const uint8_t *buf, size_t len)
{
  CommandDown command;
  DEBUG_VERBOSE("ArduinoIoTCloudNotecard::%s [%d] received %d bytes", __FUNCTION__, millis(), len);
  CBORMessageDecoder decoder;

  if (decoder.decode((Message*)&command, buf, len) != Decoder::Status::Error) {
    DEBUG_VERBOSE("ArduinoIoTCloudNotecard::%s [%d] received command id %d", __FUNCTION__, millis(), command.c.id);
    switch (command.c.id)
    {
      case CommandId::ThingUpdateCmdId:
      {
        DEBUG_VERBOSE("ArduinoIoTCloudNotecard::%s [%d] device configuration received", __FUNCTION__, millis());
        String new_thing_id = String(command.thingUpdateCmd.params.thing_id);

        if (!new_thing_id.length()) {
          DEBUG_DEBUG("ArduinoIoTCloudNotecard::%s received null Thing ID.", __FUNCTION__);
          _thing_id = "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx";

          /* Send message to device state machine to inform we have received a null thing-id */
          Message message;
          message = { DeviceRegisteredCmdId };
          _device.handleMessage(&message);
        } else {
          if (_device.isAttached() && _thing_id != new_thing_id) {
            DEBUG_DEBUG("ArduinoIoTCloudNotecard::%s detaching Thing ID: %s", __FUNCTION__, _thing_id.c_str());
            detachThing();
          }
          if (!_device.isAttached()) {
            DEBUG_DEBUG("ArduinoIoTCloudNotecard::%s attaching Thing ID: %s", __FUNCTION__, new_thing_id.c_str());
            attachThing(new_thing_id);
          }
        }
      }
      break;

      case CommandId::ThingDetachCmdId:
      {
        if (!_device.isAttached() || _thing_id != String(command.thingDetachCmd.params.thing_id)) {
          DEBUG_VERBOSE("ArduinoIoTCloudNotecard::%s [%d] thing detach rejected", __FUNCTION__, millis());
        }

        DEBUG_VERBOSE("ArduinoIoTCloudNotecard::%s [%d] thing detach received", __FUNCTION__, millis());
        detachThing();
      }
      break;

      case CommandId::TimezoneCommandDownId:
      {
        DEBUG_VERBOSE("ArduinoIoTCloudNotecard::%s [%d] timezone update received", __FUNCTION__, millis());
        _thing.handleMessage((Message*)&command);
      }
      break;

      case CommandId::LastValuesUpdateCmdId:
      {
        DEBUG_VERBOSE("ArduinoIoTCloudNotecard::%s [%d] last values received", __FUNCTION__, millis());
        CBORDecoder::decode(_thing.getPropertyContainer(),
          (uint8_t*)command.lastValuesUpdateCmd.params.last_values,
          command.lastValuesUpdateCmd.params.length, true);
        _thing.handleMessage((Message*)&command);
        execCloudEventCallback(ArduinoIoTCloudEvent::SYNC);

        /*
         * NOTE: In this current version properties are not properly integrated
         * with the new paradigm of modeling the messages with C structs. The
         * current CBOR library allocates an array in the heap thus we need to
         * delete it after decoding it with the old CBORDecoder
         */
        free(command.lastValuesUpdateCmd.params.last_values);
      }
      break;

#if OTA_ENABLED
      case CommandId::OtaUpdateCmdDownId:
      {
        DEBUG_VERBOSE("ArduinoIoTCloudNotecard::%s [%d] ota update received", __FUNCTION__, millis());
        _ota.handleMessage((Message*)&command);
      }
#endif

      default:
      break;
    }
  }
}

void ArduinoIoTCloudNotecard::processMessage(const uint8_t *buf, size_t len)
{
  DEBUG_VERBOSE("Received %d bytes from cloud. Decoding...", len);
  NotecardConnectionHandler *notecard_connection = reinterpret_cast<NotecardConnectionHandler *>(_connection);
  switch (notecard_connection->getTopicType()) {
    // Commands
    case NotecardConnectionHandler::TopicType::Command:
      processCommand(buf, len);
      break;
    // Telemetry
    case NotecardConnectionHandler::TopicType::Thing:
      CBORDecoder::decode(_thing.getPropertyContainer(), buf, len);
      break;
    default:
      DEBUG_WARNING("Unable to decode unknown topic type: 0x%2X", notecard_connection->getTopicType());
      break;
  }
}

void ArduinoIoTCloudNotecard::sendMessage(Message * msg)
{
  switch (msg->id) {
    case PropertiesUpdateCmdId:
      return sendThingPropertyContainerToCloud();
      break;

    default:
      sendCommandMsgToCloud(msg);
      break;
  }
}

void ArduinoIoTCloudNotecard::sendCommandMsgToCloud(Message * msg_)
{
  size_t bytes_encoded = CBOR_NOTE_MSG_MAX_SIZE;
  uint8_t data[CBOR_NOTE_MSG_MAX_SIZE];
  CBORMessageEncoder encoder;
  NotecardConnectionHandler *notecard_connection = reinterpret_cast<NotecardConnectionHandler *>(_connection);

  if (encoder.encode(msg_, data, bytes_encoded) == Encoder::Status::Complete) {
    if (bytes_encoded > 0) {
      notecard_connection->setTopicType(NotecardConnectionHandler::TopicType::Command);
      notecard_connection->write(data, bytes_encoded);
    }
    DEBUG_DEBUG("Encoded %d bytes for Command Message", bytes_encoded);
  } else {
    DEBUG_ERROR("Failed to encode Command Message");
  }
}

void ArduinoIoTCloudNotecard::sendThingPropertyContainerToCloud()
{
  int bytes_encoded = 0;
  uint8_t data[CBOR_NOTE_MSG_MAX_SIZE];
  NotecardConnectionHandler *notecard_connection = reinterpret_cast<NotecardConnectionHandler *>(_connection);

  // Check if any property needs encoding and send them to the cloud
  if (CBOREncoder::encode(_thing.getPropertyContainer(), data, sizeof(data), bytes_encoded, _thing.getPropertyContainerIndex(), USE_LIGHT_PAYLOADS) == CborNoError) {
    if (bytes_encoded > 0) {
      notecard_connection->setTopicType(NotecardConnectionHandler::TopicType::Thing);
      notecard_connection->write(data, bytes_encoded);
      DEBUG_DEBUG("Encoded %d bytes for Thing properties", bytes_encoded);
    }
  } else {
    DEBUG_ERROR("Failed to encode Thing properties");
  }
}

/******************************************************************************
 * EXTERN DEFINITION
 ******************************************************************************/

ArduinoIoTCloudNotecard ArduinoCloud;

#endif  // USE_NOTECARD
