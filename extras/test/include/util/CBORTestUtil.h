/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2019 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef INCLUDE_CBOR_TESTUTIL_H_
#define INCLUDE_CBOR_TESTUTIL_H_

/******************************************************************************
  INCLUDE
 ******************************************************************************/

#include <PropertyContainer.h>

#include <vector>

/******************************************************************************
  NAMESPACE
 ******************************************************************************/

namespace cbor
{

/******************************************************************************
  PROTOTYPES
 ******************************************************************************/

std::vector<uint8_t> encode(PropertyContainer & property_container, bool lightPayload = false);
void print(std::vector<uint8_t> const & vect);

} /* cbor */

#endif /* INCLUDE_CBOR_TESTUTIL_H_ */
