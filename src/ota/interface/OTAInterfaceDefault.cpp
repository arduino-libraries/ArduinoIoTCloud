/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include <AIoTC_Config.h>

#if OTA_ENABLED && ! defined(OFFLOADED_DOWNLOAD)
#include <Arduino_CRC32.h>
#include "OTAInterfaceDefault.h"
#include "../OTA.h"

OTADefaultCloudProcessInterface::OTADefaultCloudProcessInterface(MessageStream *ms, Client* client)
: OTACloudProcessInterface(ms)
, client(client)
, http_client(nullptr)
, username(nullptr), password(nullptr)
, context(nullptr) {
}

OTADefaultCloudProcessInterface::~OTADefaultCloudProcessInterface() {
  reset();
}

OTACloudProcessInterface::State OTADefaultCloudProcessInterface::startOTA() {
  assert(client != nullptr);
  assert(OTACloudProcessInterface::context != nullptr);
  assert(context == nullptr);

  context = new Context(
    OTACloudProcessInterface::context->url,
    [this](uint8_t c) {
        if (this->writeFlash(&c, 1) != 1) {
          this->context->writeError = true;
        }
    }
  );

  // check url
  if(strcmp(context->parsed_url.schema(), "https") == 0) {
    http_client = new HttpClient(*client, context->parsed_url.host(), context->parsed_url.port());
  } else {
    return UrlParseErrorFail;
  }

  // make the http get request
  OTACloudProcessInterface::State res = requestOta();
  if(res != Fetch) {
    return res;
  }

  // The following call is required to save the header value , keep it
  if(http_client->contentLength() == HttpClient::kNoContentLengthHeader) {
    DEBUG_VERBOSE("OTA ERROR: the response header doesn't contain \"ContentLength\" field");
    return HttpHeaderErrorFail;
  }

  context->contentLength = http_client->contentLength();
  context->lastReportTime = millis();
  DEBUG_VERBOSE("OTA file length: %d", context->contentLength);
  return Fetch;
}

OTACloudProcessInterface::State OTADefaultCloudProcessInterface::fetch() {
  OTACloudProcessInterface::State res = Fetch;

  if(getOtaPolicy(ChunkDownload)) {
    res = requestOta(ChunkDownload);
  }

  context->downloadedChunkSize = 0;
  context->downloadedChunkStartTime = millis();

  if(res != Fetch) {
    goto exit;
  }

  /* download chunked or timed  */
  do {
    if(!http_client->connected()) {
      res = OtaDownloadFail;
      goto exit;
    }

    if(http_client->available() == 0) {
      /* Avoid tight loop and allow yield */
      delay(1);
      continue;
    }

    int http_res = http_client->read(context->buffer, context->bufLen);

    if(http_res < 0) {
      DEBUG_VERBOSE("OTA ERROR: Download read error %d", http_res);
      res = OtaDownloadFail;
      goto exit;
    }

    parseOta(context->buffer, http_res);

    if(context->writeError) {
      DEBUG_VERBOSE("OTA ERROR: File write error");
      res = ErrorWriteUpdateFileFail;
      goto exit;
    }

    context->downloadedChunkSize += http_res;

  } while(context->downloadState < OtaDownloadCompleted && fetchMore());

  // TODO verify that the information present in the ota header match the info in context
  if(context->downloadState == OtaDownloadCompleted) {
    // Verify that the downloaded file size is matching the expected size ??
    // this could distinguish between consistency of the downloaded bytes and filesize

    // validate CRC
    context->calculatedCrc32 = arduino::crc32::finalize(context->calculatedCrc32);
    if(context->header.header.crc32 == context->calculatedCrc32) {
      DEBUG_VERBOSE("Ota download completed successfully");
      res = FlashOTA;
    } else {
      res = OtaHeaderCrcFail;
    }
  } else if(context->downloadState == OtaDownloadError) {
    DEBUG_VERBOSE("OTA ERROR: OtaDownloadError");

    res = OtaDownloadFail;
  } else if(context->downloadState == OtaDownloadMagicNumberMismatch) {
    DEBUG_VERBOSE("OTA ERROR: Magic number mismatch");
    res = OtaHeaderMagicNumberFail;
  }

exit:
  if(res != Fetch) {
    http_client->stop(); // close the connection
    delete http_client;
    http_client = nullptr;
  }
  return res;
}

