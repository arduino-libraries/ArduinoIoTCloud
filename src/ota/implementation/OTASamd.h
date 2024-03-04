#pragma once

#include "src/ota/interface/OTAInterface.h"
#include <Arduino_DebugUtils.h>

class SAMDOTACloudProcess: public OTACloudProcessInterface {
public:
  STM32H7OTACloudProcess();
protected:
  // we start the download and decompress process
  virtual State fetch(Message* msg=nullptr);

  // when the download is completed we verify for integrity and correctness of the downloaded binary
  // virtual State verifyOTA(Message* msg=nullptr); // TODO this may be performed inside download

  // whene the download is correctly finished we set the mcu to use the newly downloaded binary
  virtual State flashOTA(Message* msg=nullptr);

  // we reboot the device
  virtual State reboot(Message* msg=nullptr);
};
