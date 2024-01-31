/*
  This sketch demonstrates how to use the cloud schedule variable type.

  IMPORTANT:
  This sketch works with WiFi, GSM, NB and Ethernet enabled boards supported by Arduino IoT Cloud.

*/

#include "thingProperties.h"

#if !defined(LED_BUILTIN) && !defined(ARDUINO_NANO_ESP32)
static int const LED_BUILTIN = 2;
#endif

void setup() {
  /* Initialize the serial port and wait up to 5 seconds for a connection */
  Serial.begin(9600);
  for(unsigned long const serialBeginTime = millis(); !Serial && (millis() - serialBeginTime > 5000); ) { }

  /* Configure LED pin as an output */
  pinMode(LED_BUILTIN, OUTPUT);

  /* This function takes care of connecting your sketch variables to the ArduinoIoTCloud object */
  initProperties();

  /* Initialize Arduino IoT Cloud library */
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  setDebugMessageLevel(DBG_INFO);
  ArduinoCloud.printDebugInfo();

  /* Setup one shot schedule example */
  setupOneShotSchedule();

  /* Setup per minute schedule example */
  setupMinuteSchedule();

  /* Setup hourly schedule example */
  setupHourlySchedule();

  /* Setup daily schedule example */
  setupDailySchedule();

  /* Setup weekly schedule example */
  setupWeeklySchedule();

  /* Setup monthly schedule example */
  setupMonthlySchedule();

  /* Setup yearly schedule example */
  setupYearlySchedule();
}

 /* Setup a schedule with an active period of 5 minutes that doesn't repeat
  * Starting from 2021 11 01 17:00:00
  * Until         2021 11 02 17:00:00
  */
void setupOneShotSchedule() {

  ScheduleTimeType startingFrom = TimeServiceClass::getTimeFromString("2021 Nov 01 17:00:00");
  ScheduleTimeType until = startingFrom + ( DAYS * 1 );
  ScheduleTimeType activePeriod = MINUTES * 5;

  /* Warning: there is no cross check between until and activePeriod */
  ScheduleConfigurationType scheduleConfiguration =  Schedule::createOneShotScheduleConfiguration();

  oneShot = Schedule(startingFrom, until, activePeriod, scheduleConfiguration);
}

 /* Setup a schedule with an active period of 15 seconds that repeats each minute
  * Starting from 2021 11 01 17:00:00
  * Until         2021 11 02 17:00:00
  */
void setupMinuteSchedule() {

  ScheduleTimeType startingFrom = TimeServiceClass::getTimeFromString("2021 Nov 01 17:00:00");
  ScheduleTimeType until = startingFrom + ( DAYS * 1 );
  ScheduleTimeType activePeriod = SECONDS * 15;
  unsigned int repetitionPeriod = 1;

  /* Warning: there is no cross check between repetitionPeriod and activePeriod */
  ScheduleConfigurationType scheduleConfiguration =  Schedule::createFixedDeltaScheduleConfiguration(ScheduleUnit::Minutes, repetitionPeriod);

  minute = Schedule(startingFrom, until, activePeriod, scheduleConfiguration);
}

/* Setup a schedule with an active period of 20 minutes that repeats each hour
 * Starting from  2021 11 01 17:00:00
 * Until          2021 11 15 13:00:00
 */
void setupHourlySchedule() {

  ScheduleTimeType startingFrom = TimeServiceClass::getTimeFromString("2021 Nov 01 17:00:00");
  ScheduleTimeType until = TimeServiceClass::getTimeFromString("2021 Nov 15 13:00:00");
  ScheduleTimeType activePeriod = MINUTES * 20;
  unsigned int repetitionPeriod = 1;

  /* Warning: there is no cross check between repetitionPeriod and activePeriod */
  ScheduleConfigurationType scheduleConfiguration =  Schedule::createFixedDeltaScheduleConfiguration(ScheduleUnit::Hours, repetitionPeriod);

  hourly = Schedule(startingFrom, until, activePeriod, scheduleConfiguration);
}

/* Setup a schedule with an active period of 2 hours that repeats each day
 * Starting from  2021 11 01 17:00:00
 * Until          2021 11 15 13:00:00
 */
