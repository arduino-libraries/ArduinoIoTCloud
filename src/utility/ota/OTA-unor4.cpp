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

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <AIoTC_Config.h>

#if defined ARDUINO_UNOR4_WIFI && OTA_ENABLED

#include "OTAUpdate.h"
#include <Arduino_DebugUtils.h>
#include "tls/utility/SHA256.h"
#include "fsp_common_api.h"
#include "r_flash_lp.h"
#include "WiFiS3.h"

/******************************************************************************
 * DEFINES
 ******************************************************************************/

/* Key code for writing PRCR register. */
#define BSP_PRV_PRCR_KEY                (0xA500U)
#define BSP_PRV_PRCR_PRC1_UNLOCK        ((BSP_PRV_PRCR_KEY) | 0x2U)
#define BSP_PRV_PRCR_LOCK               ((BSP_PRV_PRCR_KEY) | 0x0U)

#define OTA_MAGIC (*((volatile uint16_t *) &R_SYSTEM->VBTBKR[4]))
#define OTA_SIZE  (*((volatile uint32_t *) &R_SYSTEM->VBTBKR[6]))

/******************************************************************************
 * FUNCTION DEFINITION
 ******************************************************************************/

static void unor4_setOTASize(uint32_t size)
{
  R_SYSTEM->PRCR = (uint16_t) BSP_PRV_PRCR_PRC1_UNLOCK;
  OTA_MAGIC = 0x07AA;
  OTA_SIZE = size;
  R_SYSTEM->PRCR = (uint16_t) BSP_PRV_PRCR_LOCK;
}

static uint32_t unor4_getOTASize()
{
  if (OTA_MAGIC == 0x07AA)
  {
    return OTA_SIZE;
  }
  return 0;
}

static int unor4_codeFlashOpen(flash_lp_instance_ctrl_t * ctrl)
{
  flash_cfg_t                  cfg;

  cfg.data_flash_bgo         = false;
  cfg.p_callback             = nullptr;
  cfg.p_context              = nullptr;
  cfg.p_extend               = nullptr;
  cfg.ipl                    = (BSP_IRQ_DISABLED);
  cfg.irq                    = FSP_INVALID_VECTOR;
  cfg.err_ipl                = (BSP_IRQ_DISABLED);
  cfg.err_irq                = FSP_INVALID_VECTOR;

  fsp_err_t rv = FSP_ERR_UNSUPPORTED;

  rv = R_FLASH_LP_Open(ctrl,&cfg);
  return rv;
}

static int unor4_codeFlashClose(flash_lp_instance_ctrl_t * ctrl)
{
  fsp_err_t rv = FSP_ERR_UNSUPPORTED;

  rv = R_FLASH_LP_Close(ctrl);
  return rv;
}

int unor4_onOTARequest(char const * ota_url)
{
  int ota_err = static_cast<int>(OTAUpdate::Error::None);
  OTAUpdate ota;

  /* Initialize the board for OTA handling. */
  if ((ota_err = static_cast<int>(ota.begin("/update.bin"))) != static_cast<int>(OTAUpdate::Error::None))
  {
    DEBUG_ERROR("OTAUpdate::begin() failed with %d", ota_err);
    return ota_err;
  }

  /* Download the OTA file from the web storage location. */
  int const ota_download = ota.download(ota_url,"/update.bin");
  if (ota_download <= 0)
  {
    DEBUG_ERROR("OTAUpdate::download() failed with %d", ota_download);
    return ota_download;
  }
  DEBUG_VERBOSE("OTAUpdate::download() %d bytes downloaded", ota_download);

  /* Verify update integrity */
  if ((ota_err = static_cast<int>(ota.verify())) != static_cast<int>(OTAUpdate::Error::None))
  {
    DEBUG_ERROR("OTAUpdate::verify() failed with %d", ota_err);
    return ota_err;
  }

  /* Store update size and write OTA magin number */
  unor4_setOTASize(ota_download);

  /* Flash new firmware */
  if ((ota_err = static_cast<int>(ota.update("/update.bin"))) != static_cast<int>(OTAUpdate::Error::None))
  {
    DEBUG_ERROR("OTAUpdate::update() failed with %d", ota_err);
    return ota_err;
  }

  return static_cast<int>(OTAUpdate::Error::None);
}

String unor4_getOTAImageSHA256()
{
  /* The length of the application can be retrieved the same way it was
   * communicated to the bootloader, that is by writing to the non-volatile
   * storage registers of the RTC.
   */
  SHA256         sha256;
  uint32_t const app_start = 0x4000;
  uint32_t const app_size  = unor4_getOTASize();

  flash_lp_instance_ctrl_t ctrl;
  unor4_codeFlashOpen(&ctrl);

  sha256.begin();
  uint32_t b = 0;
  uint32_t bytes_read = 0; for(uint32_t a = app_start;
                               bytes_read < app_size;
                               bytes_read += sizeof(b), a += sizeof(b))
  {
    /* Read the next chunk of memory. */
    memcpy(&b, reinterpret_cast<const void *>(a), sizeof(b));
    /* Feed it to SHA256. */
    sha256.update(reinterpret_cast<uint8_t *>(&b), sizeof(b));
  }

  unor4_codeFlashClose(&ctrl);

  /* Retrieve the final hash string. */
  uint8_t sha256_hash[SHA256::HASH_SIZE] = {0};
  sha256.finalize(sha256_hash);
  String sha256_str;
  std::for_each(sha256_hash,
                sha256_hash + SHA256::HASH_SIZE,
                [&sha256_str](uint8_t const elem)
                {
                  char buf[4];
                  snprintf(buf, 4, "%02X", elem);
                  sha256_str += buf;
                });
  DEBUG_ERROR("SHA256: %d bytes (of %d) read", bytes_read, app_size);
  return sha256_str;
}

bool unor4_isOTACapable()
{
  /* check firmware version */
  String const fv = WiFi.firmwareVersion();
  if (fv < String("0.3.0")) {
    return false;
  }
  return true;
}

#endif /* ARDUINO_UNOR4_WIFI */
