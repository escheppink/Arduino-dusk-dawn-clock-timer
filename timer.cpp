/*
 * Clock switch timer class
 * Controls a specific output pin based on time events
 * Supports time, sunup and sundown.
 */

#include "timer.h"
#include "persist.h"

// change to true for first time programming
#define INITIALIZE_EEPROM_MEMORY false

namespace dusk_dawn_timer {

#define PINOUT A2 // Pin to control solid-state relay

Timer::Timer(RtcControl* rtc, Dusk2Dawn* d2d)
  : mRealTimeClock(rtc),
    md2d(d2d)
{ }

void Timer::begin()
{
  if (INITIALIZE_EEPROM_MEMORY) // EEPROM initialization
  {
    Persist::clearmem();
    Persist::setWeekTimer(SUNDOWN, 15, TIME, 22*60+15);
    Persist::setWeekendTimer(SUNDOWN, 15, TIME, 22*60+45);
  }
  pinMode(PINOUT, OUTPUT);  
  digitalWrite(PINOUT, HIGH);
  Persist::getWeekTimer(mWeekDayOn.mSwitchType, mWeekDayOn.mTime, mWeekDayOff.mSwitchType, mWeekDayOff.mTime);
  Persist::getWeekendTimer(mWeekendOn.mSwitchType, mWeekendOn.mTime, mWeekendOff.mSwitchType, mWeekendOff.mTime);
}

void Timer::update()
{
  uint16_t minutesSinceMidnight = mRealTimeClock->getMinutesSinceMidnight();
  if (mMinuteCache != RtcControl::minutes(minutesSinceMidnight))
  {
    mMinuteCache = RtcControl::minutes(minutesSinceMidnight);
    if (minutesSinceMidnight == mManualSwitchTime) mManualSwitchTime = -1;
    
    uint8_t dayOfTheWeek = mRealTimeClock->getDayOfTheWeek();
    bool currentOnOff = mSwitchedOn;
    if (isWeekDay(dayOfTheWeek)) getNextWeekDaySwitch(dayOfTheWeek, minutesSinceMidnight, mNextSwitchTime, mSwitchedOn);
    else getNextWeekendSwitch(dayOfTheWeek, minutesSinceMidnight, mNextSwitchTime, mSwitchedOn);
    
    if (mManualSwitchTime != -1 &&
        mManualSwitchTime == mNextSwitchTime)
    {
      mSwitchedOn = !mSwitchedOn;      
    }
    if (currentOnOff != mSwitchedOn)
    {
      digitalWrite(PINOUT, mSwitchedOn ? LOW : HIGH);    
    }
  }
}

uint16_t Timer::getNextSwitchTime()
{
  return mNextSwitchTime;
}

void Timer::manualSwitch()
{
  if (mManualSwitchTime == -1) mManualSwitchTime = mNextSwitchTime;
  else mManualSwitchTime = -1;
  mMinuteCache = 0;
//  mManualSwitchTime = mRealTimeClock->getDateTime();
}

bool Timer::isSwitchedOn()
{
  return mSwitchedOn;
}

void Timer::setWeekTimer(const uint8_t& on_type, const int16_t& on_time, const uint8_t& off_type, const int16_t& off_time)
{
  mWeekDayOn.mSwitchType = on_type;
  mWeekDayOn.mTime = on_time;
  mWeekDayOff.mSwitchType = off_type;
  mWeekDayOff.mTime = off_time;
  Persist::setWeekTimer(mWeekDayOn.mSwitchType, mWeekDayOn.mTime, mWeekDayOff.mSwitchType, mWeekDayOff.mTime);
  mMinuteCache = 0;
}
void Timer::setWeekendTimer(const uint8_t& on_type, const int16_t& on_time, const uint8_t& off_type, const int16_t& off_time)
{
  mWeekendOn.mSwitchType = on_type;
  mWeekendOn.mTime = on_time;
  mWeekendOff.mSwitchType = off_type;
  mWeekendOff.mTime = off_time;
  Persist::setWeekendTimer(mWeekendOn.mSwitchType, mWeekendOn.mTime, mWeekendOff.mSwitchType, mWeekendOff.mTime);
  mMinuteCache = 0;  
}

int16_t Timer::getTimerTime(const SwitchAction& action)
{
  switch (action.mSwitchType)
  {
    case SUNUP:
    {
      return md2d->mSunrise + action.mTime;
    }
    case SUNDOWN:
    {
      return md2d->mSunset + action.mTime;
    }
    case TIME:
    {
      return action.mTime;
    }
  };
}

void Timer::getNextWeekDaySwitch(const uint8_t& dayOfTheWeek, const uint16_t& minutesSinceMidnight, uint16_t& nextSwitchTime, bool& switchedOn) const
{
  getNextSwitch(dayOfTheWeek, minutesSinceMidnight, getTimerTime(mWeekDayOn), getTimerTime(mWeekDayOff), nextSwitchTime, switchedOn);
}
void Timer::getNextWeekendSwitch(const uint8_t& dayOfTheWeek, const uint16_t& minutesSinceMidnight, uint16_t& nextSwitchTime, bool& switchedOn) const
{
  getNextSwitch(dayOfTheWeek, minutesSinceMidnight, getTimerTime(mWeekendOn), getTimerTime(mWeekendOff), nextSwitchTime, switchedOn);
}

void Timer::getNextSwitch(const uint8_t& dayOfTheWeek, const uint16_t& minutesSinceMidnight, const int16_t& switchOnTime, const int16_t& switchOffTime, uint16_t& nextSwitchTime, bool& switchedOn) const
{
  uint16_t switch1Time = switchOnTime; // 1 not elapsed is off
  uint16_t switch2Time = switchOffTime; // 2 not elapsed is on
  bool swap = switch1Time < switch2Time;
  if (swap)
  {
    uint16_t temp = switch1Time;
    switch1Time = switch2Time;
    switch2Time = temp;
  }
  if (minutesSinceMidnight < switch2Time) 
  {
    switchedOn = !swap; // next switch is off, so now on (if not swapped)
    nextSwitchTime = switch2Time;
  }
  else if (minutesSinceMidnight < switch1Time) 
  {
    switchedOn = swap; // next switch is on, so now off (if not swapped)     
    nextSwitchTime = switch1Time;
  }
  else
  {
    // Look at first switch of next day
    if (isWeekDay(dayOfTheWeek + 1 %7)) getNextWeekDaySwitch(dayOfTheWeek + 1, 0, nextSwitchTime, switchedOn);
    else getNextWeekendSwitch(dayOfTheWeek + 1, 0, nextSwitchTime, switchedOn);
  }
}  


} // Namespace
