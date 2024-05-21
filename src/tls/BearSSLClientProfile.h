/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _BEAR_SSL_CLIENT_PROFILE_H_
#define _BEAR_SSL_CLIENT_PROFILE_H_

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <AIoTC_Config.h>
#ifdef BOARD_HAS_ECCX08

extern "C" void aiotc_client_profile_init(br_ssl_client_context *cc, br_x509_minimal_context *xc, const br_x509_trust_anchor *trust_anchors, size_t trust_anchors_num);

#endif /* #ifdef BOARD_HAS_ECCX08 */

#endif /* _BEAR_SSL_CLIENT_PROFILE_H_ */

