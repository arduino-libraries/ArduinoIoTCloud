/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#ifdef ARDUINO_ARCH_SAMD
#define OFFLOADED_DOWNLOAD

#elif defined(ARDUINO_NANO_RP2040_CONNECT)

#elif defined(BOARD_STM32H7)

#if defined(ARDUINO_PORTENTA_H7_M7)
  #define ARDUINO_PORTENTA_OTA_MAGIC 0x2341025b
  #define ARDUINO_PORTENTA_OTA_SDMMC_SUPPORT
  #define ARDUINO_PORTENTA_OTA_QSPI_SUPPORT
#endif

#if defined(ARDUINO_NICLA_VISION)
  #define ARDUINO_PORTENTA_OTA_MAGIC 0x2341025f
  #define ARDUINO_PORTENTA_OTA_QSPI_SUPPORT
#endif

#if defined(ARDUINO_OPTA)
  #define ARDUINO_PORTENTA_OTA_MAGIC 0x23410064
  #define ARDUINO_PORTENTA_OTA_QSPI_SUPPORT
#endif

#if defined(ARDUINO_GIGA)
  #define ARDUINO_PORTENTA_OTA_MAGIC 0x23410266
  #define ARDUINO_PORTENTA_OTA_QSPI_SUPPORT
#endif


#elif defined(ARDUINO_ARCH_ESP32)
#define OTA_BASIC_AUTH

#elif defined(ARDUINO_UNOR4_WIFI)
#define OFFLOADED_DOWNLOAD
#endif
