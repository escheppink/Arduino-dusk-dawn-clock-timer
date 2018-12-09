/*
 * Real time clock abstraction
 */

#ifndef RTC_CONTROL_H
#define RTC_CONTROL_H

#include "Arduino.h"

namespace dusk_dawn_timer {
  
#define MINUTES_PER_HOUR 60
#define MINUTES_PER_DAY 1440

class RtcControl {
public:
  RtcControl();
  void begin();
  void update();

  static uint8_t getDaysPerMonth(const uint8_t& month, const uint16_t& year);
  uint8_t getDayOfTheWeek() const;
  uint16_t getYear() const;
  uint8_t getMonth() const;
  uint8_t getDay() const;
  uint16_t getMinutesSinceMidnight() const;
  bool dayLightSaving() const;
  void setDateTime(const uint16_t& year, const uint8_t& month, const uint8_t& day, const uint16_t& minutesSinceMidnight);
  static inline uint8_t hours(const int& minutesSinceMidnight) { return minutesSinceMidnight/MINUTES_PER_HOUR; }
  static inline uint8_t minutes(const int& minutesSinceMidnight) { return minutesSinceMidnight%MINUTES_PER_HOUR; }
private:
  static uint8_t dayOfTheWeek(const uint16_t& year, const uint8_t& month, const uint8_t& day);
  void updateNow();
  void checkDayLightSaving();
  uint16_t mYear;
  uint8_t mMonth;
  uint8_t mDay;
  uint8_t mDayOfTheWeek;
  uint16_t mMinutesSinceMidnight;
  unsigned long mTimeLastUpdate; // 10 second resolution is good enough.
  bool mDayLightSaving;

  // RTC based on the DS3231 chip connected via I2C and the Wire library
  class RTC_DS3231 {
  public:
      static void adjust(const uint16_t& year, const uint8_t& month, const uint8_t& day, const uint16_t& minutesSinceMidnight);
      static bool lostPower(void);
      static void now(uint16_t& year, uint8_t& month, uint8_t& day, uint16_t& minutesSinceMidnight);
  };
};

} // Namespace
#endif  // RTC_CONTROL_H
