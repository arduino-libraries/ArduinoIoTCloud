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
#include "../../AIoTC_Const.h"
#include "utility/time/TimeService.h"
#include <stdint.h>
#include <time.h>

/******************************************************************************
 * DEFINE
 ******************************************************************************/
#define SCHEDULE_UNIT_MASK    static_cast<uint32_t>(0xC0000000)
#define SCHEDULE_UNIT_SHIFT   30

#define SCHEDULE_TYPE_MASK    static_cast<uint32_t>(0x3C000000)
#define SCHEDULE_TYPE_SHIFT   26

#define SCHEDULE_MONTH_MASK   static_cast<uint32_t>(0x0000FF00)
#define SCHEDULE_MONTH_SHIFT  8

#define SCHEDULE_REP_MASK     static_cast<uint32_t>(0x03FFFFFF)
#define SCHEDULE_WEEK_MASK    static_cast<uint32_t>(0x000000FF)
#define SCHEDULE_DAY_MASK     static_cast<uint32_t>(0x000000FF)

#define SCHEDULE_ONE_SHOT     static_cast<uint32_t>(0xFFFFFFFF)

/******************************************************************************
   ENUM
 ******************************************************************************/
enum class ScheduleUnit : int32_t {
  Seconds      = 0,
  Minutes      = 1,
  Hours        = 2,
  Days         = 3
};

enum class ScheduleType : int32_t {
  OneShot      = 0,
  FixedDelta   = 1,
  Weekly       = 2,
  Monthly      = 3,
  Yearly       = 4
};

enum class ScheduleMonth : int32_t {
  Jan          = 0,
  Feb          = 1,
  Mar          = 2,
  Apr          = 3,
  May          = 4,
  Jun          = 5,
  Jul          = 6,
  Aug          = 7,
  Sep          = 8,
  Oct          = 9,
  Nov          = 10,
  Dec          = 11
};

enum class ScheduleWeekDay : int32_t {
  Sun          = 0,
  Mon          = 1,
  Tue          = 2,
  Wed          = 3,
  Thu          = 4,
  Fri          = 5,
  Sat          = 6
};

enum class ScheduleState : int32_t {
  Inactive     = 0,
  Active       = 1
};

/******************************************************************************
 * TYPEDEF
 ******************************************************************************/
typedef struct ScheduleWeeklyMask {
  ScheduleState& operator[](ScheduleWeekDay i) { return day[static_cast<int32_t>(i)];}
  ScheduleState day[7];
}ScheduleWeeklyMask;

typedef uint32_t ScheduleTimeType;
typedef uint32_t ScheduleConfigurationType;

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/
class Schedule {
  public:
    ScheduleTimeType frm, to, len, msk;
    Schedule(ScheduleTimeType s, ScheduleTimeType e, ScheduleTimeType d, ScheduleConfigurationType m): frm(s), to(e), len(d), msk(m) {}
    Schedule(const Schedule& r) : frm(r.frm), to(r.to), len(r.len), msk(r.msk) {}

    bool isActive() {

      ScheduleTimeType now = TimeService.getLocalTime();

      if(checkTimeValid(now)) {
        /* We have to wait RTC configuration and Timezone setting from the cloud */

        if(checkSchedulePeriod(now, frm, to)) {
          /* We are in the schedule range */

          if(checkScheduleMask(now, msk)) {

            /* We can assume now that the schedule is always repeating with fixed delta */
            ScheduleTimeType delta = getScheduleDelta(msk);
            if ( ( (std::max(now , frm) - std::min(now , frm)) % delta ) <= len ) {
              return true;
            }
          }
        }
      }
      return false;
    }

    static ScheduleConfigurationType createOneShotScheduleConfiguration() {
      return 0;
    }

    static ScheduleConfigurationType createFixedDeltaScheduleConfiguration(ScheduleUnit unit, uint32_t delta) {
      int32_t temp_unit = static_cast<int32_t>(unit);
      int32_t temp_type = static_cast<int32_t>(ScheduleType::FixedDelta);
      uint32_t temp_delta = delta;

      if (temp_delta > SCHEDULE_REP_MASK) {
        temp_delta = SCHEDULE_REP_MASK;
      }
      return (temp_unit << SCHEDULE_UNIT_SHIFT) | (temp_type << SCHEDULE_TYPE_SHIFT) | temp_delta;
    }

