/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <AIoTC_Config.h>

#if OTA_ENABLED && ! defined(OFFLOADED_DOWNLOAD)
#include <Arduino.h>

#include <ArduinoHttpClient.h>
#include <URLParser.h>
#include "utility/lzss/lzss.h"
#include "OTAInterface.h"

/**
 * This class is the extension of the abstract class for OTA, with the addition that
 * the download is performed by the mcu itself and not offloaded to the network peripheral
 */
class OTADefaultCloudProcessInterface: public OTACloudProcessInterface {
public:
  OTADefaultCloudProcessInterface(MessageStream *ms, Client* client=nullptr);
  ~OTADefaultCloudProcessInterface();

  inline virtual void setClient(Client* c) { client = c; }

  void setAuthentication(const char* const username, const char* const password) {
    this->username = username;
    this->password = password;
  }

protected:
  State startOTA();
  State fetch();
  void reset();
  virtual int writeFlash(uint8_t* const buffer, size_t len) = 0;

private:
  void parseOta(uint8_t* buffer, size_t bufLen);
  State requestOta(OtaFlags mode = None);
  bool fetchMore();

  Client*     client;
  HttpClient* http_client;

  const char *username, *password;

  // The amount of time that each iteration of Fetch has to take at least
  // This mitigate the issues arising from tasks run in main loop that are using all the computing time
  static constexpr uint32_t downloadTime = 2000;

  // The amount of data that each iteration of Fetch has to take at least
  // This should be enabled setting ChunkDownload OtaFlag to 1 and mitigate some Ota corner cases
  static constexpr size_t maxChunkSize = 1024 * 10;

  enum OTADownloadState: uint8_t {
    OtaDownloadHeader,
    OtaDownloadFile,
    OtaDownloadCompleted,
    OtaDownloadMagicNumberMismatch,
    OtaDownloadError
  };

protected:
  struct Context {
    Context(
      const char* url,
      std::function<void(uint8_t)> putc);

    ParsedUrl         parsed_url;
    ota::OTAHeader    header;
    OTADownloadState  downloadState;
    uint32_t          calculatedCrc32;
    uint32_t          headerCopiedBytes;
    uint32_t          downloadedSize;
    uint32_t          lastReportTime;
    uint32_t          contentLength;
    bool              writeError;

    uint32_t          downloadedChunkStartTime;
    uint32_t          downloadedChunkSize;

    // LZSS decoder
    LZSSDecoder       decoder;

    static constexpr size_t bufLen = 64;
    uint8_t buffer[bufLen];
  } *context;
};

#endif /* OTA_ENABLED && ! defined(OFFLOADED_DOWNLOAD) */
