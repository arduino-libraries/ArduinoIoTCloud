/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2020 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef _BEAR_SSL_CLIENT_PROFILE_H_
#define _BEAR_SSL_CLIENT_PROFILE_H_

/******************************************************************************
  INCLUDE
 ******************************************************************************/

#include <AIoTC_Config.h>
#ifdef BOARD_HAS_ECCX08

extern "C" void aiotc_client_profile_init(br_ssl_client_context *cc, br_x509_minimal_context *xc, const br_x509_trust_anchor *trust_anchors, size_t trust_anchors_num);

#endif /* #ifdef BOARD_HAS_ECCX08 */

#endif /* _BEAR_SSL_CLIENT_PROFILE_H_ */