    static ScheduleConfigurationType createWeeklyScheduleConfiguration(ScheduleWeeklyMask weekMask) {
      uint32_t temp_week = 0;
      int32_t temp_type = static_cast<int32_t>(ScheduleType::Weekly);

      for(size_t i = 0; i<7; i++) {
        if(weekMask[static_cast<ScheduleWeekDay>(i)] == ScheduleState::Active) {
          temp_week |= 1 << i;
        }
      }
      return (temp_type << SCHEDULE_TYPE_SHIFT) | temp_week;
    }

    static ScheduleConfigurationType createMonthlyScheduleConfiguration(int32_t dayOfTheMonth) {
      int32_t temp_day = dayOfTheMonth;
      int32_t temp_type = static_cast<int32_t>(ScheduleType::Monthly);

      if(temp_day < 1) {
        temp_day = 1;
      }

      if(temp_day > 31) {
        temp_day = 31;
      }
      return (temp_type << SCHEDULE_TYPE_SHIFT) | temp_day;
    }

    static ScheduleConfigurationType createYearlyScheduleConfiguration(ScheduleMonth month, int32_t dayOfTheMonth) {
      uint32_t temp_day = createMonthlyScheduleConfiguration(dayOfTheMonth);
      int32_t temp_month = static_cast<int32_t>(month);
      int32_t temp_type = static_cast<int32_t>(ScheduleType::Yearly);

      return (temp_type << SCHEDULE_TYPE_SHIFT) | (temp_month << SCHEDULE_MONTH_SHIFT)| temp_day;
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

    ScheduleUnit getScheduleUnit(ScheduleConfigurationType msk) {
      return static_cast<ScheduleUnit>((msk & SCHEDULE_UNIT_MASK) >> SCHEDULE_UNIT_SHIFT);
    }

    ScheduleType getScheduleType(ScheduleConfigurationType msk) {
      return static_cast<ScheduleType>((msk & SCHEDULE_TYPE_MASK) >> SCHEDULE_TYPE_SHIFT);
    }

    uint32_t getScheduleRepetition(ScheduleConfigurationType msk) {
      return (msk & SCHEDULE_REP_MASK);
    }

    uint32_t getScheduleWeekMask(ScheduleConfigurationType msk) {
      return (msk & SCHEDULE_WEEK_MASK);
    }

    uint32_t getScheduleDay(ScheduleConfigurationType msk) {
      return (msk & SCHEDULE_DAY_MASK);
    }

    uint32_t getScheduleMonth(ScheduleConfigurationType msk) {
      return ((msk & SCHEDULE_MONTH_MASK) >> SCHEDULE_MONTH_SHIFT);
    }

    bool isScheduleOneShot(ScheduleConfigurationType msk) {
      return (getScheduleType(msk) == ScheduleType::OneShot) ? true : false ;
    }

    bool isScheduleFixed(ScheduleConfigurationType msk) {
      return (getScheduleType(msk) == ScheduleType::FixedDelta) ? true : false ;
    }

    bool isScheduleWeekly(ScheduleConfigurationType msk) {
      return (getScheduleType(msk) == ScheduleType::Weekly) ? true : false ;
    }

    bool isScheduleMonthly(ScheduleConfigurationType msk) {
      return (getScheduleType(msk) == ScheduleType::Monthly) ? true : false ;
    }

    bool isScheduleYearly(ScheduleConfigurationType msk) {
      return (getScheduleType(msk) == ScheduleType::Yearly) ? true : false ;
    }

    bool isScheduleInSeconds(ScheduleConfigurationType msk) {
      if(isScheduleFixed(msk)) {
        return (getScheduleUnit(msk) == ScheduleUnit::Seconds) ? true : false ;
      } else {
        return false;
      }
    }

    bool isScheduleInMinutes(ScheduleConfigurationType msk) {
      if(isScheduleFixed(msk)) {
        return (getScheduleUnit(msk) == ScheduleUnit::Minutes) ? true : false ;
      } else {
        return false;
      }
    }

    bool isScheduleInHours(ScheduleConfigurationType msk) {
      if(isScheduleFixed(msk)) {
        return (getScheduleUnit(msk) == ScheduleUnit::Hours) ? true : false ;
      } else {
        return false;
      }
    }

    bool isScheduleInDays(ScheduleConfigurationType msk) {
      if(isScheduleFixed(msk)) {
        return (getScheduleUnit(msk) == ScheduleUnit::Days) ? true : false ;
      } else {
        return false;
      }
    }

    uint32_t getCurrentDayMask(time_t time) {
      struct tm * ptm;
      ptm = gmtime (&time);

      return 1 << ptm->tm_wday;
    }

    uint32_t getCurrentDay(time_t time) {
      struct tm * ptm;
      ptm = gmtime (&time);

      return ptm->tm_mday;
    }

    uint32_t getCurrentMonth(time_t time) {
      struct tm * ptm;
      ptm = gmtime (&time);

      return ptm->tm_mon;
    }

    bool checkTimeValid(ScheduleTimeType now) {
      return (now != 0);
    }

    bool checkSchedulePeriod(ScheduleTimeType now, ScheduleTimeType frm, ScheduleTimeType to) {
      /* Check if current time is inside the schedule period. If 'to' is equal to
       * 0 the schedule has no end.
       */
      if(now >= frm && (now < to || to == 0)) {
        return true;
      } else {
        return false;
      }
    }

    bool checkScheduleMask(ScheduleTimeType now, ScheduleConfigurationType msk) {
      if(isScheduleFixed(msk) || isScheduleOneShot(msk)) {
        return true;
      }

      if(isScheduleWeekly(msk)) {
        uint32_t currentDayMask = getCurrentDayMask(now);
        uint32_t scheduleMask = getScheduleWeekMask(msk);

        if((currentDayMask & scheduleMask) != 0) {
          return true;
        }
      }

      if(isScheduleMonthly(msk)) {
        uint32_t currentDay = getCurrentDay(now);
        uint32_t scheduleDay = getScheduleDay(msk);

        if(currentDay == scheduleDay) {
          return true;
        }
      }

      if(isScheduleYearly(msk)) {
        uint32_t currentDay = getCurrentDay(now);
        uint32_t scheduleDay = getScheduleDay(msk);
        uint32_t currentMonth = getCurrentMonth(now);
        uint32_t scheduleMonth = getScheduleMonth(msk);

        if((currentDay == scheduleDay) && (currentMonth == scheduleMonth)) {
          return true;
        }
      }

      return false;
    }

    ScheduleTimeType getScheduleDelta(ScheduleConfigurationType msk) {
      if(isScheduleInSeconds(msk)) {
        return SECONDS * getScheduleRepetition(msk);
      }

      if(isScheduleInMinutes(msk)) {
        return MINUTES * getScheduleRepetition(msk);
      }

      if(isScheduleInHours(msk)) {
        return HOURS * getScheduleRepetition(msk);
      }

      if(isScheduleInDays(msk)) {
        return DAYS * getScheduleRepetition(msk);
      }

      if(isScheduleWeekly(msk) || isScheduleMonthly(msk) || isScheduleYearly(msk)) {
        return DAYS;
      }

      return SCHEDULE_ONE_SHOT;
    }
};

class CloudSchedule : public Property {
  private:
    Schedule _value,
             _cloud_value;
  public:
    CloudSchedule() : _value(0, 0, 0, 0), _cloud_value(0, 0, 0, 0) {}
    CloudSchedule(uint32_t frm, uint32_t to, uint32_t len, uint32_t msk) : _value(frm, to, len, msk), _cloud_value(frm, to, len, msk) {}

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

    virtual CborError appendAttributesToCloud(CborEncoder *encoder) {
      CHECK_CBOR_MULTI(appendAttribute(_value.frm, "frm", encoder));
      CHECK_CBOR_MULTI(appendAttribute(_value.to, "to", encoder));
      CHECK_CBOR_MULTI(appendAttribute(_value.len, "len", encoder));
      CHECK_CBOR_MULTI(appendAttribute(_value.msk, "msk", encoder));
      return CborNoError;
    }

    virtual void setAttributesFromCloud() {
      setAttribute(_cloud_value.frm, "frm");
      setAttribute(_cloud_value.to, "to");
      setAttribute(_cloud_value.len, "len");
      setAttribute(_cloud_value.msk, "msk");
    }
};

#endif /* CLOUDSCHEDULE_H_ */