OTACloudProcessInterface::State OTADefaultCloudProcessInterface::requestOta(OtaFlags mode) {
  int http_res = 0;

  /* stop connected client */
  http_client->stop();

  /* request chunk */
  http_client->beginRequest();
  http_res = http_client->get(context->parsed_url.path());

  if(username != nullptr && password != nullptr) {
    http_client->sendBasicAuth(username, password);
  }

  if((mode & ChunkDownload) == ChunkDownload) {
    char range[128] = {0};
    size_t rangeSize = context->downloadedSize + maxChunkSize > context->contentLength ? context->contentLength - context->downloadedSize : maxChunkSize;
    sprintf(range, "bytes=%" PRIu32 "-%" PRIu32, context->downloadedSize, context->downloadedSize + rangeSize);
    DEBUG_VERBOSE("OTA downloading range: %s", range);
    http_client->sendHeader("Range", range);
  }

  http_client->endRequest();

  if(http_res == HTTP_ERROR_CONNECTION_FAILED) {
    DEBUG_VERBOSE("OTA ERROR: http client error connecting to server \"%s:%d\"",
      context->parsed_url.host(), context->parsed_url.port());
    return ServerConnectErrorFail;
  } else if(http_res == HTTP_ERROR_TIMED_OUT) {
    DEBUG_VERBOSE("OTA ERROR: http client timeout \"%s\"", OTACloudProcessInterface::context->url);
    return OtaHeaderTimeoutFail;
  } else if(http_res != HTTP_SUCCESS) {
    DEBUG_VERBOSE("OTA ERROR: http client returned %d on  get \"%s\"", http_res, OTACloudProcessInterface::context->url);
    return OtaDownloadFail;
  }

  int statusCode = http_client->responseStatusCode();

  if((((mode & ChunkDownload) == ChunkDownload) && (statusCode != 206)) ||
     (((mode & ChunkDownload) != ChunkDownload) && (statusCode != 200))) {
    DEBUG_VERBOSE("OTA ERROR: get response on \"%s\" returned status %d", OTACloudProcessInterface::context->url, statusCode);
    return HttpResponseFail;
  }

  http_client->skipResponseHeaders();
  return Fetch;
}

bool OTADefaultCloudProcessInterface::fetchMore() {
  if (getOtaPolicy(ChunkDownload)) {
    return context->downloadedChunkSize < maxChunkSize;
  } else {
    return (millis() - context->downloadedChunkStartTime) < downloadTime;
  }
}

void OTADefaultCloudProcessInterface::parseOta(uint8_t* buffer, size_t bufLen) {
  assert(context != nullptr); // This should never fail

  for(uint8_t* cursor=(uint8_t*)buffer; cursor<buffer+bufLen; ) {
    switch(context->downloadState) {
    case OtaDownloadHeader: {
      const uint32_t headerLeft = context->headerCopiedBytes + bufLen <= sizeof(context->header.buf) ? bufLen : sizeof(context->header.buf) - context->headerCopiedBytes;
      memcpy(context->header.buf+context->headerCopiedBytes, buffer, headerLeft);
      cursor += headerLeft;
      context->headerCopiedBytes += headerLeft;

      // when finished go to next state
      if(sizeof(context->header.buf) == context->headerCopiedBytes) {
        context->downloadState = OtaDownloadFile;

        context->calculatedCrc32 = arduino::crc32::update(
          context->calculatedCrc32,
          &(context->header.header.magic_number),
          sizeof(context->header) - offsetof(ota::OTAHeader, header.magic_number)
        );

        if(context->header.header.magic_number != OtaMagicNumber) {
          context->downloadState = OtaDownloadMagicNumberMismatch;
          return;
        }
        context->downloadedSize += sizeof(context->header.buf);
      }

      break;
    }
    case OtaDownloadFile: {
      const uint32_t dataLeft = bufLen - (cursor-buffer);
      context->decoder.decompress(cursor, dataLeft); // TODO verify return value

      context->calculatedCrc32 = arduino::crc32::update(
          context->calculatedCrc32,
          cursor,
          dataLeft
        );

      cursor += dataLeft;
      context->downloadedSize += dataLeft;

      if((millis() - context->lastReportTime) > 10000) { // Report the download progress each X millisecond
        DEBUG_VERBOSE("OTA Download Progress %d/%d", context->downloadedSize, context->contentLength);

        reportStatus(context->downloadedSize);
        context->lastReportTime = millis();
      }

      // TODO there should be no more bytes available when the download is completed
      if(context->downloadedSize == context->contentLength) {
        context->downloadState = OtaDownloadCompleted;
      }

      if(context->downloadedSize > context->contentLength) {
        context->downloadState = OtaDownloadError;
      }
      // TODO fail if we exceed a timeout? and available is 0 (client is broken)
      break;
    }
    case OtaDownloadCompleted:
      return;
    default:
      context->downloadState = OtaDownloadError;
      return;
    }
  }
}

void OTADefaultCloudProcessInterface::reset() {
  if(http_client != nullptr) {
    http_client->stop(); // close the connection
    delete http_client;
    http_client = nullptr;
  }

  if(client!=nullptr && client->connected()) {
    client->stop();
  }

  // free the context pointer
  if(context != nullptr) {
    delete context;
    context = nullptr;
  }
}

OTADefaultCloudProcessInterface::Context::Context(
  const char* url, std::function<void(uint8_t)> putc)
    : parsed_url(url)
    , downloadState(OtaDownloadHeader)
    , calculatedCrc32(arduino::crc32::begin())
    , headerCopiedBytes(0)
    , downloadedSize(0)
    , lastReportTime(0)
    , contentLength(0)
    , writeError(false)
    , downloadedChunkSize(0)
    , decoder(putc) { }

#endif /* OTA_ENABLED && ! defined(OFFLOADED_DOWNLOAD) */
