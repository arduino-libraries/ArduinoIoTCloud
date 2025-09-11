/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once
#include <math.h>

namespace arduino { namespace math {

    template<typename T>
    inline bool ieee754_different(const T a, const T b, const T delta = 0.0) {
        /* The following comparison is introduced to take into account the very peculiar
         * way of handling NaN values by the standard IEEE754.
         * We consider two floating point number different if their binary representation is different.
         * or if those two IEEE754 values are numbers or zero(which is handled on its own) exceed
         * a certain threshold
         */
        return memcmp((uint8_t*)&a, (uint8_t*)&b, sizeof(b)) != 0 && (
            (std::fpclassify(a) != FP_NORMAL && std::fpclassify(a) != FP_ZERO) ||
            (std::fpclassify(b) != FP_NORMAL && std::fpclassify(b) != FP_ZERO) ||
            fabs(a - b) >= delta
        );
    }
}}
