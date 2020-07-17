/*
   This file is part of ArduinoIoTCloud.

   Copyright 2020 ARDUINO SA (http://www.arduino.cc/)

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

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#ifndef HOST
  #include <AIoTC_Config.h>
#else
  #define OTA_ENABLED (1)
#endif
#if OTA_ENABLED

#include "OTALogic.h"

#ifndef HOST
  #include <Arduino.h>
  #include <Arduino_DebugUtils.h>
#else
  #include <algorithm> /* for std::min, otherwise Arduino defines min() */
  using namespace std;
#endif

#include <string.h>

/******************************************************************************
 * CTOR/DTOR
 ******************************************************************************/

OTALogic::OTALogic()
: _is_configured{false}
, _ota_storage{nullptr}
, _ota_state{OTAState::Init}
, _ota_error{OTAError::None}
{
  init_mqtt_ota_buffer();
  init_ota_binary_data();
}

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

void OTALogic::setOTAStorage(OTAStorage & ota_storage)
{
  _ota_storage = &ota_storage;
  _is_configured = true;
}

OTAError OTALogic::update()
{
  /* This if clause should never happen. None the less we
   * should insure ourselves against this scenario because
   * otherwise we'll have a nullptr dereferencing.
   */
  if (!_is_configured) {
    _ota_state = OTAState::Error;
    _ota_error = OTAError::NoOTAStorageConfigured;
    return _ota_error;
  }

  OTAState prev_ota_state;
  /* The purpose of this loop is to allow the transition of
   * more than one state per a singular call of 'update'. If
   * no state transitition takes place then the loop is exited
   * because it means that external input (such as the arrival)
   * of new data is required.
   */
  do
  {
    prev_ota_state = _ota_state;
    switch(_ota_state)
    {
    case OTAState::Init:           _ota_state = handle_Init          (); break;
    case OTAState::Idle:           _ota_state = handle_Idle          (); break;
    case OTAState::StartDownload:  _ota_state = handle_StartDownload (); break;
    case OTAState::WaitForHeader:  _ota_state = handle_WaitForHeader (); break;
    case OTAState::HeaderReceived: _ota_state = handle_HeaderReceived(); break;
    case OTAState::WaitForBinary:  _ota_state = handle_WaitForBinary (); break;
    case OTAState::BinaryReceived: _ota_state = handle_BinaryReceived(); break;
    case OTAState::Verify:         _ota_state = handle_Verify        (); break;
    case OTAState::Rename:         _ota_state = handle_Rename        (); break;
    case OTAState::Reset:          _ota_state = handle_Reset         (); break;
    case OTAState::Error:                                                break;
    }
  } while(_ota_state != prev_ota_state);

  return _ota_error;
}

void OTALogic::onOTADataReceived(uint8_t const * const data, size_t const length)
{
  size_t const bytes_available = (MQTT_OTA_BUF_SIZE - _mqtt_ota_buf.num_bytes);
  if(length <= bytes_available)
  {
    memcpy(_mqtt_ota_buf.buf + _mqtt_ota_buf.num_bytes, data, length);
    _mqtt_ota_buf.num_bytes += length;
  }
  else
  {
    _ota_state = OTAState::Error;
    _ota_error = OTAError::ReceivedDataOverrun;
  }
}

