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

namespace STM32H747OTA {
  /* External QSPI flash + MBR + FatFs */
  static const uint32_t constexpr STORAGE_TYPE = ((1 << 2) | (1 << 5) | (1 << 7));
  /* Default OTA partition */
  static const uint32_t constexpr PARTITION = 2;
  /* OTA Magic number */
  static const uint32_t constexpr MAGIC = 0x07AA;
  /* OTA download folder name */
  static const char constexpr FOLDER[] = "ota";
  /* OTA update filename */
  static const char constexpr NAME[] = "UPDATE.BIN";
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
  bool findProgramLength(uint32_t & program_length);
  void storageClean();

  FILE* decompressed;
  mbed::BlockDevice* _bd_raw_qspi;

  mbed::BlockDevice* _bd;
  mbed::FATFileSystem* _fs;

  String _filename;
};
