/*
 * Clock switch timer class
 * Controls a specific output pin based on time events
 * Supports time, sunup and sundown.
 */

#ifndef TIMER_H
#define TIMER_H

#include "rtccontrol.h"
#include "Dusk2Dawn.h"

namespace dusk_dawn_timer {

// Timer types below are used in persist, do not change!
#define TIME 0
#define SUNUP 1
#define SUNDOWN 2

struct SwitchAction
{
  SwitchAction() { }
  SwitchAction(uint8_t type) : mSwitchType(type), mTime(0) {}
  SwitchAction(uint8_t type, int16_t time) : mSwitchType(type), mTime(time) {}  
  SwitchAction(int16_t time) : mSwitchType(TIME), mTime(time) {}
  
  uint8_t mSwitchType;
  int16_t mTime;
};

class Timer {
public:
  Timer(RtcControl* rtc, Dusk2Dawn* d2d);
  void begin();
  void update();
  uint16_t getNextSwitchTime();

  void manualSwitch();
  bool isSwitchedOn();
  bool isSwitchedManual() { return mManualSwitchTime != -1; }

  inline uint8_t getWeekDayOnType() { return mWeekDayOn.mSwitchType; }
  inline int16_t getWeekDayOnTime() { return mWeekDayOn.mTime; }
  inline uint8_t getWeekDayOffType() { return mWeekDayOff.mSwitchType; }
  inline int16_t getWeekDayOffTime() { return mWeekDayOff.mTime; }
  inline uint8_t getWeekendOnType() { return mWeekendOn.mSwitchType; }
  inline int16_t getWeekendOnTime() { return mWeekendOn.mTime; }
  inline uint8_t getWeekendOffType() { return mWeekendOff.mSwitchType; }
  inline int16_t getWeekendOffTime() { return mWeekendOff.mTime; }
  
  void setWeekTimer(const uint8_t& on_type, const int16_t& on_time, const uint8_t& off_type, const int16_t& off_time);  
  void setWeekendTimer(const uint8_t& on_type, const int16_t& on_time, const uint8_t& off_type, const int16_t& off_time);  
private:
  inline static bool isWeekDay(uint8_t dayOfTheWeek) { return dayOfTheWeek > 0 && dayOfTheWeek < 5; /* Mo, Tu, We, Th */ }
  int16_t getTimerTime(const SwitchAction& action);
  void getNextWeekDaySwitch(const uint8_t& dayOfTheWeek, const uint16_t& minutesSinceMidnight, uint16_t& nextSwitchTime, bool& switchedOn) const;
  void getNextWeekendSwitch(const uint8_t& dayOfTheWeek, const uint16_t& minutesSinceMidnight, uint16_t& nextSwitchTime, bool& switchedOn) const;
  void getNextSwitch(const uint8_t& dayOfTheWeek, const uint16_t& minutesSinceMidnight, const int16_t& switchOnTime, const int16_t& switchOffTime, uint16_t& nextSwitchTime, bool& switchedOn) const;
  
  RtcControl* mRealTimeClock;
  Dusk2Dawn* md2d;
  SwitchAction mWeekDayOn;
  SwitchAction mWeekDayOff;
  SwitchAction mWeekendOn;
  SwitchAction mWeekendOff;
  bool mSwitchedOn = false;
  uint8_t mMinuteCache = 0;
  uint16_t mNextSwitchTime = 0;
  int16_t mManualSwitchTime = -1;
};

} // Namespace

#endif // TIMER_H
