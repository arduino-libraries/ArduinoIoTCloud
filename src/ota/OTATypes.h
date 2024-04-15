/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include "AIoTC_Config.h"

#if OTA_ENABLED
#include <stdint.h>

namespace ota {
  enum class OTAError : int16_t {
    None                  = 0,
    NoCapableBootloader   = -1,
    NoOtaStorage          = -2,
    OtaStorageInit        = -3,
    OtaStorageOpen        = -4,
    OtaHeaderLength       = -5,
    OtaHeaderCrc          = -6,
    OtaHeaterMagicNumber  = -7,
    ParseHttpHeader       = -8,
    UrlParseError         = -9,
    ServerConnectError    = -10,
    HttpHeaderError       = -11,
    OtaDownload           = -12,
    OtaHeaderTimeout      = -13,
    HttpResponse          = -14,
    OtaStorageEnd         = -15,
    StorageConfig         = -16,
    Library               = -17,
    Modem                 = -18,
    ErrorOpenUpdateFile   = -19,
    ErrorWriteUpdateFile  = -20,
    ErrorReformat         = -21,
    ErrorUnmount          = -22,
    ErrorRename           = -23,
    CaStorageInit         = -24,
    CaStorageOpen         = -25,
  };

#ifndef OFFLOADED_DOWNLOAD
  union HeaderVersion {
    struct __attribute__((packed)) {
      uint32_t header_version    :  6;
      uint32_t compression       :  1;
      uint32_t signature         :  1;
      uint32_t spare             :  4;
      uint32_t payload_target    :  4;
      uint32_t payload_major     :  8;
      uint32_t payload_minor     :  8;
      uint32_t payload_patch     :  8;
      uint32_t payload_build_num : 24;
    } field;
    uint8_t buf[sizeof(field)];
    static_assert(sizeof(buf) == 8, "Error: sizeof(HEADER.VERSION) != 8");
  };

  union OTAHeader {
    struct __attribute__((packed)) {
      uint32_t len;
      uint32_t crc32;
      uint32_t magic_number;
      HeaderVersion hdr_version;
    } header;
    uint8_t buf[sizeof(header)];
    static_assert(sizeof(buf) == 20, "Error: sizeof(HEADER) != 20");
  };
#endif // OFFLOADED_DOWNLOAD
}

#endif // OTA_ENABLED