void setupDailySchedule() {

  ScheduleTimeType startingFrom = TimeServiceClass::getTimeFromString("2021 Nov 01 17:00:00");
  ScheduleTimeType until = TimeServiceClass::getTimeFromString("2021 Nov 15 13:00:00");
  ScheduleTimeType activePeriod = HOURS * 2;
  unsigned int repetitionPeriod = 1;

  /* Warning: there is no cross check between repetitionPeriod and activePeriod */
  ScheduleConfigurationType scheduleConfiguration =  Schedule::createFixedDeltaScheduleConfiguration(ScheduleUnit::Days, repetitionPeriod);

  daily = Schedule(startingFrom, until, activePeriod, scheduleConfiguration);
}

/* Setup a schedule with an active period of 3 minutes with a weekly configuration
 * Starting from  2021 11 01 17:00:00
 * Until          2021 11 31 17:00:00
 * Weekly configuration
 * Sunday    -> Inactive
 * Monday    -> Active
 * Tuesday   -> Inactive
 * Wednesday -> Active
 * Thursday  -> Inactive
 * Friday    -> Active
 * Saturday  -> Inactive
 */
void setupWeeklySchedule() {

  unsigned int startingFrom = TimeServiceClass::getTimeFromString("2021 Nov 01 17:00:00");
  unsigned int until = startingFrom + ( DAYS * 30 );
  unsigned int executionPeriod = MINUTES * 3;

  ScheduleWeeklyMask WeeklyMask = {
    ScheduleState::Inactive,   /* Sunday */
    ScheduleState::Active,     /* Monday */
    ScheduleState::Inactive,   /* Tuesday */
    ScheduleState::Active,     /* Wednesday */
    ScheduleState::Inactive,   /* Thursday */
    ScheduleState::Active,     /* Friday */
    ScheduleState::Inactive,   /* Saturday */
  };

  ScheduleConfigurationType scheduleConfiguration =  Schedule::createWeeklyScheduleConfiguration(WeeklyMask);

  weekly = Schedule(startingFrom, until, executionPeriod, scheduleConfiguration);
}

/* Setup a schedule with an active period of 1 day that repeats each third day of the month
 * Starting from  2021 11 01 17:00:00
 * Until          2022 11 15 13:00:00
 */
void setupMonthlySchedule() {

  ScheduleTimeType startingFrom = TimeServiceClass::getTimeFromString("2021 Nov 01 17:00:00");
  ScheduleTimeType until = TimeServiceClass::getTimeFromString("2021 Nov 15 13:00:00");
  ScheduleTimeType activePeriod = DAYS * 1;
  int dayOfMonth = 3;

  ScheduleConfigurationType scheduleConfiguration =  Schedule::createMonthlyScheduleConfiguration(dayOfMonth);

  monthly = Schedule(startingFrom, until, activePeriod, scheduleConfiguration);
}


/* Setup a schedule with an active period of 2 days that repeats each year on November 6th
 * Starting from  2021 11 06 17:00:00
 * Until          2041 11 15 13:00:00
 */
void setupYearlySchedule() {

  ScheduleTimeType startingFrom = TimeServiceClass::getTimeFromString("2021 Nov 06 17:00:00");
  ScheduleTimeType until = TimeServiceClass::getTimeFromString("2041 Nov 06 13:00:00");
  ScheduleTimeType activePeriod = DAYS * 2;
  int dayOfMonth = 6;

  ScheduleConfigurationType scheduleConfiguration =  Schedule::createYearlyScheduleConfiguration(ScheduleMonth::Nov, dayOfMonth);

  yearly = Schedule(startingFrom, until, activePeriod, scheduleConfiguration);
}

void loop() {
  ArduinoCloud.update();

  /* Print a message when the oneShot schedule is active */
  if(oneShot.isActive()) {
    Serial.println("One shot schedule is active");
  }

  /* Print a message when the per minute schedule is active */
  if(minute.isActive()) {
    Serial.println("Per minute schedule is active");
  }

  /* Print a message when the hourly schedule is active */
  if(hourly.isActive()) {
    Serial.println("Hourly schedule is active");
  }

  /* Print a message when the daily schedule is active */
  if(daily.isActive()) {
    Serial.println("Daily schedule is active");
  }
  
  /* Activate LED when the weekly schedule is active */
  digitalWrite(LED_BUILTIN, weekly.isActive());

  /* Print a message when the monthly schedule is active */
  if(monthly.isActive()) {
    Serial.println("Monthly schedule is active");
  }

  /* Print a message when the yearly schedule is active */
  if(yearly.isActive()) {
    Serial.println("Yearly schedule is active");
  }

}
