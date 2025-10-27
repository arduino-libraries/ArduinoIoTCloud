/*
  Copyright (c) 2025 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once
#include <Arduino.h>
#include "opta_info.h"
class OptaFactoryTestClass {
public:
  void begin();
  void optaIDTest();
  bool poll();
private:
  void ledManage();
  void inputManage();
  void rs485Manage();
  void printInfo();
  void printModel();
  static void endCallback();
  static void buttonCallbackRS485();
  void showRS485SuccessResult();
  static void rs485Rcv();
  static inline uint32_t _rs485_pulse = 0;
  bool _all_on = false;
  bool _rs485_test_done = false;
  static inline bool _test_running = false;
  static inline bool _rs485_ok = false;
  static inline bool _showRS485Result = false;
  OptaBoardInfo *_info = nullptr;
  uint32_t _ms10 = 0;
  uint32_t _ms100 = 0;
  uint32_t _nextBoardInfoPrint = 0;
  uint32_t _nextRS485Run = 0;
};

extern OptaFactoryTestClass OptaFactoryTest;
