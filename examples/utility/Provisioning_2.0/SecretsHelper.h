/*
  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include <Arduino.h>
#include <Arduino_UniqueHWId.h>

inline String GetUHWID() {
  UniqueHWId Id;
  if (Id.begin()) {
    return Id.get();
  }
  return "";
}
