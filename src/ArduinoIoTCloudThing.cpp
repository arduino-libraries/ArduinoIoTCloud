/*
  This file is part of the Arduino_SecureElement library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/


/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <AIoTC_Config.h>

#if defined(HAS_NOTECARD) || defined(HAS_TCP)

#include "ArduinoIoTCloudThing.h"
#include "interfaces/CloudProcess.h"
#include "property/types/CloudWrapperInt.h"
#include "property/types/CloudWrapperUnsignedInt.h"

/******************************************************************************
 * CTOR/DTOR
 ******************************************************************************/
ArduinoCloudThing::ArduinoCloudThing(MessageStream* ms)
: CloudProcess(ms),
_state{State::Init},
_syncAttempt(0, 0),
_propertyContainer(),
_propertyContainerIndex(0),
_utcOffset(0),
_utcOffsetProperty(nullptr),
_utcOffsetExpireTime(0),
_utcOffsetExpireTimeProperty(nullptr) {
}

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

void ArduinoCloudThing::begin() {
  Property* property;

  property = new CloudWrapperInt(_utcOffset);
  _utcOffsetProperty = &addPropertyToContainer(getPropertyContainer(),
                                               *property,
                                               "tz_offset",
                                               Permission::ReadWrite, -1);
  _utcOffsetProperty->writeOnDemand();
  property = new CloudWrapperUnsignedInt(_utcOffsetExpireTime);
  _utcOffsetExpireTimeProperty = &addPropertyToContainer(getPropertyContainer(),
                                                         *property,
                                                         "tz_dst_until",
                                                         Permission::ReadWrite, -1);
  _utcOffsetExpireTimeProperty->writeOnDemand();
}

void ArduinoCloudThing::update() {
  handleMessage(nullptr);
}

int ArduinoCloudThing::connected() {
  return _state > State::Disconnect ? 1 : 0;
}

void ArduinoCloudThing::handleMessage(Message* m) {
  _command = UnknownCmdId;
  if (m != nullptr) {
    _command = m->id;
    if (_command == TimezoneCommandDownId) {
      _utcOffset = reinterpret_cast<TimezoneCommandDown*>(m)->params.offset;
      _utcOffsetExpireTime = reinterpret_cast<TimezoneCommandDown*>(m)->params.until;
    }
  }

  /* Run through the state machine. */
  State nextState = _state;
  switch (_state) {
    case State::Init:              nextState = handleInit();              break;
    case State::RequestLastValues: nextState = handleRequestLastValues(); break;
    case State::Connected:         nextState = handleConnected();         break;
    case State::Disconnect:        nextState = handleDisconnect();        break;
  }

  /* Handle external events */
  switch (_command) {
    case LastValuesUpdateCmdId:
      if (_state == State::RequestLastValues) {
        DEBUG_VERBOSE("CloudThing::%s Thing is synced", __FUNCTION__);
        nextState = State::Connected;
      }
      break;

    /* We have received a timezone update */
    case TimezoneCommandDownId:
      TimeService.setTimeZoneData(_utcOffset, _utcOffsetExpireTime);
    break;

    /* We have received a reset command */
    case ResetCmdId:
      nextState = State::Init;
      break;

    default:
      break;
  }

  _command = UnknownCmdId;
  _state = nextState;
}

ArduinoCloudThing::State ArduinoCloudThing::handleInit() {
  _syncAttempt.begin(AIOT_CONFIG_TIMEOUT_FOR_LASTVALUES_SYNC_ms);
  return State::RequestLastValues;
}

ArduinoCloudThing::State ArduinoCloudThing::handleRequestLastValues() {
  /* Check whether or not we need to send a new request. */
  if (_syncAttempt.isRetry() && !_syncAttempt.isExpired()) {
    return State::RequestLastValues;
  }

  /* Track the number of times a get-last-values request was sent to the cloud.
   * If no data is received within a certain number of retry-requests it's a
   * better strategy to disconnect and re-establish connection from the ground up.
   */
  if (_syncAttempt.getRetryCount() > AIOT_CONFIG_LASTVALUES_SYNC_MAX_RETRY_CNT) {
    return State::Disconnect;
  }

  _syncAttempt.retry();

  /* Send message upstream to inform infrastructure we need to request thing
   * last values
   */
  DEBUG_VERBOSE("CloudThing::%s not int sync. %d next sync request in %d ms",
                __FUNCTION__, _syncAttempt.getRetryCount(), _syncAttempt.getWaitTime());
  Message message = { LastValuesBeginCmdId };
  deliver(&message);

  return State::RequestLastValues;
}

ArduinoCloudThing::State ArduinoCloudThing::handleConnected() {
  /* Check if a primitive property wrapper is locally changed.
  * This function requires an existing time service which in
  * turn requires an established connection. Not having that
  * leads to a wrong time set in the time service which inhibits
  * the connection from being established due to a wrong data
  * in the reconstructed certificate.
  */
  updateTimestampOnLocallyChangedProperties(getPropertyContainer());

  /* Configure Time service with timezone data:
  * _utcOffset [offset + dst]
  * _utcOffsetExpireTime [posix timestamp until _utcOffset is valid]
  */
  if (_utcOffsetProperty->isDifferentFromCloud() ||
      _utcOffsetExpireTimeProperty->isDifferentFromCloud()) {
    _utcOffsetProperty->fromCloudToLocal();
    _utcOffsetExpireTimeProperty->fromCloudToLocal();
    TimeService.setTimeZoneData(_utcOffset, _utcOffsetExpireTime);
  }

  /* Check if any property needs encoding and send them to the cloud */
  Message message = { PropertiesUpdateCmdId };
  deliver(&message);

  if (getTime() > _utcOffsetExpireTime) {
    return State::RequestLastValues;
  }

  return State::Connected;
}

ArduinoCloudThing::State ArduinoCloudThing::handleDisconnect() {
  return State::Disconnect;
}

#endif /* HAS_NOTECARD || HAS_TCP */
