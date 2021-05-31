/*
   This file is part of ArduinoIoTCloud.

   Copyright 2020 ARDUINO SA (http://www.arduino.cc/)

   This software is released under the GNU General Public License version 3,
   which covers the main part of arduino-cli.
   The terms of this license can be found at:
   https://www.gnu.org/licenses/gpl-3.0.en.html

   You can be released from the requirements of the above licenses by purchasing
   a commercial license. Buying such a license is mandatory if you want to modify or
   otherwise use the software for commercial activities involving the Arduino
   software without disclosing the source code of your own applications. To purchase
   a commercial license, send an email to license@arduino.cc.
*/

#if defined(ARDUINO_NANO_RP2040_CONNECT)

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include "OTA.h"

#include "../watchdog/Watchdog.h"

#include <Arduino_DebugUtils.h>

/******************************************************************************
 * FUNCTION DEFINITION
 ******************************************************************************/

/* Original code: http://stackoverflow.com/questions/2616011/easy-way-to-parse-a-url-in-c-cross-platform */
#include <string>
#include <algorithm>
#include <cctype>
#include <functional>
#include <iostream>

struct URI {
  public:
    URI(const std::string& url_s) {
      this->parse(url_s);
    }
    std::string protocol_, host_, path_, query_;
  private:
    void parse(const std::string& url_s);
};

using namespace std;

// ctors, copy, equality, ...
// TODO: change me into something embedded friendly (this function adds ~100KB to flash)
void URI::parse(const string& url_s)
{
  const string prot_end("://");
  string::const_iterator prot_i = search(url_s.begin(), url_s.end(),
      prot_end.begin(), prot_end.end());
  protocol_.reserve(distance(url_s.begin(), prot_i));
  transform(url_s.begin(), prot_i,
      back_inserter(protocol_),
      ptr_fun<int,int>(tolower)); // protocol is icase
  if( prot_i == url_s.end() )
    return;
  advance(prot_i, prot_end.length());
  string::const_iterator path_i = find(prot_i, url_s.end(), '/');
  host_.reserve(distance(prot_i, path_i));
  transform(prot_i, path_i,
      back_inserter(host_),
      ptr_fun<int,int>(tolower)); // host is icase
  string::const_iterator query_i = find(path_i, url_s.end(), '?');
  path_.assign(path_i, query_i);
  if( query_i != url_s.end() )
    ++query_i;
  query_.assign(query_i, url_s.end());
}

int rp2040_connect_onOTARequest(char const * ota_url)
{
  SFU::begin();

   mbed_watchdog_reset();

#if 0
  /* Just to be safe delete any remains from previous updates. */
  struct stat st;
  int err = stat("/ota/UPDATE.BIN", &st);
  if (err == 0) {
    remove("/ota/UPDATE.BIN");
  }
#endif

  URI url(ota_url);
  Client * client = nullptr;
  int port = 0;

  if (url.protocol_ == "http") {
    client = new WiFiClient();
    port = 80;
  } else if (url.protocol_ == "https") {
    client = new WiFiSSLClient();
    port = 443;
  } else {
    DEBUG_ERROR("%s: Failed to parse OTA URL %s", __FUNCTION__, ota_url);
    return static_cast<int>(OTAError::RP2040_UrlParseError);
  }

  const char* host = url.host_.c_str();

  mbed_watchdog_reset();

  int ret = client->connect(host, port);
  if (!ret)
  {
    DEBUG_ERROR("%s: Connection failure with OTA storage server %s", __FUNCTION__, host);
    return static_cast<int>(OTAError::RP2040_ServerConnectError);
  }

  mbed_watchdog_reset();

  client->println(String("GET ") + url.path_.c_str() + " HTTP/1.1");
  client->println(String("Host: ") + host);
  client->println("Connection: close");
  client->println();

  mbed_watchdog_reset();

  FILE * file = fopen("/ota/UPDATE.BIN", "wb");
  if (!file)
  {
    DEBUG_ERROR("%s: fopen() failed", __FUNCTION__);
    fclose(file);
    return static_cast<int>(OTAError::RP2040_ErrorOpenUpdateFile);
  }

  String http_header;
  bool is_header_complete = false;

  while (!client->available())
  {
    delay(10);
    mbed_watchdog_reset();
  }

  while (client->available())
  {
    mbed_watchdog_reset();

    char const c = client->read();

    if(!is_header_complete)
    {
      http_header += c;
      if (http_header.endsWith("\r\n\r\n"))
        is_header_complete = true;
    }
    else
    {
      if (fwrite(&c, 1, sizeof(c), file) != sizeof(c))
      {
        DEBUG_ERROR("%s: Writing of firmware image to flash failed", __FUNCTION__);
        return static_cast<int>(OTAError::RP2040_ErrorWriteUpdateFile);
      }
    }
  }

  int const file_len = ftell(file);
  DEBUG_DEBUG("%s: %d bytes received", __FUNCTION__, file_len);

  mbed_watchdog_reset();

  while (file_len == 0) {
      delay(1000);
  }

  fclose(file);

  /* Perform the reset to reboot to SxU. */
  NVIC_SystemReset();

  return static_cast<int>(OTAError::None);
}

#endif /* ARDUINO_NANO_RP2040_CONNECT */