/******************************************************************************
 * PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

OTAState OTALogic::handle_Init()
{
  DBG_VERBOSE(__PRETTY_FUNCTION__);
  if (_ota_storage->init()) {
    return OTAState::Idle;
  } else {
    _ota_error = OTAError::StorageInitFailed;
    return OTAState::Error;
  }
}

OTAState OTALogic::handle_Idle()
{
  if(_mqtt_ota_buf.num_bytes > 0) {
    return OTAState::StartDownload;
  }
  return OTAState::Idle;
}

OTAState OTALogic::handle_StartDownload()
{
  DBG_VERBOSE(__PRETTY_FUNCTION__);
  if(_ota_storage->open()) {
    return OTAState::WaitForHeader;
  } else {
    _ota_error = OTAError::StorageOpenFailed;
    return OTAState::Error;
  }
}

OTAState OTALogic::handle_WaitForHeader()
{
  DBG_VERBOSE(__PRETTY_FUNCTION__);
  if(_mqtt_ota_buf.num_bytes >= OTA_BINARY_HEADER_SIZE) {
    return OTAState::HeaderReceived;
  }
  return OTAState::WaitForHeader;
}

OTAState OTALogic::handle_HeaderReceived()
{
  DBG_VERBOSE(__PRETTY_FUNCTION__);
  /* The OTA header has been received, let's extract it
   * from the MQTT OTA receive buffer.
   */
  union OTAHeader
  {
    struct __attribute__((packed))
    {
      uint32_t len;
      uint32_t crc32;
    } header;
    uint8_t buf[sizeof(header)];
  };

  OTAHeader ota_header;
  memcpy(ota_header.buf, _mqtt_ota_buf.buf, OTA_BINARY_HEADER_SIZE);

  /* Assign the extracted header values to the member variables */
  _ota_bin_data.hdr_len   = ota_header.header.len;
  _ota_bin_data.hdr_crc32 = ota_header.header.crc32;

  DBG_VERBOSE("%s: header length = %d", __PRETTY_FUNCTION__, _ota_bin_data.hdr_len);
  DBG_VERBOSE("%s: header CRC32 = %d", __PRETTY_FUNCTION__, _ota_bin_data.hdr_crc32);

  /* Reset the counter which is responsible for keeping tabs on how many bytes have been received */
  _ota_bin_data.bytes_received = 0;

  /* Initialize the CRC variable for calculating the CRC32 of the received data package */
  _ota_bin_data.crc32 = crc_init();

  /* Eliminate the header from the receive buffer and transition to WaitForBinary state. */
  memcpy(_mqtt_ota_buf.buf,
         _mqtt_ota_buf.buf + OTA_BINARY_HEADER_SIZE,
         _mqtt_ota_buf.num_bytes - OTA_BINARY_HEADER_SIZE);
  _mqtt_ota_buf.num_bytes -= OTA_BINARY_HEADER_SIZE;

  return OTAState::WaitForBinary;
}

OTAState OTALogic::handle_WaitForBinary()
{
  if (_mqtt_ota_buf.num_bytes > 0) {
    return OTAState::BinaryReceived;
  }
  return OTAState::WaitForBinary;
}

OTAState OTALogic::handle_BinaryReceived()
{
  /* Write to OTA storage */
  if(_ota_storage->write(_mqtt_ota_buf.buf, _mqtt_ota_buf.num_bytes) != _mqtt_ota_buf.num_bytes)
  {
    _ota_error = OTAError::StorageWriteFailed;
    return OTAState::Error;
  }

  /* Update CRC32 */
  _ota_bin_data.crc32 = crc_update(_ota_bin_data.crc32, _mqtt_ota_buf.buf, _mqtt_ota_buf.num_bytes);

  /* Update received data counters and clear MQTT OTA receive buffer */
  _ota_bin_data.bytes_received += _mqtt_ota_buf.num_bytes;
  _mqtt_ota_buf.num_bytes = 0;

  DBG_VERBOSE("%s: %d bytes written", __PRETTY_FUNCTION__, _ota_bin_data.bytes_received);

  if(_ota_bin_data.bytes_received >= _ota_bin_data.hdr_len) {
    _ota_storage->close();
    _ota_bin_data.crc32 = crc_finalize(_ota_bin_data.crc32);
    return OTAState::Verify;
  }

  return OTAState::WaitForBinary;
}

OTAState OTALogic::handle_Verify()
{
  DBG_VERBOSE(__PRETTY_FUNCTION__);
  if(_ota_bin_data.crc32 == _ota_bin_data.hdr_crc32) {
    return OTAState::Rename;
  } else {
    _ota_storage->remove();
    _ota_error = OTAError::ChecksumMismatch;
    return OTAState::Error;
  }
}

OTAState OTALogic::handle_Rename()
{
  DBG_VERBOSE(__PRETTY_FUNCTION__);
  if(_ota_storage->rename()) {
    _ota_storage->deinit();
    return OTAState::Reset;
  }
  else {
    _ota_error = OTAError::RenameOfTempFileFailed;
    return OTAState::Error;
  }
}

OTAState OTALogic::handle_Reset()
{
#if !defined(HOST) && !defined(ESP8266)
  /* After completion of the reset a second stage bootloader
   * such as the SFU is examining the OTA storage medium for
   * the existence of a binary update file. Should such a file
   * exist the 2nd stage bootloader if performing the firmware
   * update before starting the application, otherwise the app
   * is started directly.
   */
  DBG_VERBOSE(__PRETTY_FUNCTION__);
  delay(250);
  NVIC_SystemReset();
#endif /* HOST */
  return OTAState::Reset;
}

void OTALogic::init_mqtt_ota_buffer()
{
  memset(_mqtt_ota_buf.buf, 0U, sizeof(_mqtt_ota_buf.buf));
  _mqtt_ota_buf.num_bytes = 0;
}

void OTALogic::init_ota_binary_data()
{
  _ota_bin_data.hdr_len        = 0;
  _ota_bin_data.hdr_crc32      = 0;
  _ota_bin_data.bytes_received = 0;
  _ota_bin_data.crc32          = crc_init();
}

#endif /* OTA_ENABLED */
