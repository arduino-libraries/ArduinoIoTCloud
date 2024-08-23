/*
  This file is part of the Arduino_SecureElement library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef TIMED_ATTEMPT_H
#define TIMED_ATTEMPT_H

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class TimedAttempt {

public:
  TimedAttempt(unsigned long minDelay, unsigned long maxDelay);

  void begin(unsigned long delay);
  void begin(unsigned long minDelay, unsigned long maxDelay);
  unsigned long reconfigure(unsigned long minDelay, unsigned long maxDelay);
  unsigned long retry();
  unsigned long reload();
  void reset();
  bool isRetry();
  bool isExpired();
  unsigned int getRetryCount();
  unsigned int getWaitTime();

private:
  unsigned long _minDelay;
  unsigned long _maxDelay;
  unsigned long _retryTick;
  unsigned long _retryDelay;
  unsigned int _retryCount;
};

#endif /* TIMED_ATTEMPT_H */
