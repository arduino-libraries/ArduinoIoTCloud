#if defined(BOARD_STM32H7) && OTA_ENABLED
#include "OTAPortentaH7.h"

STM32H7OTACloudProcess::STM32H7OTACloudProcess() {}

void STM32H7OTACloudProcess::update() {
  OTACloudProcessInterface::update();
  watchdog_reset();
}

State STM32H7OTACloudProcess::fetch(Message *msg) {
  
}

State STM32H7OTACloudProcess::flashOTA(Message *msg) {
  /* Schedule the firmware update. */
  if((ota_portenta_err = ota_portenta_qspi.update()) != Arduino_Portenta_OTA::Error::None) {
    DEBUG_ERROR("Arduino_Portenta_OTA_QSPI::update() failed with %d", static_cast<int>(ota_portenta_err));
    return static_cast<int>(ota_portenta_err);
  }
}

State STM32H7OTACloudProcess::reboot(Message *msg) {
  // TODO save information about the progress reached in the ota

  // This command reboots the mcu
  NVIC_SystemReset();

  return Resume; // This won't ever be reached
}

#endif // defined(BOARD_STM32H7) && OTA_ENABLED