/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include "ota/interface/OTAInterface.h"
#include "OTAUpdate.h"
#include "r_flash_lp.h"

class UNOR4OTACloudProcess: public OTACloudProcessInterface {
public:
  UNOR4OTACloudProcess(MessageStream *ms);

  bool isOtaCapable() override;
protected:
  virtual OTACloudProcessInterface::State resume(Message* msg=nullptr) override;

  // we are overriding the method of startOTA in order to download ota file on ESP32
  virtual OTACloudProcessInterface::State startOTA() override;

  // we start the download and decompress process
  virtual OTACloudProcessInterface::State fetch() override;

  // whene the download is correctly finished we set the mcu to use the newly downloaded binary
  virtual OTACloudProcessInterface::State flashOTA() override;

  // we reboot the device
  virtual OTACloudProcessInterface::State reboot() override;

  virtual void reset() override;

  constexpr void* appStartAddress();
  uint32_t appSize();

  bool appFlashOpen();
  bool appFlashClose();

public:
  // used to access to flash memory for sha256 calculation
  flash_lp_instance_ctrl_t  ctrl;
  flash_cfg_t               cfg;

  OTAUpdate ota;
  static const char UPDATE_FILE_NAME[];
};
