/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once
#include "ota/interface/OTAInterfaceDefault.h"

#include <QSPIFBlockDevice.h>

#include <BlockDevice.h>
#include <MBRBlockDevice.h>
#include <FATFileSystem.h>
#include <LittleFileSystem.h>

#include "WiFi.h" /* WiFi from ArduinoCore-mbed */
#include <SocketHelpers.h>

#define APOTA_QSPI_FLASH_FLAG         (1 << 2)
#define APOTA_SDCARD_FLAG             (1 << 3)
#define APOTA_RAW_FLAG                (1 << 4)
#define APOTA_FATFS_FLAG              (1 << 5)
#define APOTA_LITTLEFS_FLAG           (1 << 6)
#define APOTA_MBR_FLAG                (1 << 7)

namespace portenta {
  enum StorageType {
      QSPI_FLASH_FATFS        = APOTA_QSPI_FLASH_FLAG | APOTA_FATFS_FLAG,
      QSPI_FLASH_FATFS_MBR    = APOTA_QSPI_FLASH_FLAG | APOTA_FATFS_FLAG | APOTA_MBR_FLAG,
      SD_FATFS                = APOTA_SDCARD_FLAG | APOTA_FATFS_FLAG,
      SD_FATFS_MBR            = APOTA_SDCARD_FLAG | APOTA_FATFS_FLAG | APOTA_MBR_FLAG,
  };
}

class STM32H7OTACloudProcess: public OTADefaultCloudProcessInterface {
public:
  STM32H7OTACloudProcess(MessageStream *ms, Client* client=nullptr);
  ~STM32H7OTACloudProcess();
  void update() override;

  virtual bool isOtaCapable() override;
protected:
  virtual OTACloudProcessInterface::State resume(Message* msg=nullptr) override;

  // we are overriding the method of startOTA in order to open the destination file for the ota download
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
  bool storageInit();
  bool storageOpen();

  void storageClean();

  FILE* decompressed;
  // static const char UPDATE_FILE_NAME[];
  mbed::BlockDevice* _bd_raw_qspi;
  uint32_t _program_length;

  mbed::BlockDevice* _bd;
  mbed::FATFileSystem* _fs;

  mbed::MBRBlockDevice* cert_bd_qspi;
  mbed::FATFileSystem*  cert_fs_qspi;

  const portenta::StorageType storage=portenta::QSPI_FLASH_FATFS_MBR;
  const uint32_t data_offset=2;

  static const char UPDATE_FILE_NAME[];
};
