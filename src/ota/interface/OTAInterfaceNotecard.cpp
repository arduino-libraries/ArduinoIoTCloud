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
#if OTA_ENABLED && not defined(OFFLOADED_DOWNLOAD)

#include "OTAInterfaceNotecard.h"

#include <Notecard.h>

#include "ota/OTA.h"

/******************************************************************************
 * DEFINES
 ******************************************************************************/

#define OTA_DEFAULT_SEGMENT_SIZE 32768
#define OTA_XFER_MAX_TRIES 3

/******************************************************************************
 * CONSTANTS
 ******************************************************************************/

static const uint32_t crc_table[256] = {
  0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
  0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
  0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
  0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
  0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
  0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
  0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
  0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
  0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
  0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
  0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
  0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
  0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
  0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
  0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
  0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
  0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
  0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
  0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
  0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
  0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
  0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
  0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
  0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
  0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
  0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
  0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
  0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
  0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
  0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
  0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
  0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

/******************************************************************************
 * LOCAL MODULE FUNCTIONS
 ******************************************************************************/

uint32_t crc_update(uint32_t crc, const void * data, size_t data_len) {
  const unsigned char *d = (const unsigned char *)data;
  unsigned int tbl_idx;

  while (data_len--) {
    tbl_idx = (crc ^ *d) & 0xff;
    crc = (crc_table[tbl_idx] ^ (crc >> 8)) & 0xffffffff;
    d++;
  }

  return crc & 0xffffffff;
}

/******************************************************************************
 * CTOR/DTOR
 ******************************************************************************/

OTADefaultCloudProcessInterface::OTADefaultCloudProcessInterface(MessageStream *ms, Client* client)
: OTACloudProcessInterface(ms)
, _connection(nullptr)
, _context(nullptr)
{
  (void)client;  // Unnecessary for this implementation
}

OTADefaultCloudProcessInterface::~OTADefaultCloudProcessInterface() {
  reset();
}

OTADefaultCloudProcessInterface::Context::Context(
  const char* url, std::function<void(uint8_t)> putc)
    : parsed_url(url)
    , downloadState(OtaDownloadHeader)
    , calculatedCrc32(0xFFFFFFFF)
    , headerCopiedBytes(0)
    , decodedSize(0)
    , downloadedSize(0)
    , writeError(false)
    , chunk(0)
    , cloud_max(OTA_DEFAULT_SEGMENT_SIZE)
    , cloud_offset(0)
    , cloud_response(206)
    , cloud_total(0)
    , cobs(0)
    , length(0)
    , offset(0)
    , segment(0)
    , try_count(0)
    , decoder(putc)
    , max_xfer_len(0) { }

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

OTACloudProcessInterface::State OTADefaultCloudProcessInterface::startOTA() {
  assert(OTACloudProcessInterface::context != nullptr);
  assert(_context == nullptr);

  OTACloudProcessInterface::State result;

  DEBUG_DEBUG("OTADefaultCloudProcessInterface::%s initializing download from \"%s\"", __FUNCTION__, OTACloudProcessInterface::context->url);

  // Clear the Notecard binary store
  const uint32_t max_storage = clearNotecardBinaryStore();

  // Establish the context
  _context = new Context(
    OTACloudProcessInterface::context->url,
    [this](uint8_t c) {
      if (this->writeFlash(&c, 1) != 1) {
        this->_context->writeError = true;
      }
    }
  );
  _context->cloud_max = ((max_storage < OTA_DEFAULT_SEGMENT_SIZE) ? max_storage : OTA_DEFAULT_SEGMENT_SIZE);

  // The OTA context operates on a fixed-size buffer (`OTA_CTX_BUF_LEN`), so
  // we need to ensure the encoded data we download does not overrun the buffer.
  // Both `offset` and `length` are based on the decoded data size, so we must
  // use `NoteBinaryCodecMaxDecodedLength()` to determine the maximum length of
  // the data we may safely request.
  _context->max_xfer_len = NoteBinaryCodecMaxDecodedLength(OTA_CTX_BUF_LEN);

  // Validate the context
  if (!_context->cloud_max) {
    DEBUG_ERROR("OTA ERROR: Notecard binary store does not have available space");
    result = OTACloudProcessInterface::State::OtaDownloadFail;
  } else if (strcmp(_context->parsed_url.schema(), "https")) {
    DEBUG_ERROR("OTA ERROR: Invalid URL schema");
    result = OTACloudProcessInterface::State::UrlParseErrorFail;
  } else {
    result = OTACloudProcessInterface::State::Fetch;
  }

  return result;
}

OTACloudProcessInterface::State OTADefaultCloudProcessInterface::fetch() {
  OTACloudProcessInterface::State result;

  const bool bytes_to_process = (_context->offset < _context->length);
  const bool download_complete = (_context->downloadState == OtaDownloadCompleted);
  if (!bytes_to_process && !download_complete) {
    // Prepare for the next segment
    clearNotecardBinaryStore();
    _context->cloud_offset += _context->offset;

    // Download the binary to the Notecard
    size_t http_response_status_code = downloadBinaryToNotecardFromPath(_context->parsed_url.path());
    if ((200 == http_response_status_code) || (206 == http_response_status_code)) {
      _context->downloadedSize = (_context->cloud_offset + _context->length);
      DEBUG_INFO("OTA: Downloaded %u of %u bytes (%u%% complete)", _context->downloadedSize, _context->cloud_total, ((_context->downloadedSize * 100) / _context->cloud_total));
      reportStatus(_context->downloadedSize);
      result = OTACloudProcessInterface::State::Fetch;
    } else {
      DEBUG_ERROR("OTA ERROR: Failed to download binary to Notecard from OTA server");
      result = OTACloudProcessInterface::State::OtaDownloadFail;
    }
  } else if (bytes_to_process) {
    // Transfer binary from Notecard into context buffer
    const uint32_t context_buffer_len = loadContextFromNotecardBinaryStore();
    if (_context->downloadState == OtaDownloadError || _context->downloadState == OtaDownloadMagicNumberMismatch) {
      DEBUG_ERROR("OTA ERROR: Failed to transfer binary from Notecard");
      result = OTACloudProcessInterface::State::OtaDownloadFail;
    } else {
      // Write bytes from the context buffer into device flash.
      parseOta(_context->buffer, context_buffer_len);
      if (_context->downloadState == OtaDownloadError) {
        result = OTACloudProcessInterface::State::OtaDownloadFail;
      } else if (_context->downloadState == OtaDownloadMagicNumberMismatch) {
        result = OTACloudProcessInterface::State::OtaHeaderMagicNumberFail;
      } else if (_context->writeError) {
        DEBUG_ERROR("OTA ERROR: Failed to write binary to device flash");
        result = OTACloudProcessInterface::State::ErrorWriteUpdateFileFail;
      } else {
        result = OTACloudProcessInterface::State::Fetch;
      }
    }
  } else if (download_complete) {
    // Validate CRC
    _context->calculatedCrc32 ^= 0xFFFFFFFF; // finalize CRC
    if (_context->header.header.crc32 == _context->calculatedCrc32) {
      DEBUG_INFO("OTA: OTA binary passed CRC validation");
      result = OTACloudProcessInterface::State::FlashOTA;
    } else {
      DEBUG_ERROR("OTA ERROR: OTA binary failed CRC validation");
      result = OTACloudProcessInterface::State::OtaHeaderCrcFail;
    }
  } else {
    // It is not possible to reach this point
    DEBUG_VERBOSE("OTADefaultCloudProcessInterface::%s [%d] ERROR! Hacking too much time!", __FUNCTION__, ::millis());
    result = OTACloudProcessInterface::State::OtaDownloadFail;
  }

  return result;
}

void OTADefaultCloudProcessInterface::reset() {
  // free the context pointer
  if(_context != nullptr) {
    delete _context;
    _context = nullptr;
  }
  clearNotecardBinaryStore();
  DEBUG_DEBUG("OTADefaultCloudProcessInterface::%s complete", __FUNCTION__);
}

/******************************************************************************
 * PRIVATE SUPPORT FUNCTIONS
 ******************************************************************************/

uint32_t OTADefaultCloudProcessInterface::clearNotecardBinaryStore (void) {
  uint32_t result;

  const Notecard &notecard = reinterpret_cast<NotecardConnectionHandler *>(_connection)->getNotecard();

  if (J *req = notecard.newRequest("card.binary")) {
    JAddBoolToObject(req, "delete", true);
    if (J *rsp = notecard.requestAndResponse(req)) {
      // Check the response for errors
      if (notecard.responseError(rsp)) {
        const char *err = JGetString(rsp, "err");
        DEBUG_ERROR("%s\n", err);
        result = 0;
      } else {
        result = static_cast<uint32_t>(JGetInt(rsp, "max"));
        DEBUG_DEBUG("OTADefaultCloudProcessInterface::%s emptied Notecard binary store; %d bytes available", __FUNCTION__, result);
      }
      notecard.deleteResponse(rsp);
    } else {
      DEBUG_ERROR("OTA ERROR: Failed to receive response from Notecard");
      result = 0;
    }
  } else {
    DEBUG_ERROR("OTA ERROR: Failed to allocate request: card.binary");
    result = 0;
  }

  return result;
}

size_t OTADefaultCloudProcessInterface::downloadBinaryToNotecardFromPath (const char *bin_path_) {
  ssize_t result;

  const Notecard &notecard = reinterpret_cast<NotecardConnectionHandler *>(_connection)->getNotecard();

  /** Due to the space constraints of the Notecard, the binary file cannot be
   * downloaded in a single request, and segmentation of the binary is required.
   * As a result, the application remains respoonsive, and there is no need to
   * implement the watchdog feature of the `ArduinoIoTCloud` library.
   */

  DEBUG_DEBUG("OTADefaultCloudProcessInterface::%s downloading segment %u (range %u to %u) from path \"%s\"...", __FUNCTION__, ++_context->segment, _context->cloud_offset, (_context->cloud_offset + _context->cloud_max), _context->parsed_url.path());

  if (J *req = notecard.newRequest("web.get")) {
    JAddStringToObject(req, "route", "arduino-iot-cloud.ota");
    JAddStringToObject(req, "name", bin_path_);
    JAddStringToObject(req, "content", "application/octet-stream");
    JAddBoolToObject(req, "binary", true);
    JAddIntToObject(req, "offset", _context->cloud_offset);
    JAddIntToObject(req, "max", _context->cloud_max);
    JAddIntToObject(req, "seconds", 600);
    if (J *rsp = notecard.requestAndResponse(req)) {
      // Check the response for errors
      if (notecard.responseError(rsp)) {
        const char *err = JGetString(rsp, "err");
        DEBUG_ERROR("%s\n", err);
        result = 0;
      } else {
        _context->chunk = 0;
        _context->cloud_total = static_cast<uint32_t>(JGetInt(rsp, "total"));
        _context->cobs = static_cast<uint32_t>(JGetInt(rsp, "cobs"));
        _context->length = static_cast<uint32_t>(JGetInt(rsp, "length"));
        _context->offset = 0;
        _context->cloud_response = static_cast<uint32_t>(JGetInt(rsp, "result"));
        DEBUG_DEBUG("OTADefaultCloudProcessInterface::%s captured context | cloud_max: %u cloud_offset: %u cloud_response: %u cloud_total: %u cobs: %u length: %u offset: %u", __FUNCTION__, _context->cloud_max, _context->cloud_offset, _context->cloud_response, _context->cloud_total, _context->cobs, _context->length, _context->offset);
        if (200 != _context->cloud_response && 206 != _context->cloud_response) {
          _context->downloadState = OtaDownloadError;
          DEBUG_ERROR("OTA ERROR: Received error response code: %u", _context->cloud_response);
          const size_t http_response_msg_len = NoteBinaryCodecMaxEncodedLength(_context->length);
          char * http_response_msg = (char *)malloc(http_response_msg_len);
          if (http_response_msg == nullptr) {
            DEBUG_ERROR("OTA ERROR: Failed to allocate memory for error message");
          } else if (NoteBinaryStoreReceive(reinterpret_cast<uint8_t *>(http_response_msg), http_response_msg_len, _context->offset, _context->length)) {
            DEBUG_ERROR("OTA ERROR: Failed to retrieve error message from Notecard binary store");
          } else {
            http_response_msg[_context->length] = '\0';
            DEBUG_ERROR("OTA ERROR: %s", http_response_msg);
          }
          free(http_response_msg);
          clearNotecardBinaryStore();
        } else {
          DEBUG_DEBUG("OTADefaultCloudProcessInterface::%s downloaded %u cobs encoded bytes from OTA server", __FUNCTION__, _context->cobs);
        }
        result = _context->cloud_response;
      }
      notecard.deleteResponse(rsp);
    } else {
      DEBUG_ERROR("OTA ERROR: Failed to receive response from Notecard");
      result = 0;
    }
  } else {
    DEBUG_ERROR("OTA ERROR: Failed to allocate request: web.get");
    result = 0;
  }

  return result;
}

size_t OTADefaultCloudProcessInterface::loadContextFromNotecardBinaryStore (void) {
  size_t result;

  ++_context->try_count;
  const uint32_t bytes_remaining = (_context->length - _context->offset);
  const uint32_t rx_len = (( bytes_remaining > _context->max_xfer_len)
                            ? _context->max_xfer_len
                            : bytes_remaining);

  // Transfer the chunk into the context buffer
  DEBUG_DEBUG("OTADefaultCloudProcessInterface::%s transferring chunk %u of segment %u...", __FUNCTION__, ++_context->chunk, _context->segment);
  if (NoteBinaryStoreReceive(_context->buffer, OTA_CTX_BUF_LEN, _context->offset, rx_len)) {
    --_context->chunk;
    DEBUG_WARNING("OTA WARNING: Failed to receive chunk from Notecard binary store");
    // Retry the chunk for max retries
    if (_context->try_count <= OTA_XFER_MAX_TRIES) {
      DEBUG_INFO("OTA: Will attempt to retry the chunk transfer %d more times", (OTA_XFER_MAX_TRIES - _context->try_count));
    } else {
      _context->downloadState = OtaDownloadError;
      DEBUG_ERROR("OTA ERROR: Failed to receive chunk from Notecard binary store");
    }
    result = 0;
  } else {
    // Clear try count
    _context->try_count = 0;

    // Update the offset
    _context->offset += rx_len;

    DEBUG_DEBUG("OTADefaultCloudProcessInterface::%s transferred %u of %u bytes (segment %u%% complete)", __FUNCTION__, _context->offset, _context->length, ((_context->offset * 100) / _context->length));
    result = rx_len;

    // Log for the sake of curiosity
    DEBUG_VERBOSE("OTADefaultCloudProcessInterface::%s [%d] *** Decoded Binary Data ***", __FUNCTION__, ::millis());
    for (size_t i = 0 ; i < rx_len ; i += 4) {
      if ((i + 4) <= rx_len) {
        DEBUG_VERBOSE("OTADefaultCloudProcessInterface::%s [%d] %02X %02X %02X %02X", __FUNCTION__, ::millis(), _context->buffer[i], _context->buffer[(i + 1)], _context->buffer[(i + 2)], _context->buffer[(i + 3)]);
      } else if ((i + 3) == rx_len) {
        DEBUG_VERBOSE("OTADefaultCloudProcessInterface::%s [%d] %02X %02X %02X --", __FUNCTION__, ::millis(), _context->buffer[i], _context->buffer[(i + 1)], _context->buffer[(i + 2)]);
      } else if ((i + 2) == rx_len) {
        DEBUG_VERBOSE("OTADefaultCloudProcessInterface::%s [%d] %02X %02X -- --", __FUNCTION__, ::millis(), _context->buffer[i], _context->buffer[(i + 1)]);
      } else {
        DEBUG_VERBOSE("OTADefaultCloudProcessInterface::%s [%d] %02X -- -- --", __FUNCTION__, ::millis(), _context->buffer[i]);
      }
    }
    DEBUG_VERBOSE("OTADefaultCloudProcessInterface::%s [%d] *** Decoded Binary Data ***", __FUNCTION__, ::millis());
  }

  return result;
}

void OTADefaultCloudProcessInterface::parseOta(uint8_t * buffer_, size_t buf_len_) {
  assert(_context != nullptr); // This should never fail

  for (uint8_t *cursor = buffer_ ; cursor < (buffer_ + buf_len_) ; ) {
    switch(_context->downloadState) {
      case OtaDownloadHeader: {
        DEBUG_VERBOSE("OTADefaultCloudProcessInterface::%s [%d] parsing OTA header...", __FUNCTION__, ::millis());
        const uint32_t header_bytes_remaining = (sizeof(_context->header.buf) - _context->headerCopiedBytes);
        const bool more_header_bytes = (buf_len_ < header_bytes_remaining);
        const uint32_t copied = (more_header_bytes ? buf_len_ : header_bytes_remaining);
        memcpy((_context->header.buf + _context->headerCopiedBytes), cursor, copied);
        cursor += copied;
        _context->headerCopiedBytes += copied;

        if (more_header_bytes) {
          DEBUG_DEBUG("OTADefaultCloudProcessInterface::%s copied %u of %u header bytes", __FUNCTION__, _context->headerCopiedBytes, sizeof(_context->header.buf));
          _context->downloadState = OtaDownloadHeader;
        } else if (sizeof(_context->header.buf) == _context->headerCopiedBytes) {
          _context->calculatedCrc32 = crc_update(
            _context->calculatedCrc32,
            &(_context->header.header.magic_number),
            (sizeof(_context->header) - offsetof(ota::OTAHeader, header.magic_number))
          );

          // Validate the number of bytes to be downloaded
          const uint32_t legacy_excluded_bytes = (sizeof(_context->header.header.len) + sizeof(_context->header.header.crc32));
          if ((_context->header.header.len + legacy_excluded_bytes) != _context->cloud_total) {
            DEBUG_ERROR("OTA ERROR: OTA binary length mismatch (header: (%u + %u) != actual: %u)", _context->header.header.len, legacy_excluded_bytes, _context->cloud_total);
            _context->downloadState = OtaDownloadError;
            return;
          }

          // Validate the magic number
          if (_context->header.header.magic_number != OtaMagicNumber) {
            DEBUG_ERROR("OTA ERROR: Magic number mismatch");
            _context->downloadState = OtaDownloadMagicNumberMismatch;
            return;
          }

          DEBUG_DEBUG("OTADefaultCloudProcessInterface::%s parsed OTA Header | len: %u crc32: %u magic_number: 0x%08X hdr_version | header_version: %u compression: %s signature: %s spare: %u payload_target: %u payload_major: %u payload_minor: %u payload_patch: %u payload_build_num: %u", __FUNCTION__, _context->header.header.len, _context->header.header.crc32, _context->header.header.magic_number, _context->header.header.hdr_version.field.header_version, (_context->header.header.hdr_version.field.compression ? "true" : "false"), (_context->header.header.hdr_version.field.signature ? "true" : "false"), _context->header.header.hdr_version.field.spare, _context->header.header.hdr_version.field.payload_target, _context->header.header.hdr_version.field.payload_major, _context->header.header.hdr_version.field.payload_minor, _context->header.header.hdr_version.field.payload_patch, _context->header.header.hdr_version.field.payload_build_num);
          _context->downloadState = OtaDownloadFile;
        } else {
          DEBUG_ERROR("OTA ERROR: Failed to parse OTA header");
          _context->downloadState = OtaDownloadError;
        }
        break;
      }
      case OtaDownloadFile: {
        DEBUG_VERBOSE("OTADefaultCloudProcessInterface::%s [%d] parsing OTA file...", __FUNCTION__, ::millis());

        // Decompress and write to flash
        const uint32_t header_bytes = (cursor - buffer_);
        if (!_context->decoder.decompress(cursor, (buf_len_ - header_bytes))) {
          DEBUG_DEBUG("OTADefaultCloudProcessInterface::%s completed data decompression", __FUNCTION__);
        }

        _context->calculatedCrc32 = crc_update(
          _context->calculatedCrc32,
          cursor,
          (buf_len_ - header_bytes)
        );

        // Advance the cursor
        const uint32_t decoded_bytes = (buf_len_ - header_bytes);
        cursor += decoded_bytes;
        _context->decodedSize += decoded_bytes;
        DEBUG_VERBOSE("OTADefaultCloudProcessInterface::%s [%d] decoded and wrote %d of %d bytes to flash", __FUNCTION__, ::millis(), _context->decodedSize, (_context->cloud_total - _context->headerCopiedBytes));

        // Verify there are no more bytes available once the download has completed
        const bool download_complete = (_context->downloadedSize == _context->cloud_total);
        const bool transfer_complete = (_context->offset == _context->length);
        if (_context->downloadedSize > _context->cloud_total) {
          DEBUG_ERROR("OTA ERROR: Downloaded more bytes than expected");
          _context->downloadState = OtaDownloadError;
        } else if (download_complete && transfer_complete) {
          _context->downloadState = OtaDownloadCompleted;
        }
        break;
      }
      case OtaDownloadCompleted:
        DEBUG_VERBOSE("OTADefaultCloudProcessInterface::%s [%d] download complete - exiting - no data to parse", __FUNCTION__, ::millis());
        return;
      default:
        DEBUG_ERROR("OTA ERROR: invalid download state: %d", _context->downloadState);
        _context->downloadState = OtaDownloadError;
        return;
    }
  }
}

#endif // OTA_ENABLED && not defined(OFFLOADED_DOWNLOAD)
#endif // HAS_NOTECARD
