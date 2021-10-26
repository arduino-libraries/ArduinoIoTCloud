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
class Scheduler : public TimeService {
  public:
    unsigned int start, end, duration, mask;
    Scheduler(unsigned int s, unsigned int e, unsigned int d, unsigned int m): start(s), end(e), duration(d), mask(m) {}

    bool isActive() {

      unsigned int now = getTime();
      if(checkSchedulePeriod(now, start, end)) {
        /* We are in the schedule range */

        if(checkScheduleMask(now, mask)) {
        
          /* We can assume now that the schedule is always repeating with fixed delta */ 
          unsigned int delta = getScheduleDelta(mask);
          if ( ( (std::max(now , start) - std::min(now , start)) % delta ) <= duration ) {
            return true;
          }
        }
      }
      return false;
    }

    Scheduler& operator=(Scheduler & aScheduler) {
      start = aScheduler.start;
      end = aScheduler.end;
      duration = aScheduler.duration;
      mask = aScheduler.mask;
      return *this;
    }

    bool operator==(Scheduler & aScheduler) {
      return start == aScheduler.start && end == aScheduler.end && duration == aScheduler.duration && mask == aScheduler.mask;
    }

    bool operator!=(Scheduler & aScheduler) {
      return !(operator==(aScheduler));
    }
  private:
    bool isScheduleOneShot(unsigned int mask) {
      if((mask & 0x3C000000) == 0x00000000) {
        return true;
      } else {
        return false;
      }
    }

    bool isScheduleFixed(unsigned int mask) {
      if((mask & 0x3C000000) == 0x04000000) {
        return true;
      } else {
        return false;
      }
    }

    bool isScheduleWeekly(unsigned int mask) {
      if((mask & 0x3C000000) == 0x08000000) {
        return true;
      } else {
        return false;
      }
    }

    bool isScheduleMonthly(unsigned int mask) {
      if((mask & 0x3C000000) == 0x0C000000) {
        return true;
      } else {
        return false;
      }
    }

    bool isScheduleYearly(unsigned int mask) {
      if((mask & 0x3C000000) == 0x10000000) {
        return true;
      } else {
        return false;
      }
    }

    bool isScheduleInSeconds(unsigned int mask) {
      if((mask & 0xC0000000) == 0x00000000) {
        return true;
      } else {
        return false;
      }
    }

    bool isScheduleInMinutes(unsigned int mask) {
      if((mask & 0xC0000000) == 0x40000000) {
        return true;
      } else {
        return false;
      }
    }

    bool isScheduleInHours(unsigned int mask) {
      if((mask & 0xC0000000) == 0x80000000) {
        return true;
      } else {
        return false;
      }
    }

    bool isScheduleInDays(unsigned int mask) {
      if((mask & 0xC0000000) == 0xC0000000) {
        return true;
      } else {
        return false;
      }
    }

    unsigned int timeToWeekMask(time_t rawtime) {
      struct tm * ptm;
      ptm = gmtime ( &rawtime );

      return 1 << ptm->tm_wday;
    }

    unsigned int timeToDay(time_t rawtime) {
      struct tm * ptm;
      ptm = gmtime ( &rawtime );

      return ptm->tm_mday;
    }

    unsigned int timeToMonth(time_t rawtime) {
      struct tm * ptm;
      ptm = gmtime ( &rawtime );

      return ptm->tm_mon;
    }

    unsigned int getScheduleRawMask(unsigned int mask) {
      return mask & 0x03FFFFFF;
    }

    unsigned int getScheduleWeekMask(unsigned int mask) {
      return mask & 0x000000FF;
    }

    unsigned int getScheduleDay(unsigned int mask) {
      return mask & 0x000000FF;
    }

    unsigned int getScheduleMonth(unsigned int mask) {
      return (mask & 0x0000FF00) >> 8;
    }

    bool checkSchedulePeriod(unsigned int now, unsigned int start, unsigned int end) {
      if(now >= start && (now < end || end == 0)) {
        return true;
      } else {
        return false;
      }
    }

    bool checkScheduleMask(unsigned int now, unsigned int mask) {
      if(isScheduleFixed(mask) || isScheduleOneShot(mask)) {
        return true;
      } 
      
      if(isScheduleWeekly(mask)) {
        unsigned int nowMask = timeToWeekMask(now);
        unsigned int scheduleMask = getScheduleWeekMask(mask);
        
        if((nowMask & scheduleMask) == 0) {
          return false;
        } else {
          return true;
        }
      }

      if(isScheduleMonthly(mask)) {
        unsigned int nowDay = timeToDay(now);
        unsigned int scheduleDay = getScheduleDay(mask);

        if(nowDay != scheduleDay) {
          return false;
        } else {
          return true;
        }
      }

      if(isScheduleYearly(mask)) {
        unsigned int nowDay = timeToDay(now);
        unsigned int scheduleDay = getScheduleDay(mask);
        unsigned int nowMonth = timeToMonth(now);
        unsigned int scheduleMonth = getScheduleMonth(mask);

        if((nowDay != scheduleDay) || (nowMonth != scheduleMonth)) {
          return false;
        } else {
          return true;
        }
      }

      return false;
    }

    unsigned int getScheduleDelta(unsigned int mask) {
      if(isScheduleOneShot(mask)) {
        return 0xFFFFFFFF;
      }
      
      if(isScheduleFixed(mask)) {
        if(isScheduleInSeconds(mask)) {
          return getScheduleRawMask(mask);
        }

        if(isScheduleInMinutes(mask)) {
          return 60 * getScheduleRawMask(mask);
        }

        if(isScheduleInHours(mask)) {
          return 60 * 60 * getScheduleRawMask(mask);
        }
      }

      if(isScheduleWeekly(mask) || isScheduleMonthly(mask) || isScheduleYearly(mask)) {
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
    CloudScheduler() : _value(0, 0, 0, 0), _cloud_value(0, 0, 0, 0) {}
    CloudScheduler(unsigned int start, unsigned int end, unsigned int duration, unsigned int mask) : _value(start, end, duration, mask), _cloud_value(start, end, duration, mask) {}

    virtual bool isDifferentFromCloud() {

      return _value != _cloud_value;
    }

    CloudScheduler& operator=(Scheduler aScheduler) {
      _value.start = aScheduler.start;
      _value.end = aScheduler.end;
      _value.duration = aScheduler.duration;
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
      CHECK_CBOR(appendAttribute(_value.mask));
      return CborNoError;
    }
    virtual void setAttributesFromCloud() {
      setAttribute(_cloud_value.start);
      setAttribute(_cloud_value.end);
      setAttribute(_cloud_value.duration);
      setAttribute(_cloud_value.mask);
    }
};

#endif /* CLOUDSCHEDULER_H_ */
