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

#ifndef CLOUDSCHEDULE_H_
#define CLOUDSCHEDULE_H_

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
class Schedule : public TimeService {
  public:
    unsigned int frm, to, len, msk;
    Schedule(unsigned int s, unsigned int e, unsigned int d, unsigned int m): frm(s), to(e), len(d), msk(m) {}

    bool isActive() {

      unsigned int now = getTime();
      if(checkSchedulePeriod(now, frm, to)) {
        /* We are in the schedule range */

        if(checkScheduleMask(now, msk)) {
        
          /* We can assume now that the schedule is always repeating with fixed delta */ 
          unsigned int delta = getScheduleDelta(msk);
          if ( ( (std::max(now , frm) - std::min(now , frm)) % delta ) <= len ) {
            return true;
          }
        }
      }
      return false;
    }

    Schedule& operator=(Schedule & aSchedule) {
      frm = aSchedule.frm;
      to  = aSchedule.to;
      len = aSchedule.len;
      msk = aSchedule.msk;
      return *this;
    }

    bool operator==(Schedule & aSchedule) {
      return frm == aSchedule.frm && to == aSchedule.to && len == aSchedule.len && msk == aSchedule.msk;
    }

    bool operator!=(Schedule & aSchedule) {
      return !(operator==(aSchedule));
    }
  private:
    bool isScheduleOneShot(unsigned int msk) {
      if((msk & 0x3C000000) == 0x00000000) {
        return true;
      } else {
        return false;
      }
    }

    bool isScheduleFixed(unsigned int msk) {
      if((msk & 0x3C000000) == 0x04000000) {
        return true;
      } else {
        return false;
      }
    }

    bool isScheduleWeekly(unsigned int msk) {
      if((msk & 0x3C000000) == 0x08000000) {
        return true;
      } else {
        return false;
      }
    }

    bool isScheduleMonthly(unsigned int msk) {
      if((msk & 0x3C000000) == 0x0C000000) {
        return true;
      } else {
        return false;
      }
    }

    bool isScheduleYearly(unsigned int msk) {
      if((msk & 0x3C000000) == 0x10000000) {
        return true;
      } else {
        return false;
      }
    }

    bool isScheduleInSeconds(unsigned int msk) {
      if((msk & 0xC0000000) == 0x00000000) {
        return true;
      } else {
        return false;
      }
    }

    bool isScheduleInMinutes(unsigned int msk) {
      if((msk & 0xC0000000) == 0x40000000) {
        return true;
      } else {
        return false;
      }
    }

    bool isScheduleInHours(unsigned int msk) {
      if((msk & 0xC0000000) == 0x80000000) {
        return true;
      } else {
        return false;
      }
    }

    bool isScheduleInDays(unsigned int msk) {
      if((msk & 0xC0000000) == 0xC0000000) {
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

    unsigned int getScheduleRawMask(unsigned int msk) {
      return msk & 0x03FFFFFF;
    }

    unsigned int getScheduleWeekMask(unsigned int msk) {
      return msk & 0x000000FF;
    }

    unsigned int getScheduleDay(unsigned int msk) {
      return msk & 0x000000FF;
    }

    unsigned int getScheduleMonth(unsigned int msk) {
      return (msk & 0x0000FF00) >> 8;
    }

    bool checkSchedulePeriod(unsigned int now, unsigned int frm, unsigned int to) {
      if(now >= frm && (now < to || to == 0)) {
        return true;
      } else {
        return false;
      }
    }

    bool checkScheduleMask(unsigned int now, unsigned int msk) {
      if(isScheduleFixed(msk) || isScheduleOneShot(msk)) {
        return true;
      } 
      
      if(isScheduleWeekly(msk)) {
        unsigned int nowMask = timeToWeekMask(now);
        unsigned int scheduleMask = getScheduleWeekMask(msk);
        
        if((nowMask & scheduleMask) == 0) {
          return false;
        } else {
          return true;
        }
      }

      if(isScheduleMonthly(msk)) {
        unsigned int nowDay = timeToDay(now);
        unsigned int scheduleDay = getScheduleDay(msk);

        if(nowDay != scheduleDay) {
          return false;
        } else {
          return true;
        }
      }

      if(isScheduleYearly(msk)) {
        unsigned int nowDay = timeToDay(now);
        unsigned int scheduleDay = getScheduleDay(msk);
        unsigned int nowMonth = timeToMonth(now);
        unsigned int scheduleMonth = getScheduleMonth(msk);

        if((nowDay != scheduleDay) || (nowMonth != scheduleMonth)) {
          return false;
        } else {
          return true;
        }
      }

      return false;
    }

    unsigned int getScheduleDelta(unsigned int msk) {
      if(isScheduleOneShot(msk)) {
        return 0xFFFFFFFF;
      }
      
      if(isScheduleFixed(msk)) {
        if(isScheduleInSeconds(msk)) {
          return getScheduleRawMask(msk);
        }

        if(isScheduleInMinutes(msk)) {
          return 60 * getScheduleRawMask(msk);
        }

        if(isScheduleInHours(msk)) {
          return 60 * 60 * getScheduleRawMask(msk);
        }
      }

      if(isScheduleWeekly(msk) || isScheduleMonthly(msk) || isScheduleYearly(msk)) {
        return 60 * 60 * 24;
      }

      return 0;
    }
};

class CloudSchedule : public Property {
  private:
    Schedule _value,
             _cloud_value;
  public:
    CloudSchedule() : _value(0, 0, 0, 0), _cloud_value(0, 0, 0, 0) {}
    CloudSchedule(unsigned int frm, unsigned int to, unsigned int len, unsigned int msk) : _value(frm, to, len, msk), _cloud_value(frm, to, len, msk) {}

    virtual bool isDifferentFromCloud() {

      return _value != _cloud_value;
    }

    CloudSchedule& operator=(Schedule aSchedule) {
      _value.frm = aSchedule.frm;
      _value.to  = aSchedule.to;
      _value.len = aSchedule.len;
      _value.msk = aSchedule.msk;
      updateLocalTimestamp();
      return *this;
    }

    Schedule getCloudValue() {
      return _cloud_value;
    }

    Schedule getValue() {
      return _value;
    }

    bool isActive() {
      return _value.isActive();
    }

    virtual void fromCloudToLocal() {
      _value = _cloud_value;
    }
    virtual void fromLocalToCloud() {
      _cloud_value = _value;
    }
    virtual CborError appendAttributesToCloud() {
      CHECK_CBOR(appendAttribute(_value.frm));
      CHECK_CBOR(appendAttribute(_value.to));
      CHECK_CBOR(appendAttribute(_value.len));
      CHECK_CBOR(appendAttribute(_value.msk));
      return CborNoError;
    }
    virtual void setAttributesFromCloud() {
      setAttribute(_cloud_value.frm);
      setAttribute(_cloud_value.to);
      setAttribute(_cloud_value.len);
      setAttribute(_cloud_value.msk);
    }
};

#endif /* CLOUDSCHEDULE_H_ */
