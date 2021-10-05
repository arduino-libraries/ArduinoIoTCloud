//
// This file is part of ArduinoCloudThing
//
// Copyright 2021 ARDUINO SA (http://www.arduino.cc/)
//
// This software is released under the GNU General Public License version 3,
// which covers the main part of ArduinoCloudThing.
// The terms of this license can be found at:
// https://www.gnu.org/licenses/gpl-3.0.en.html
//
// You can be released from the requirements of the above licenses by purchasing
// a commercial license. Buying such a license is mandatory if you want to modify or
// otherwise use the software for commercial activities involving the Arduino
// software without disclosing the source code of your own applications. To purchase
// a commercial license, send an email to license@arduino.cc.
//

#ifndef CLOUDSCHEDULER_H_
#define CLOUDSCHEDULER_H_

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include <Arduino.h>
#include "../Property.h"
#include "utility/time/TimeService.h"
#include <time.h>

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/
enum class MaskType : int {
  minute =  0,
  hour   =  1,
  day    =  2,
  week   =  3, /*Weekly daymask */
  month  =  4, /*Day of the month 1-31 */
  year   =  5  /*Month 1-12 + Day of the month 1-31 */
};

class Scheduler : public TimeService {
  public:
    unsigned int start, end, duration;
    int type;
    unsigned int mask;
    Scheduler(unsigned int s, unsigned int e, unsigned int d, int t, unsigned int m): start(s), end(e), duration(d), type(t), mask(m) {}

    bool isActive() {

      unsigned int now = getTime();
      if(now >= start && (now < end || end == 0)) {
        /* We are in the schedule range */

        if(type == 3 || type == 4 || type == 5) {
          unsigned int nowmask = timeToMask(type, now);
          if ( (nowmask & mask) == 0) {
            /* This is not the correct Day or Month */
            return false;
          }
        }

        /* We can assume now that the schedule is always repeating with fixed delta */ 
        unsigned int delta = getScheduleDelta(type, mask);
        if ( ( (std::max(now , start) - std::min(now , start)) % delta ) <= duration ) {
          return true;
        }
      }
      return false;
    }

    Scheduler& operator=(Scheduler & aScheduler) {
      start = aScheduler.start;
      end = aScheduler.end;
      duration = aScheduler.duration;
      type = aScheduler.type;
      mask = aScheduler.mask;
      return *this;
    }

    bool operator==(Scheduler & aScheduler) {
      return start == aScheduler.start && end == aScheduler.end && duration == aScheduler.duration && type == aScheduler.type && mask == aScheduler.mask;
    }

    bool operator!=(Scheduler & aScheduler) {
      return !(operator==(aScheduler));
    }
  private:

    unsigned int timeToMask(int type, time_t rawtime) {
      struct tm * ptm;
      ptm = gmtime ( &rawtime );

      if (type == 3) {
        return 1 << ptm->tm_wday;
      }

      if (type == 4) {
        return ptm->tm_mday;
      }

      if (type == 5) {
        return (tm->tm_mon << 16) | ptm->tm_mday;
      }
      return 0;
    }

    unsigned int getScheduleDelta(int type, unsigned int mask) {
      if (type == 0) {
        return 60 * mask;
      }

      if (type == 1) {
        return 60 * 60 * mask;
      }

      if (type == 2) {
        return 60 * 60 * 24 * mask;
      }

      if (type == 3) {
        return 60 * 60 * 24;
      }

      if (type == 4) {
        return 60 * 60 * 24;
      }

      if (type == 5) {
        return 60 * 60 * 24;
      }
      return 0;
    }
};

class CloudScheduler : public Property {
  private:
    Scheduler _value,
              _cloud_value;
  public:
    CloudScheduler() : _value(0, 0, 0, 0, 0), _cloud_value(0, 0, 0, 0, 0) {}
    CloudScheduler(unsigned int start, unsigned int end, unsigned int duration, int type, unsigned int mask) : _value(start, end, duration, type, mask), _cloud_value(start, end, duration, type, mask) {}

    virtual bool isDifferentFromCloud() {

      return _value != _cloud_value;
    }

    CloudScheduler& operator=(Scheduler aScheduler) {
      _value.start = aScheduler.start;
      _value.end = aScheduler.end;
      _value.duration = aScheduler.duration;
      _value.type = aScheduler.type;
      _value.mask = aScheduler.mask;
      updateLocalTimestamp();
      return *this;
    }

    Scheduler getCloudValue() {
      return _cloud_value;
    }

    Scheduler getValue() {
      return _value;
    }

    virtual void fromCloudToLocal() {
      _value = _cloud_value;
    }
    virtual void fromLocalToCloud() {
      _cloud_value = _value;
    }
    virtual CborError appendAttributesToCloud() {
      CHECK_CBOR(appendAttribute(_value.start));
      CHECK_CBOR(appendAttribute(_value.end));
      CHECK_CBOR(appendAttribute(_value.duration));
      CHECK_CBOR(appendAttribute(_value.type));
      CHECK_CBOR(appendAttribute(_value.mask));
      return CborNoError;
    }
    virtual void setAttributesFromCloud() {
      setAttribute(_cloud_value.start);
      setAttribute(_cloud_value.end);
      setAttribute(_cloud_value.duration);
      setAttribute(_cloud_value.type);
      setAttribute(_cloud_value.mask);
    }
};

#endif /* CLOUDSCHEDULER_H_ */
