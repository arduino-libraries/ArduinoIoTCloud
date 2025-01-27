/*
  This file is part of the ArduinoIoTCloud library.

  Copyright 2024 Blues (http://www.blues.com/)

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include "AIoTC_Config.h"

#if defined(HAS_NOTECARD)

#include "ArduinoIoTCloudNotecard.h"

#include <algorithm>
#include <functional>

#include <Arduino.h>
#include <NotecardConnectionHandler.h>

#include "cbor/CBOREncoder.h"
#include "cbor/MessageDecoder.h"
#include "cbor/MessageEncoder.h"

/******************************************************************************
 * CONSTANTS
 ******************************************************************************/

static size_t const CBOR_MSG_BUFFER_SIZE = 256;
static size_t const CBOR_LORA_PAYLOAD_MAX_SIZE = 236;
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
  ,_notecard_polling_interval_ms{DEFAULT_READ_INTERVAL_MS}
  ,_interrupt_pin{-1}
  ,_data_available{false}
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

  // Begin the Notecard time service
  _time_service.begin(&connection_);

  // Setup retry timers
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
  DEBUG_INFO("***** Arduino IoT Cloud Notecard - %s *****", AIOT_CONFIG_LIB_VERSION);
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
  // Run through the state machine
  State next_state = _state;
  switch (_state)
  {
  case State::ConnectPhy:           next_state = handle_ConnectPhy();           break;
  case State::SyncTime:             next_state = handle_SyncTime();             break;
  case State::Connected:            next_state = handle_Connected();            break;
  case State::Disconnect:           next_state = handle_Disconnect();           break;
  }
  _state = next_state;
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
    DEBUG_VERBOSE("ArduinoIoTCloudNotecard::%s [%d] internal clock configured to posix timestamp %d", __FUNCTION__, millis(), current_time);
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

  // Poll Notecard for new messages
  pollNotecard();

  // Call CloudDevice process to get configuration
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
    // Call CloudThing process to synchronize properties
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

  // Setup timer for broker connection and restart
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

  bool check_data = ((now_ms - _notecard_last_poll_ms) > _notecard_polling_interval_ms);
  if (interrupts_enabled) {
    check_data = (_data_available || ((now_ms - _notecard_last_poll_ms) > FAILSAFE_READ_INTERVAL_MS));
  }

  if (check_data) {
    result = _connection->available();
    _data_available = (interrupts_enabled && ::digitalRead(_interrupt_pin));
    _notecard_last_poll_ms = now_ms;
  } else {
    result = false;
  }

  return result;
}

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
  if (bytes_received == len && _connection->available()) {
    DEBUG_ERROR("ArduinoIoTCloudNotecard::%s buffer overflow on inbound message", __FUNCTION__);
  } else {
    DEBUG_DEBUG("ArduinoIoTCloudNotecard::%s received %d bytes from cloud", __FUNCTION__, bytes_received);
    len = bytes_received;
  }
}

void ArduinoIoTCloudNotecard::pollNotecard(void)
{
  /* Decode available data. */
  if (available()) {
    size_t note_len = CBOR_MSG_BUFFER_SIZE;
    uint8_t note_buf[CBOR_MSG_BUFFER_SIZE];
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

          // Send message to device state machine to inform we have received a null thing-id
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
  DEBUG_VERBOSE("ArduinoIoTCloudNotecard::%s [%d] decoding %d bytes from cloud...", __FUNCTION__, millis(), len);
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
      sendThingPropertyContainerToCloud();
      break;

    default:
      sendCommandMsgToCloud(msg);
      break;
  }
}

void ArduinoIoTCloudNotecard::sendCommandMsgToCloud(Message * msg_)
{
  size_t bytes_encoded = CBOR_MSG_BUFFER_SIZE;
  uint8_t data[CBOR_MSG_BUFFER_SIZE];
  CBORMessageEncoder encoder;
  NotecardConnectionHandler *notecard_connection = reinterpret_cast<NotecardConnectionHandler *>(_connection);

  if (encoder.encode(msg_, data, bytes_encoded) == Encoder::Status::Complete) {
    if (CBOR_LORA_PAYLOAD_MAX_SIZE < bytes_encoded) {
      DEBUG_WARNING("Encoded %d bytes for Command Message. Exceeds maximum payload size of %d bytes, and cannot be sent to cloud.", bytes_encoded, CBOR_LORA_PAYLOAD_MAX_SIZE);
    } else if (bytes_encoded > 0) {
      DEBUG_DEBUG("ArduinoIoTCloudNotecard::%s encoded %d bytes for Command Message", __FUNCTION__, bytes_encoded);
      notecard_connection->setTopicType(NotecardConnectionHandler::TopicType::Command);
      if (notecard_connection->write(data, bytes_encoded)) {
        DEBUG_ERROR("Failed to send Command Message to cloud");
      } else {
        notecard_connection->initiateNotehubSync(NotecardConnectionHandler::SyncType::Inbound);
      }
    } else {
      DEBUG_DEBUG("ArduinoIoTCloudNotecard::%s encoded zero (0) bytes for Command Message", __FUNCTION__);
    }
  } else {
    DEBUG_ERROR("Failed to encode Command Message");
  }
}

void ArduinoIoTCloudNotecard::sendThingPropertyContainerToCloud()
{
  int bytes_encoded = 0;
  uint8_t data[CBOR_MSG_BUFFER_SIZE];
  NotecardConnectionHandler *notecard_connection = reinterpret_cast<NotecardConnectionHandler *>(_connection);

  // Check if any property needs encoding and send them to the cloud
  if (CBOREncoder::encode(_thing.getPropertyContainer(), data, sizeof(data), bytes_encoded, _thing.getPropertyContainerIndex(), USE_LIGHT_PAYLOADS) == CborNoError) {
    if (static_cast<int>(CBOR_LORA_PAYLOAD_MAX_SIZE) < bytes_encoded) {
      DEBUG_ERROR("Encoded %d bytes for Thing properties. Exceeds maximum encoded payload size of %d bytes, and cannot sync with cloud.", bytes_encoded, CBOR_LORA_PAYLOAD_MAX_SIZE);
    } else if (bytes_encoded < 0) {
      DEBUG_ERROR("Encoding Thing properties resulted in error: %d.", bytes_encoded);
    } else if (bytes_encoded > 0) {
      DEBUG_DEBUG("ArduinoIoTCloudNotecard::%s encoded %d bytes of Thing properties", __FUNCTION__, bytes_encoded);
      notecard_connection->setTopicType(NotecardConnectionHandler::TopicType::Thing);
      if (notecard_connection->write(data, bytes_encoded)) {
        DEBUG_ERROR("Failed to sync Thing properties with cloud");
      }
    }
  } else {
    DEBUG_ERROR("Failed to encode Thing properties");
  }
}

/******************************************************************************
 * EXTERN DEFINITION
 ******************************************************************************/

ArduinoIoTCloudNotecard ArduinoCloud;

#endif  // HAS_NOTECARD
