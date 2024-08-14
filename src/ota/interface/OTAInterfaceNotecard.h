/*
  This file is part of the ArduinoIoTCloud library.

  Copyright 2024 Blues (http://www.blues.com/)

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef OTA_INTERFACE_NOTECARD_H_
#define OTA_INTERFACE_NOTECARD_H_

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <Arduino_ConnectionHandler.h>
#include <ArduinoHttpClient.h>
#include <URLParser.h>

#include "interfaces/MessageStream.h"
#include "ota/interface/OTAInterface.h"
#include "utility/lzss/lzss.h"

/******************************************************************************
 * DEFINES
 ******************************************************************************/

#define OTA_CTX_BUF_LEN 1024

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

/**
 * @brief The OTADefaultCloudProcessInterface class
 *
 * This class is the extension of the abstract class for OTA, with the addition
 * that the download is performed by the MCU itself and not offloaded to the
 * Notecard.
 */
class OTADefaultCloudProcessInterface: public OTACloudProcessInterface {
  public:
    OTADefaultCloudProcessInterface(MessageStream *ms, Client* client=nullptr);
    virtual ~OTADefaultCloudProcessInterface();

    inline virtual void setConnection(ConnectionHandler * connection) override { _connection = connection; }

  protected:
    virtual State startOTA() override;
    virtual State fetch() override;
    virtual void reset() override;
    virtual int writeFlash(uint8_t* const buffer, size_t len) = 0;

  private:
    enum OTADownloadState: uint8_t {
      OtaDownloadHeader,
      OtaDownloadFile,
      OtaDownloadCompleted,
      OtaDownloadMagicNumberMismatch,
      OtaDownloadError
    };

    uint32_t clearNotecardBinaryStore (void);
    size_t downloadBinaryToNotecardFromPath (const char *bin_path);
    size_t loadContextFromNotecardBinaryStore (void);
    void parseOta(uint8_t* buffer, size_t buf_len);

    ConnectionHandler * _connection;

  protected:
    struct Context {
      Context(const char* url, std::function<void(uint8_t)> putc);

      ParsedUrl         parsed_url;
      ota::OTAHeader    header;
      OTADownloadState  downloadState;
      uint32_t          calculatedCrc32;
      uint32_t          headerCopiedBytes;
      uint32_t          decodedSize;
      uint32_t          downloadedSize;
      bool              writeError;

      // Notecard binary store information
      uint32_t          chunk;
      uint32_t          cloud_max;
      uint32_t          cloud_offset;
      uint32_t          cloud_response;
      uint32_t          cloud_total;
      uint32_t          cobs;
      uint32_t          length;
      uint32_t          offset;
      uint32_t          segment;
      uint32_t          try_count;

      // LZSS decoder
      LZSSDecoder       decoder;

      size_t max_xfer_len;
      uint8_t buffer[OTA_CTX_BUF_LEN];
    } *_context;
};

#endif /* OTA_INTERFACE_NOTECARD_H_ */
