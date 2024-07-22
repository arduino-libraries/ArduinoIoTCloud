/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once
#include "AIoTC_Config.h"
#if OTA_ENABLED

#include "OTAConfig.h"
#ifdef ARDUINO_ARCH_SAMD

#include "implementation/OTASamd.h"
using ArduinoCloudOTA = SAMDOTACloudProcess;

// TODO Check if a macro already exist
// constexpr uint32_t OtaMagicNumber = 0x23418054; // MKR_WIFI_1010
constexpr uint32_t OtaMagicNumber = 0x23418057; // NANO_33_IOT

#elif defined(ARDUINO_NANO_RP2040_CONNECT)
#include "implementation/OTANanoRP2040.h"
using ArduinoCloudOTA = NANO_RP2040OTACloudProcess;

// TODO Check if a macro already exist
constexpr uint32_t OtaMagicNumber = 0x2341005E; // TODO check this value is correct

#elif defined(BOARD_STM32H7)
#include "implementation/OTASTM32H7.h"
using ArduinoCloudOTA = STM32H7OTACloudProcess;

constexpr uint32_t OtaMagicNumber = ARDUINO_PORTENTA_OTA_MAGIC;

#elif defined(ARDUINO_ARCH_ESP32)
#include "implementation/OTAEsp32.h"
using ArduinoCloudOTA = ESP32OTACloudProcess;

#if defined (ARDUINO_NANO_ESP32)
  constexpr uint32_t OtaMagicNumber = 0x23410070;
#else
  constexpr uint32_t OtaMagicNumber = 0x45535033;
#endif

#elif defined(ARDUINO_UNOR4_WIFI)

#include "implementation/OTAUnoR4.h"
using ArduinoCloudOTA = UNOR4OTACloudProcess;

constexpr uint32_t OtaMagicNumber = 0x23411002;

#else
#error "This Board doesn't support OTA"
#endif

#endif // OTA_ENABLED
