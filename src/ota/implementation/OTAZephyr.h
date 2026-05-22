/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2026 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include "ota/interface/OTAInterfaceDefault.h"

#if OTA_STORAGE_ZEPHYR

#include <zephyr/fs/fs.h>
#include <zephyr/storage/flash_map.h>

class ZephyrOTACloudProcess: public OTADefaultCloudProcessInterface {
public:
  ZephyrOTACloudProcess(MessageStream *ms, Client* client=nullptr);
  ~ZephyrOTACloudProcess();

  virtual bool isOtaCapable() override;

protected:
  virtual OTACloudProcessInterface::State resume(Message* msg=nullptr) override;
  virtual OTACloudProcessInterface::State startOTA() override;
  virtual OTACloudProcessInterface::State flashOTA() override;
  virtual OTACloudProcessInterface::State reboot() override;
  virtual int writeFlash(uint8_t* const buffer, size_t len) override;
  virtual void reset() override;

  void* appStartAddress() override;
  uint32_t appSize() override;
  bool appFlashOpen() override;
  bool appFlashClose() override;

  void calculateSHA256(SHA256& sha256_calc) override;

private:
  int closeUpdateFile();

  struct fs_file_t _file;
  bool _file_open;
  const struct flash_area *_app_fa;
};

#endif /* OTA_STORAGE_ZEPHYR */
