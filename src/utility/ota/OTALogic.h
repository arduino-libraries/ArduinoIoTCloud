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

#ifndef ARDUINO_OTA_LOGIC_H_
#define ARDUINO_OTA_LOGIC_H_

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#ifndef HOST
  #include <AIoTC_Config.h>
#else
  #define OTA_ENABLED (1)
#endif
#if OTA_ENABLED

#include "OTAStorage.h"

#include "crc.h"

/******************************************************************************
 * CONSTANT
 ******************************************************************************/

static size_t const MQTT_OTA_BUF_SIZE = 256;

/******************************************************************************
 * TYPEDEF
 ******************************************************************************/

enum class OTAState
{
  Init, Idle, StartDownload, WaitForHeader, HeaderReceived, WaitForBinary, BinaryReceived, Verify, Rename, Reset, Error
};

enum class OTAError : int
{
  None                   = 0,
  StorageInitFailed      = 1,
  StorageOpenFailed      = 2,
  StorageWriteFailed     = 3,
  ChecksumMismatch       = 4,
  ReceivedDataOverrun    = 5,
  RenameOfTempFileFailed = 6,
  NoOTAStorageConfigured = 7,
  DownloadFailed         = 8,
};

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class OTALogic
{

public:

  OTALogic();


  void setOTAStorage(OTAStorage & ota_storage);


  OTAError update();
  uint32_t onOTADataReceived(uint8_t const * const data, size_t const length);

#ifdef HOST
  inline OTAState state() const { return _ota_state; }
  inline OTAError error() const { return _ota_error; }
#endif


private:

  typedef struct
  {
    size_t  num_bytes;
    uint8_t buf[MQTT_OTA_BUF_SIZE];
  } sMQTTOTABuffer;

  typedef struct
  {
    uint32_t hdr_len;
    uint32_t hdr_crc32;
    uint32_t bytes_received;
    crc_t    crc32;
  } sOTABinaryData;

  bool _is_configured;
  OTAStorage * _ota_storage;
  OTAState _ota_state;
  OTAError _ota_error;
  sMQTTOTABuffer _mqtt_ota_buf;
  sOTABinaryData _ota_bin_data;

  static size_t const OTA_BINARY_HEADER_SIZE = sizeof(_ota_bin_data.hdr_len) + sizeof(_ota_bin_data.hdr_crc32);

  OTAState handle_Init();
  OTAState handle_Idle();
  OTAState handle_StartDownload();
  OTAState handle_WaitForHeader();
  OTAState handle_HeaderReceived();
  OTAState handle_WaitForBinary();
  OTAState handle_BinaryReceived();
  OTAState handle_Verify();
  OTAState handle_Rename();
  OTAState handle_Reset();

  void init_mqtt_ota_buffer();
  void init_ota_binary_data();

};

#endif /* OTA_ENABLED */

#endif /* ARDUINO_OTA_LOGIC_H_ */
