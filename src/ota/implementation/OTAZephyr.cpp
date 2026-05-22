/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2026 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "AIoTC_Config.h"

#if OTA_STORAGE_ZEPHYR

#include "OTAZephyr.h"

#include <zephyr/devicetree.h>
#include <zephyr/devicetree/fixed-partitions.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/sys/reboot.h>

/* The loader (loader/main.c try_ota_update()) installs whatever is at
 * /ota:/UPDATE.BIN on the next boot, after re-checking the inner sketch
 * header for magic + bounds. All payload validation (outer CRC32 over
 * the .ota body, optional LZSS decompression) is done here in the
 * sketch by OTADefaultCloudProcessInterface — the loader trusts the
 * file produced by writeFlash().
 */
#define OTA_FILE_PATH "/ota:/UPDATE.BIN"

ZephyrOTACloudProcess::ZephyrOTACloudProcess(MessageStream *ms, Client* client)
: OTADefaultCloudProcessInterface(ms, client)
, _file_open(false)
, _app_fa(nullptr) {
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
  return fs_statvfs("/ota:", &stat) == 0;
}

OTACloudProcessInterface::State ZephyrOTACloudProcess::startOTA() {
  if (!isOtaCapable()) {
    return NoOtaStorageFail;
  }

  /* Clear any leftover update file from a previous aborted attempt. */
  fs_unlink(OTA_FILE_PATH);

  fs_file_t_init(&_file);
  int rc = fs_open(&_file, OTA_FILE_PATH, FS_O_CREATE | FS_O_WRITE);
  if (rc < 0) {
    return ErrorOpenUpdateFileFail;
  }
  _file_open = true;

  return OTADefaultCloudProcessInterface::startOTA();
}

int ZephyrOTACloudProcess::writeFlash(uint8_t* const buffer, size_t len) {
  if (!_file_open) {
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
  return Reboot;
}

OTACloudProcessInterface::State ZephyrOTACloudProcess::reboot() {
  sys_reboot(SYS_REBOOT_COLD);
  return Resume; /* unreachable */
}

void ZephyrOTACloudProcess::reset() {
  OTADefaultCloudProcessInterface::reset();
  closeUpdateFile();
  fs_unlink(OTA_FILE_PATH);
}

int ZephyrOTACloudProcess::closeUpdateFile() {
  if (!_file_open) {
    return 0;
  }
  fs_sync(&_file);
  int rc = fs_close(&_file);
  _file_open = false;
  return rc;
}

/* SHA256 of the running sketch image. The sketch lives in the
 * user_sketch flash partition; stream it through SHA256 using
 * flash_area_read so we work whether or not the partition is XIP. */
bool ZephyrOTACloudProcess::appFlashOpen() {
  if (_app_fa != nullptr) {
    return true;
  }
  return flash_area_open(FIXED_PARTITION_ID(user_sketch), &_app_fa) == 0;
}

bool ZephyrOTACloudProcess::appFlashClose() {
  if (_app_fa != nullptr) {
    flash_area_close(_app_fa);
    _app_fa = nullptr;
  }
  return true;
}

void* ZephyrOTACloudProcess::appStartAddress() {
  /* Not used directly: calculateSHA256() overrides streaming. */
  return nullptr;
}

uint32_t ZephyrOTACloudProcess::appSize() {
  return DT_REG_SIZE(DT_NODELABEL(user_sketch));
}

void ZephyrOTACloudProcess::calculateSHA256(SHA256& sha256_calc) {
  if (!appFlashOpen()) {
    return;
  }

  sha256_calc.begin();

  uint8_t chunk[256];
  const size_t total = _app_fa->fa_size;
  size_t offset = 0;
  while (offset < total) {
    size_t want = (total - offset) < sizeof(chunk) ? (total - offset) : sizeof(chunk);
    if (flash_area_read(_app_fa, offset, chunk, want) != 0) {
      break;
    }
    sha256_calc.update(chunk, want);
    offset += want;
  }

  appFlashClose();
}

#endif /* OTA_STORAGE_ZEPHYR */
