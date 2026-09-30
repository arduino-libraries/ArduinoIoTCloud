/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2026 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "AIoTC_Config.h"

#if defined(ARDUINO_ARCH_ZEPHYR) && OTA_ENABLED

#include "OTAZephyr.h"

#include <zephyr/devicetree.h>
#include <zephyr/devicetree/fixed-partitions.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/drivers/flash.h>
#include <ota_api.h>

/* The loader (loader/main.c try_ota_update()) installs whatever is at
 * /ota:/UPDATE.BIN on the next boot, after re-checking the inner sketch
 * header for magic + bounds. All payload validation (outer CRC32 over
 * the .ota body, optional LZSS decompression) is done here in the
 * sketch by OTADefaultCloudProcessInterface — the loader trusts the
 * file produced by writeFlash().
 */

#define LOADER_OTA

#ifdef LOADER_OTA
#define OTA_FILE OTA_LOADER_FILENAME

#define DT_LOADER_PARTITION DT_NODE_BY_PARTITION_LABEL(image_0)
#else
#define SKETCH_OTA
#define OTA_FILE OTA_SKETCH_FILENAME
#endif

#define DT_SKETCH_PARTITION DT_NODE_BY_PARTITION_LABEL(user_sketch)

ZephyrOTACloudProcess::ZephyrOTACloudProcess(MessageStream *ms, Client* client)
: OTADefaultCloudProcessInterface(ms, client)
, _app_fa(nullptr)
#ifdef LOADER_OTA
, _loader_fa(nullptr)
#endif
{
  fs_file_t_init(&_file);
}

ZephyrOTACloudProcess::~ZephyrOTACloudProcess() {
  closeUpdateFile();
}

OTACloudProcessInterface::State ZephyrOTACloudProcess::resume(Message* /*msg*/) {
  /* The loader unlinks UPDATE.BIN on a successful install, so there is
   * no post-OTA state to recover from on this side. */
  return OtaBegin;
}

bool ZephyrOTACloudProcess::isOtaCapable() {
  struct fs_statvfs stat;

  // FIXME replace /ota with a CONFIG Variable
  return fs_statvfs("/ota:", &stat) == 0;
}

OTACloudProcessInterface::State ZephyrOTACloudProcess::startOTA() {
  if (!isOtaCapable()) {
    return NoOtaStorageFail;
  }

  /* Clear any leftover update file from a previous aborted attempt. */
  fs_unlink(OTA_FILE); // TODO check if file doesn't exist before

  fs_file_t_init(&_file);
  int rc = fs_open(&_file, OTA_FILE, FS_O_CREATE | FS_O_WRITE);
  if (rc < 0) {
    return ErrorOpenUpdateFileFail;
  }

  return OTADefaultCloudProcessInterface::startOTA();
}

int ZephyrOTACloudProcess::writeFlash(uint8_t* const buffer, size_t len) {
  if (_file.mp == NULL) {
    return -1;
  }
  ssize_t n = fs_write(&_file, buffer, len);
  return (n < 0) ? -1 : (int)n;
}

OTACloudProcessInterface::State ZephyrOTACloudProcess::flashOTA() {
  /* Close + sync the update file so the loader sees a complete payload
   * on the next boot. The loader-side install is gated only by the
   * file's presence, so the close must succeed before we reboot. */
  if (closeUpdateFile() != 0) {
    return OtaStorageEndFail;
  }

#ifdef LOADER_OTA
  ota_loader_ready();
#else
  ota_sketch_ready();
#endif

  return Reboot;
}

OTACloudProcessInterface::State ZephyrOTACloudProcess::reboot() {
#ifdef LOADER_OTA
  ota_loader_start();
#else
  ota_sketch_start();
#endif

  return Resume; /* unreachable */
}

void ZephyrOTACloudProcess::reset() {
  OTADefaultCloudProcessInterface::reset();
  closeUpdateFile();
  fs_unlink(OTA_FILE);
}

int ZephyrOTACloudProcess::closeUpdateFile() {
  if (_file.mp == NULL) {
    return 0;
  }
  fs_sync(&_file);
  return fs_close(&_file);
}

/* SHA256 of the running sketch image. The sketch lives in the
 * user_sketch flash partition; stream it through SHA256 using
 * flash_area_read so we work whether or not the partition is XIP. */
bool ZephyrOTACloudProcess::appFlashOpen() {
  if (_app_fa != nullptr) {
    return true;
  }

  int rc = flash_area_open(DT_PARTITION_ID(DT_SKETCH_PARTITION), &_app_fa);
  if(rc != 0) {
    DEBUG_ERROR("failed to open flash: %d", rc);
    return false;
  }

#ifdef LOADER_OTA
  rc = flash_area_open(DT_PARTITION_ID(DT_LOADER_PARTITION), &_loader_fa);
  if(rc != 0) {
    DEBUG_ERROR("failed to open flash: %d", rc);
    return false;
  }
#endif

  rc = flash_area_read(_app_fa, 0, _sketch_header.buf, sizeof(_sketch_header));

  DEBUG_VERBOSE("Header ver: %02x, len: %d, magic: %04x, flags: %02x",
    _sketch_header.hdr.ver, _sketch_header.hdr.len, _sketch_header.hdr.magic, _sketch_header.hdr.flags);
  return rc == 0;
}

bool ZephyrOTACloudProcess::appFlashClose() {
  if (_app_fa != nullptr) {
    flash_area_close(_app_fa);
    _app_fa = nullptr;
  }
#ifdef LOADER_OTA
  if (_loader_fa != nullptr) {
    flash_area_close(_loader_fa);
    _loader_fa = nullptr;
  }
#endif
  return true;
}

void* ZephyrOTACloudProcess::appStartAddress() {
  /* Not used directly: calculateSHA256() overrides streaming. */
  return nullptr;
}

uint32_t ZephyrOTACloudProcess::appSize() {
  appFlashOpen();
#ifdef LOADER_OTA
  return _sketch_header.hdr.len + PARTITION_NODE_SIZE(DT_LOADER_PARTITION);
#else
  return _sketch_header.hdr.len;
#endif
}

void ZephyrOTACloudProcess::calculateSHA256(SHA256& sha256_calc) {
  if (!appFlashOpen()) {
    DEBUG_ERROR("failed to open flash for sha256sum");
    return;
  }

  sha256_calc.begin();

  uint8_t chunk[12];
  const size_t total = appSize();
#ifdef LOADER_OTA
  const size_t start_offset = PARTITION_NODE_OFFSET(DT_LOADER_PARTITION);
  const struct device *const _flash_dev = _loader_fa->fa_dev;
#else
  const size_t start_offset = PARTITION_NODE_OFFSET(DT_SKETCH_PARTITION);
  const struct device *const _flash_dev = _app_fa->fa_dev;
#endif
  size_t offset = 0;
  int rc = 0;

  /* Note: this sha calculation assumes that the loader partition and sketch partition are
   * contiguous and accessible by the same flash
   */
  while (offset < total) {
    size_t want = (total - offset) < sizeof(chunk) ? (total - offset) : sizeof(chunk);

    if ((rc=flash_read(_flash_dev, offset + start_offset, chunk, want)) != 0) {
      DEBUG_ERROR("failed to read flash: %d", rc);
      break;
    }
    sha256_calc.update(chunk, want);
    offset += want;
  }

  appFlashClose();
}

#endif /* defined(ARDUINO_ARCH_ZEPHYR) && OTA_ENABLED */
