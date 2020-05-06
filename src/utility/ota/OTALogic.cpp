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

#include "OTALogic.h"

#ifndef HOST
  #include <Arduino.h>
#else
  #include <algorithm> /* for std::min, otherwise Arduino defines min() */
  using namespace std;
#endif

#include <string.h>

/******************************************************************************
 * CTOR/DTOR
 ******************************************************************************/

OTALogic::OTALogic()
: _ota_state{OTAState::Init}
, _mqtt_ota_buf{0, {0}}
, _ota_bin_data{0, 0, 0, crc_init()}
{

}

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

void OTALogic::update(OTAStorage * ota_storage)
{
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
    case OTAState::Init:           _ota_state = handle_Init          (ota_storage); break;
    case OTAState::Idle:           _ota_state = handle_Idle          ();            break;
    case OTAState::StartDownload:  _ota_state = handle_StartDownload (ota_storage); break;
    case OTAState::WaitForHeader:  _ota_state = handle_WaitForHeader ();            break;
    case OTAState::HeaderReceived: _ota_state = handle_HeaderReceived();            break;
    case OTAState::WaitForBinary:  _ota_state = handle_WaitForBinary ();            break;
    case OTAState::BinaryReceived: _ota_state = handle_BinaryReceived(ota_storage); break;
    case OTAState::Verify:         _ota_state = handle_Verify        (ota_storage); break;
    case OTAState::Reset:          _ota_state = handle_Reset         ();            break;
    case OTAState::Error:                                                           break;
    }
  } while(_ota_state != prev_ota_state);
}

void OTALogic::onOTADataReceived(uint8_t const * const data, size_t const length)
{
  size_t const bytes_available = (MQTT_OTA_BUF_SIZE - _mqtt_ota_buf.num_bytes);
  size_t const bytes_to_copy = min(bytes_available, length);
  memcpy(_mqtt_ota_buf.buf + _mqtt_ota_buf.num_bytes, data, bytes_to_copy);
  _mqtt_ota_buf.num_bytes += bytes_to_copy;
}

/******************************************************************************
 * PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

OTAState OTALogic::handle_Init(OTAStorage * ota_storage)
{
  if (ota_storage && ota_storage->init()) {
    return OTAState::Idle;
  }
  return OTAState::Error;
}

OTAState OTALogic::handle_Idle()
{
  if(_mqtt_ota_buf.num_bytes > 0) {
    return OTAState::StartDownload;
  }
  return OTAState::Idle;
}

OTAState OTALogic::handle_StartDownload(OTAStorage * ota_storage)
{
  if(ota_storage && ota_storage->open()) {
    return OTAState::WaitForHeader;
  }
  return OTAState::Error;
}

OTAState OTALogic::handle_WaitForHeader()
{
  if(_mqtt_ota_buf.num_bytes >= OTA_BINARY_HEADER_SIZE) {
    return OTAState::HeaderReceived;
  }
  return OTAState::WaitForHeader;
}

OTAState OTALogic::handle_HeaderReceived()
{
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

OTAState OTALogic::handle_BinaryReceived(OTAStorage * ota_storage)
{
  if(!ota_storage) return OTAState::Error;

  /* Write to OTA storage */
  ota_storage->write(_mqtt_ota_buf.buf, _mqtt_ota_buf.num_bytes);

  /* Update CRC32 */
  _ota_bin_data.crc32 = crc_update(_ota_bin_data.crc32, _mqtt_ota_buf.buf, _mqtt_ota_buf.num_bytes);

  /* Update received data counters and clear MQTT OTA receive buffer */
  _ota_bin_data.bytes_received += _mqtt_ota_buf.num_bytes;
  _mqtt_ota_buf.num_bytes = 0;

  if(_ota_bin_data.bytes_received >= _ota_bin_data.hdr_len) {
    ota_storage->close();
    _ota_bin_data.crc32 = crc_finalize(_ota_bin_data.crc32);
    return OTAState::Verify;
  }

  return OTAState::WaitForBinary;
}

OTAState OTALogic::handle_Verify(OTAStorage * ota_storage)
{
  if(!ota_storage) return OTAState::Error;

  if(_ota_bin_data.crc32 == _ota_bin_data.hdr_crc32) {
    ota_storage->deinit();
    return OTAState::Reset;
  } else {
    ota_storage->remove();
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
  NVIC_SystemReset();
#endif /* HOST */
  return OTAState::Reset;
}
