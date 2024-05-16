/*
 * Copyright (c) 2018 Arduino SA. All rights reserved.
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

#ifndef _BEAR_SSL_CLIENT_H_
#define _BEAR_SSL_CLIENT_H_

#include <AIoTC_Config.h>
#ifdef BOARD_HAS_ECCX08

#ifndef BEAR_SSL_CLIENT_OBUF_SIZE
#define BEAR_SSL_CLIENT_OBUF_SIZE 512 + 85
#endif

#ifndef BEAR_SSL_CLIENT_IBUF_SIZE
#define BEAR_SSL_CLIENT_IBUF_SIZE 8192 + 85 + 325 - BEAR_SSL_CLIENT_OBUF_SIZE
#endif

#include <Arduino.h>
#include <Client.h>

#include "bearssl/bearssl.h"

typedef unsigned long(*GetTimeCallbackFunc)();

class BearSSLClient : public Client {

public:

  BearSSLClient(Client* client, const br_x509_trust_anchor* myTAs, int myNumTAs, GetTimeCallbackFunc func);
  BearSSLClient();
  virtual ~BearSSLClient();


  inline void setClient(Client& client) { _client = &client; }
  inline void setProfile(void(*client_init_function)(br_ssl_client_context *cc, br_x509_minimal_context *xc, const br_x509_trust_anchor *trust_anchors, size_t trustrust_anchorst_anchors_num)) { _br_ssl_client_init_function = client_init_function; }
  inline void setTrustAnchors(const br_x509_trust_anchor* myTAs, int myNumTAs) { _TAs = myTAs; _numTAs = myNumTAs; }
  inline void onGetTime(GetTimeCallbackFunc callback) { _get_time_func = callback;}

  virtual int connect(IPAddress ip, uint16_t port);
  virtual int connect(const char* host, uint16_t port);
  virtual size_t write(uint8_t);
  virtual size_t write(const uint8_t *buf, size_t size);
  virtual int available();
  virtual int read();
  virtual int read(uint8_t *buf, size_t size);
  virtual int peek();
  virtual void flush();
  virtual void stop();
  virtual uint8_t connected();
  virtual operator bool();

  using Print::write;

  enum class SNI {
    Insecure
  };

  void setInsecure(SNI insecure) __attribute__((deprecated("INSECURE. DO NOT USE IN PRODUCTION")));

  void setEccSlot(int ecc508KeySlot, const byte cert[], int certLength);
  void setEccSlot(int ecc508KeySlot, const char cert[]);

  int errorCode();

private:
  int connectSSL(const char* host);
  static int clientRead(void *ctx, unsigned char *buf, size_t len);
  static int clientWrite(void *ctx, const unsigned char *buf, size_t len);
  static void clientAppendCert(void *ctx, const void *data, size_t len);

private:
  Client* _client;
  const br_x509_trust_anchor* _TAs;
  int _numTAs;
  bool _noSNI;
  GetTimeCallbackFunc _get_time_func;

  br_ec_private_key _ecKey;
  br_x509_certificate _ecCert;
  bool _ecCertDynamic;

  /* FIXME By introducing _sslio_closing we are overriding the correct behaviour of SSL protocol
   *       where the client is require to correctly close the ssl session. In the way we use it
   *       we are blocking bearssl from sending any data on the underlying level, this fix requires
   *       further investigation in the bearssl code
   */
  bool _sslio_closing;
  br_ssl_client_context _sc;
  br_x509_minimal_context _xc;
  unsigned char _ibuf[BEAR_SSL_CLIENT_IBUF_SIZE];
  unsigned char _obuf[BEAR_SSL_CLIENT_OBUF_SIZE];
  br_sslio_context _ioc;

  void (*_br_ssl_client_init_function)(br_ssl_client_context *cc, br_x509_minimal_context *xc, const br_x509_trust_anchor *trust_anchors, size_t trust_anchors_num);
};

#endif /* #ifdef BOARD_HAS_ECCX08 */

#endif
