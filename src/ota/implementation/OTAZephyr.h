/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2026 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include "ota/interface/OTAInterfaceDefault.h"

#include <zephyr/fs/fs.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr_sketch_header.h>

union sketch_header {
	struct {
		uint8_t _padding[SKETCH_HEADER_LEN - sizeof(struct sketch_header_v1) - 1];
		struct sketch_header_v1 hdr;
	};
	uint8_t buf[SKETCH_HEADER_LEN];
};

class ZephyrOTACloudProcess: public OTADefaultCloudProcessInterface {
public:
  ZephyrOTACloudProcess(MessageStream *ms, Client* client=nullptr);
  ~ZephyrOTACloudProcess();

  bool isOtaCapable() override;

protected:
  OTACloudProcessInterface::State resume(Message* msg=nullptr) override;
  OTACloudProcessInterface::State startOTA() override;
  OTACloudProcessInterface::State flashOTA() override;
  OTACloudProcessInterface::State reboot() override;
  int writeFlash(uint8_t* const buffer, size_t len) override;
  void reset() override;

  void* appStartAddress() override;
  uint32_t appSize() override;
  bool appFlashOpen() override;
  bool appFlashClose() override;

  void calculateSHA256(SHA256& sha256_calc) override;

private:
  int closeUpdateFile();

  struct fs_file_t _file;
  const struct flash_area *_app_fa;
  const struct flash_area *_loader_fa;

  sketch_header _sketch_header;
};
