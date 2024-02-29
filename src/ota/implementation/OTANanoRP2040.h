/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include "ota/interface/OTAInterfaceDefault.h"

#include "FATFileSystem.h"
#include "FlashIAPBlockDevice.h"

class NANO_RP2040OTACloudProcess: public OTADefaultCloudProcessInterface {
public:
  NANO_RP2040OTACloudProcess(MessageStream *ms, Client* client=nullptr);
  ~NANO_RP2040OTACloudProcess();

  virtual bool isOtaCapable() override;
protected:
  virtual OTACloudProcessInterface::State resume(Message* msg=nullptr) override;

  virtual OTACloudProcessInterface::State startOTA() override;

  // whene the download is correctly finished we set the mcu to use the newly downloaded binary
  virtual OTACloudProcessInterface::State flashOTA() override;

  // we reboot the device
  virtual OTACloudProcessInterface::State reboot() override;

  // write the decompressed char buffer of the incoming ota
  virtual int writeFlash(uint8_t* const buffer, size_t len) override;

  virtual void reset() override;

  void* appStartAddress();
  uint32_t appSize();
  bool appFlashOpen() { return true; };
  bool appFlashClose() { return true; };
private:
  FlashIAPBlockDevice flash;
  FILE* decompressed;
  mbed::FATFileSystem* fs;
  static const char UPDATE_FILE_NAME[];

  int close_fs();
};
