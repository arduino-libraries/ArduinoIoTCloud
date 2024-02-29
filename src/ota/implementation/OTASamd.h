/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include "ota/interface/OTAInterface.h"
#include <Arduino_DebugUtils.h>

class SAMDOTACloudProcess: public OTACloudProcessInterface {
public:
  SAMDOTACloudProcess(MessageStream *ms);

  virtual bool isOtaCapable() override;
protected:
  virtual OTACloudProcessInterface::State resume(Message* msg=nullptr) override;

  // we are overriding the method of startOTA in order to download ota file on ESP32
  virtual OTACloudProcessInterface::State startOTA() override;

  // we start the download and decompress process
  virtual OTACloudProcessInterface::State fetch() override;

  // whene the download is correctly finished we set the mcu to use the newly downloaded binary
  virtual OTACloudProcessInterface::State flashOTA();

  // we reboot the device
  virtual OTACloudProcessInterface::State reboot();

  virtual void reset() override;

  void* appStartAddress();
  uint32_t appSize();

  bool appFlashOpen()  { return true; }
  bool appFlashClose() { return true; }

private:
  enum class ninaOTAError : int {
    None            = 0,
    Open            = 1,
    Length          = 2,
    CRC             = 3,
    Rename          = 4,
  };
};
