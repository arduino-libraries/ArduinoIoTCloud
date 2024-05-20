/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <AIoTC_Config.h>

#if OTA_ENABLED
#include "OTAInterface.h"
#include "../OTA.h"

extern "C" unsigned long getTime();

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

#ifdef DEBUG_VERBOSE
const char* const OTACloudProcessInterface::STATE_NAMES[] = { // used only for debug purposes
  "Resume",
  "OtaBegin",
  "Idle",
  "OtaAvailable",
  "StartOTA",
  "Fetch",
  "FlashOTA",
  "Reboot",
  "Fail",
  "NoCapableBootloaderFail",
  "NoOtaStorageFail",
  "OtaStorageInitFail",
  "OtaStorageOpenFail",
  "OtaHeaderLengthFail",
  "OtaHeaderCrcFail",
  "OtaHeaderMagicNumberFail",
  "ParseHttpHeaderFail",
  "UrlParseErrorFail",
  "ServerConnectErrorFail",
  "HttpHeaderErrorFail",
  "OtaDownloadFail",
  "OtaHeaderTimeoutFail",
  "HttpResponseFail",
  "OtaStorageEndFail",
  "StorageConfigFail",
  "LibraryFail",
  "ModemFail",
  "ErrorOpenUpdateFileFail",
  "ErrorWriteUpdateFileFail",
  "ErrorReformatFail",
  "ErrorUnmountFail",
  "ErrorRenameFail",
};
#endif // DEBUG_VERBOSE

OTACloudProcessInterface::OTACloudProcessInterface(MessageStream *ms)
: CloudProcess(ms)
, policies(None)
, state(Resume)
, previous_state(Resume)
, report_last_timestamp(0)
, report_counter(0)
, context(nullptr) {
}

OTACloudProcessInterface::~OTACloudProcessInterface() {
  clean();
}

void OTACloudProcessInterface::handleMessage(Message* msg) {

  if ((state >= OtaAvailable || state < 0) && previous_state != state) {
    reportStatus(static_cast<int32_t>(state<0? state : 0));
  }

  // this allows to do status report only when the state changes
  previous_state = state;

  switch(state) {
  case Resume:         updateState(resume(msg));    break;
  case OtaBegin:       updateState(otaBegin());     break;
  case Idle:           updateState(idle(msg));      break;
  case OtaAvailable:   updateState(otaAvailable()); break;
  case StartOTA:       updateState(startOTA());     break;
  case Fetch:          updateState(fetch());        break;
  case FlashOTA:       updateState(flashOTA());     break;
  case Reboot:         updateState(reboot());       break;
  case OTAUnavailable: break;
  default:             updateState(fail()); // all the states that are not defined are failures
  }
}

OTACloudProcessInterface::State OTACloudProcessInterface::otaBegin() {
  if(!isOtaCapable()) {
    DEBUG_VERBOSE("OTA is not available on this board");
    return OTAUnavailable;
  }

  struct OtaBeginUp msg = {
    OtaBeginUpId,
  };

  SHA256 sha256_calc;
  calculateSHA256(sha256_calc);

  sha256_calc.finalize(sha256);
  memcpy(msg.params.sha, sha256, SHA256::HASH_SIZE);

  DEBUG_VERBOSE("calculated SHA256: "
      "0x%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X"
      "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
      sha256[0],  sha256[1],  sha256[2],  sha256[3],  sha256[4],  sha256[5],  sha256[6],  sha256[7],
      sha256[8],  sha256[9],  sha256[10], sha256[11], sha256[12], sha256[13], sha256[14], sha256[15],
      sha256[16], sha256[17], sha256[18], sha256[19], sha256[20], sha256[21], sha256[22], sha256[23],
      sha256[24], sha256[25], sha256[26], sha256[27], sha256[28], sha256[29], sha256[30], sha256[31]
    );

  deliver((Message*)&msg);

  return Idle;
}

void OTACloudProcessInterface::calculateSHA256(SHA256& sha256_calc) {
  auto res = appFlashOpen();
  if(!res) {
    // TODO return error
    return;
  }

  sha256_calc.begin();
  sha256_calc.update(
    reinterpret_cast<const uint8_t*>(appStartAddress()),
    appSize());
  appFlashClose();
}

OTACloudProcessInterface::State OTACloudProcessInterface::idle(Message* msg) {
  // if a msg arrived, it may be an OTAavailable, then go to otaAvailable
  // otherwise do nothing
  if(msg!=nullptr && msg->id == OtaUpdateCmdDownId) {
    // save info coming from this message
    assert(context == nullptr); // This should never fail

    struct OtaUpdateCmdDown* ota_msg = (struct OtaUpdateCmdDown*)msg;

    context = new OtaContext(
        ota_msg->params.id, ota_msg->params.url,
        ota_msg->params.initialSha256, ota_msg->params.finalSha256
      );

    // TODO verify that initialSha256 is the sha256 on board
    // TODO verify that final sha is not the current sha256 (?)
    return OtaAvailable;
  }

  return Idle;
}

OTACloudProcessInterface::State OTACloudProcessInterface::otaAvailable() {
  // depending on the policy decided on this device the ota process can start immediately
  // or wait for confirmation from the user
  if((policies & (ApprovalRequired | Approved)) == ApprovalRequired ) {
    return OtaAvailable;
  } else {
    policies &= ~Approved;
    return StartOTA;
  } // TODO add an abortOTA command? in this case delete the context
}

OTACloudProcessInterface::State OTACloudProcessInterface::fail() {
  reset();
  clean();

  return Idle;
}

void OTACloudProcessInterface::clean() {
  // free the context pointer
  if(context != nullptr) {
    delete context;
    context = nullptr;
  }
}

void OTACloudProcessInterface::reportStatus(int32_t state_data) {
  if(context == nullptr) {
    // FIXME handle this case: ota not in progress
    return;
  }
  uint32_t new_timestamp = getTime();

  struct OtaProgressCmdUp msg = {
    OtaProgressCmdUpId,
  };

  memcpy(msg.params.id, context->id, ID_SIZE);
  msg.params.state        = state>=0 ? state : State::Fail;

  if(new_timestamp == report_last_timestamp) {
    msg.params.time       = new_timestamp*1e6 + ++report_counter;
  } else {
    msg.params.time       = new_timestamp*1e6;
    report_counter = 0;
    report_last_timestamp = new_timestamp;
  }

  msg.params.state_data   = state_data;

  deliver((Message*)&msg);
}

OTACloudProcessInterface::OtaContext::OtaContext(
    uint8_t id[ID_SIZE], const char* url,
    uint8_t* initialSha256, uint8_t* finalSha256
    ) : url((char*) malloc(strlen(url) + 1)) {

  memcpy(this->id, id, ID_SIZE);
  strcpy(this->url, url);
  memcpy(this->initialSha256, initialSha256, 32);
  memcpy(this->finalSha256, finalSha256, 32);
}

OTACloudProcessInterface::OtaContext::~OtaContext() {
  free(url);
}

#endif /* OTA_ENABLED */
