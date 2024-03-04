#pragma once

#include "src/ota/interface/OTAInterface.h"

class STM32H7OTACloudProcess: public OTACloudProcessInterface {
public:
  STM32H7OTACloudProcess();
  void update();

  // retrocompatibility functions used in old ota prtotocol based on properties
  int otaRequest(char const * ota_url, NetworkAdapter iface);
  String getOTAImageSHA256();
  bool isOTACapable();
protected:
  // we start the download and decompress process
  virtual State fetch(Message* msg=nullptr);

  // whene the download is correctly finished we set the mcu to use the newly downloaded binary
  virtual State flashOTA(Message* msg=nullptr);

  // we reboot the device
  virtual State reboot(Message* msg=nullptr);
};
