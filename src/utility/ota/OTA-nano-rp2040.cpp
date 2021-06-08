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

#include <SFU.h>

#include "mbed.h"
#include "FATFileSystem.h"
#include "FlashIAPBlockDevice.h"

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
  watchdog_reset();

  int err = -1;
  FlashIAPBlockDevice flash(XIP_BASE + 0xF00000, 0x100000);
  if ((err = flash.init()) < 0)
  {
    DEBUG_ERROR("%s: flash.init() failed with %d", __FUNCTION__, err);
    return static_cast<int>(OTAError::RP2040_ErrorFlashInit);
  }

  watchdog_reset();

  flash.erase(XIP_BASE + 0xF00000, 0x100000);

  watchdog_reset();

  mbed::FATFileSystem fs("ota");
  if ((err = fs.reformat(&flash)) != 0)
  {
     DEBUG_ERROR("%s: fs.reformat() failed with %d", __FUNCTION__, err);
     return static_cast<int>(OTAError::RP2040_ErrorReformat);
  }

  watchdog_reset();

  FILE * file = fopen("/ota/UPDATE.BIN.LZSS", "wb");
  if (!file)
  {
    DEBUG_ERROR("%s: fopen() failed", __FUNCTION__);
    fclose(file);
    return static_cast<int>(OTAError::RP2040_ErrorOpenUpdateFile);
  }

  watchdog_reset();

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
    fclose(file);
    return static_cast<int>(OTAError::RP2040_UrlParseError);
  }

  watchdog_reset();

  if (!client->connect(url.host_.c_str(), port))
  {
    DEBUG_ERROR("%s: Connection failure with OTA storage server %s", __FUNCTION__, url.host_.c_str());
    fclose(file);
    return static_cast<int>(OTAError::RP2040_ServerConnectError);
  }

  watchdog_reset();

  client->println(String("GET ") + url.path_.c_str() + " HTTP/1.1");
  client->println(String("Host: ") + url.host_.c_str());
  client->println("Connection: close");
  client->println();

  watchdog_reset();

  /* Receive HTTP header. */
  String http_header;
  bool is_header_complete     = false,
       is_http_header_timeout = false;
  for (unsigned long const start = millis(); !is_header_complete;)
  {
    is_http_header_timeout = (millis() - start) > AIOT_CONFIG_RP2040_OTA_HTTP_HEADER_RECEIVE_TIMEOUT_ms;
    if (is_http_header_timeout) break;

    watchdog_reset();

    if (client->available())
    {
      char const c = client->read();

      http_header += c;
      if (http_header.endsWith("\r\n\r\n"))
        is_header_complete = true;
    }
  }

  if (!is_header_complete)
  {
    DEBUG_ERROR("%s: Error receiving HTTP header %s", __FUNCTION__, is_http_header_timeout ? "(timeout)":"");
    fclose(file);
    return static_cast<int>(OTAError::RP2040_HttpHeaderError);
  }

  /* Extract concent length from HTTP header. A typical entry looks like
   *   "Content-Length: 123456"
   */
  char const * content_length_ptr = strstr(http_header.c_str(), "Content-Length");
  if (!content_length_ptr)
  {
    DEBUG_ERROR("%s: Failure to extract content length from http header", __FUNCTION__);
    fclose(file);
    return static_cast<int>(OTAError::RP2040_ErrorParseHttpHeader);
  }
  /* Find start of numerical value. */
  char * ptr = const_cast<char *>(content_length_ptr);
  for (; (*ptr != '\0') && !isDigit(*ptr); ptr++) { }
  /* Extract numerical value. */
  String content_length_str;
  for (; isDigit(*ptr); ptr++) content_length_str += *ptr;
  int const content_length_val = atoi(content_length_str.c_str());
  DEBUG_VERBOSE("%s: Length of OTA binary according to HTTP header = %d bytes", __FUNCTION__, content_length_val);

  /* Receive as many bytes as are indicated by the HTTP header - or die trying. */
  int  bytes_received = 0;
  bool is_http_data_timeout = false;
  for(unsigned long const start = millis(); bytes_received < content_length_val;)
  {
    is_http_data_timeout = (millis() - start) > AIOT_CONFIG_RP2040_OTA_HTTP_DATA_RECEIVE_TIMEOUT_ms;
    if (is_http_data_timeout) break;

    watchdog_reset();

    if (client->available())
    {
      char const c = client->read();

      if (fwrite(&c, 1, sizeof(c), file) != sizeof(c))
      {
        DEBUG_ERROR("%s: Writing of firmware image to flash failed", __FUNCTION__);
        fclose(file);
        return static_cast<int>(OTAError::RP2040_ErrorWriteUpdateFile);
      }

      bytes_received++;
    }
  }

  if (bytes_received != content_length_val) {
    DEBUG_ERROR("%s: Error receiving HTTP data %s (%d bytes received, %d expected)", __FUNCTION__, is_http_data_timeout ? "(timeout)":"", bytes_received, content_length_val);
    fclose(file);
    return static_cast<int>(OTAError::RP2040_HttpDataError);
  }

  DEBUG_INFO("%s: %d bytes received", __FUNCTION__, ftell(file));
  fclose(file);

  /* Unmount the filesystem. */
  if ((err = fs.unmount()) != 0)
  {
     DEBUG_ERROR("%s: fs.unmount() failed with %d", __FUNCTION__, err);
     return static_cast<int>(OTAError::RP2040_ErrorUnmount);
  }

  /* Perform the reset to reboot to SFU. */
  mbed_watchdog_trigger_reset();
  /* If watchdog is enabled we should not reach this point */
  NVIC_SystemReset();

  return static_cast<int>(OTAError::None);
}

#endif /* ARDUINO_NANO_RP2040_CONNECT */
