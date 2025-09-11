/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2025 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/******************************************************************************
  INCLUDE
 ******************************************************************************/

#include <AIoTC_Config.h>
#ifdef BOARD_HAS_ECCX08

#include "bearssl/inner.h"

/* see bearssl_ssl.h */
void aiotc_client_profile_init(br_ssl_client_context *cc, br_x509_minimal_context *xc, const br_x509_trust_anchor *trust_anchors, size_t trust_anchors_num)
{
  /*
   * The "full" profile supports all implemented cipher suites.
   *
   * Rationale for suite order, from most important to least
   * important rule:
   *
   * -- Don't use 3DES if AES or ChaCha20 is available.
   * -- Try to have Forward Secrecy (ECDHE suite) if possible.
   * -- When not using Forward Secrecy, ECDH key exchange is
   *    better than RSA key exchange (slightly more expensive on the
   *    client, but much cheaper on the server, and it implies smaller
   *    messages).
   * -- ChaCha20+Poly1305 is better than AES/GCM (faster, smaller code).
   * -- GCM is better than CBC.
   * -- AES-128 is preferred over AES-256 (AES-128 is already
   *    strong enough, and AES-256 is 40% more expensive).
   */
  static const uint16_t suites[] = {
    BR_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256
  };

  /*
   * Reset client context and set supported versions from TLS-1.0
   * to TLS-1.2 (inclusive).
   */
  br_ssl_client_zero(cc);
  br_ssl_engine_set_versions(&cc->eng, BR_TLS12, BR_TLS12);

  /*
   * X.509 engine uses SHA-256 to hash certificate DN (for
   * comparisons).
   */
  br_x509_minimal_init(xc, &br_sha256_vtable, trust_anchors, trust_anchors_num);

  /*
   * Set suites and asymmetric crypto implementations. We use the
   * "i31" code for RSA (it is somewhat faster than the "i32"
   * implementation).
   * TODO: change that when better implementations are made available.
   */
  br_ssl_engine_set_suites(&cc->eng, suites, (sizeof suites) / (sizeof suites[0]));
  br_ssl_engine_set_default_ecdsa(&cc->eng);
  br_x509_minimal_set_ecdsa(xc, br_ssl_engine_get_ec(&cc->eng), br_ssl_engine_get_ecdsa(&cc->eng));

  /*
   * Set supported hash functions, for the SSL engine and for the
   * X.509 engine.
   */
  br_ssl_engine_set_hash(&cc->eng, br_sha256_ID, &br_sha256_vtable);
  br_x509_minimal_set_hash(xc, br_sha256_ID, &br_sha256_vtable);

  /*
   * Link the X.509 engine in the SSL engine.
   */
  br_ssl_engine_set_x509(&cc->eng, &xc->vtable);

  /*
   * Set the PRF implementations.
   */
  br_ssl_engine_set_prf_sha256(&cc->eng, &br_tls12_sha256_prf);

  /*
   * Symmetric encryption. We use the "default" implementations
   * (fastest among constant-time implementations).
   */
  br_ssl_engine_set_default_aes_gcm(&cc->eng);
}

#endif /* #ifdef BOARD_HAS_ECCX08 */
