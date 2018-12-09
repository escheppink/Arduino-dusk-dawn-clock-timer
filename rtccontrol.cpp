#include "rtccontrol.h"
// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include <Wire.h>

namespace dusk_dawn_timer {
  
#define NORMALDELAY 1000  // 10 sec

#define DS3231_ADDRESS  0x68
#define DS3231_CONTROL  0x0E
#define DS3231_STATUSREG 0x0F

RtcControl::RtcControl()
  : mDayLightSaving(false)
{ }

void RtcControl::begin()
{
  Wire.begin();

  if (RTC_DS3231::lostPower()) {
    // This line sets the RTC with an explicit date & time
    RTC_DS3231::adjust(2018,1,1,0);    
  }
  updateNow();
  mTimeLastUpdate = millis();
  checkDayLightSaving(); 
}

void RtcControl::update()
{
  // RTC time update
  unsigned long now = millis();
  if (now - mTimeLastUpdate > NORMALDELAY)
  {
    mTimeLastUpdate = now;
    updateNow();
  }
}

const uint8_t daysInMonth [] PROGMEM = { 31,28,31,30,31,30,31,31,30,31,30,31 };

uint8_t RtcControl::getDaysPerMonth(const uint8_t& month, const uint16_t& year)
{
  uint8_t days(pgm_read_byte(daysInMonth + month - 1));
  if (month == 2 && year%4 == 0) days++;
  return days;
}

uint8_t RtcControl::getDayOfTheWeek() const
{
  return mDayOfTheWeek;
}
uint16_t RtcControl::getYear() const
{
  return mYear;
}
uint8_t RtcControl::getMonth() const
{
  if (mDayLightSaving &&
      ((mMinutesSinceMidnight + 60) / MINUTES_PER_DAY) == 1 &&
      mDay == getDaysPerMonth(mMonth, mYear))
  {
    return mMonth + 1;
  }
  return mMonth;
}
uint8_t RtcControl::getDay() const
{
  if (mDayLightSaving &&
      ((mMinutesSinceMidnight + 60) / MINUTES_PER_DAY) == 1)
  {
    return (mDay == getDaysPerMonth(mMonth, mYear) ? 1 : mDay + 1);
  }
  else return mDay;
}
uint16_t RtcControl::getMinutesSinceMidnight() const
{
  if (mDayLightSaving)
  {
    return (mMinutesSinceMidnight + 60) % MINUTES_PER_DAY;
  }
  else return mMinutesSinceMidnight;
}

bool RtcControl::dayLightSaving() const
{
  return mDayLightSaving;
}

void RtcControl::updateNow()
{
  RTC_DS3231::now(mYear, mMonth, mDay, mMinutesSinceMidnight);    
  mDayOfTheWeek = dayOfTheWeek(mYear, getMonth(), getDay());
  // Day light saving time check
  if (mDayLightSaving == true &&
      getMonth() == 10 &&
      getDay() >= 25 &&
      getDayOfTheWeek() == 0 && // Starting at 0, so Sunday
      hours(getMinutesSinceMidnight()) == 3)
  {
    mDayLightSaving = false;
  }
  else if (mDayLightSaving== false &&
           getMonth() == 3 &&
           getDay() >= 25 &&
           getDayOfTheWeek() == 0 && // Starting at 0, so Sunday
           hours(getMinutesSinceMidnight()) == 2)
  {
    mDayLightSaving = true;
  }
}

uint8_t RtcControl::dayOfTheWeek(const uint16_t& year, const uint8_t& month, const uint8_t& day) 
{
  int adjustment, mm, yy;
  adjustment = (14 - month) / 12;
  mm = month + 12 * adjustment - 2;
  yy = year - adjustment;
  return (day + (13 * mm - 1) / 5 +  yy + yy / 4 - yy / 100 + yy / 400) % 7;
}

void RtcControl::setDateTime(const uint16_t& year, const uint8_t& month, const uint8_t& day, const uint16_t& minutesSinceMidnight)
{
  RTC_DS3231::adjust(year, month, day, (mDayLightSaving && minutesSinceMidnight > 60) ? minutesSinceMidnight - 60 : minutesSinceMidnight);
  updateNow();
  checkDayLightSaving();
}

void RtcControl::checkDayLightSaving()
{
  // Initialize daylightsaving
  if (getMonth() < 3 || getMonth() > 10)
  {
    mDayLightSaving = false; 
  }
  else if (getMonth() > 3 && getMonth() < 10)
  {
    mDayLightSaving = true; 
  }
  else
  {
    uint8_t previousSunday = getDay() - (dayOfTheWeek(mYear, getMonth(), getDay())+1);
    if (getMonth() == 3) 
    {
      mDayLightSaving = previousSunday >= 25;
    }
    else if (getMonth() == 10) 
    {
      mDayLightSaving = previousSunday < 25;
    }
  }  
}

////////////////////////////////////////////////////////////////////////////////
// RTC_DS3231 implementation

static uint8_t bcd2bin (uint8_t val) { return val - 6 * (val >> 4); }
static uint8_t bin2bcd (uint8_t val) { return val + 6 * (val / 10); }

static uint8_t read_i2c_register(uint8_t addr, uint8_t reg) {
  Wire.beginTransmission(addr);
  Wire.write((byte)reg);
  Wire.endTransmission();

  Wire.requestFrom(addr, (byte)1);
  return Wire.read();
}

static void write_i2c_register(uint8_t addr, uint8_t reg, uint8_t val) {
  Wire.beginTransmission(addr);
  Wire.write((byte)reg);
  Wire.write((byte)val);
  Wire.endTransmission();
}

bool RtcControl::RTC_DS3231::lostPower(void) {
  return (read_i2c_register(DS3231_ADDRESS, DS3231_STATUSREG) >> 7);
}

void RtcControl::RTC_DS3231::adjust(const uint16_t& year, const uint8_t& month, const uint8_t& day, const uint16_t& minutesSinceMidnight) {
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write((byte)0); // start at location 0
  Wire.write(bin2bcd(0)); // seconds
  Wire.write(bin2bcd(minutesSinceMidnight%MINUTES_PER_HOUR));
  Wire.write(bin2bcd(minutesSinceMidnight/MINUTES_PER_HOUR));
  Wire.write(bin2bcd(0));
  Wire.write(bin2bcd(day));
  Wire.write(bin2bcd(month));
  Wire.write(bin2bcd(year - 2000));
  Wire.endTransmission();

  uint8_t statreg = read_i2c_register(DS3231_ADDRESS, DS3231_STATUSREG);
  statreg &= ~0x80; // flip OSF bit
  write_i2c_register(DS3231_ADDRESS, DS3231_STATUSREG, statreg);
}

void RtcControl::RTC_DS3231::now(uint16_t& year, uint8_t& month, uint8_t& day, uint16_t& minutesSinceMidnight) {
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write((byte)0);  
  Wire.endTransmission();

  Wire.requestFrom(DS3231_ADDRESS, 7);
  Wire.read(); // seconds
  minutesSinceMidnight = bcd2bin(Wire.read()) + MINUTES_PER_HOUR * bcd2bin(Wire.read());
  Wire.read();
  day = bcd2bin(Wire.read());
  month = bcd2bin(Wire.read());
  year = bcd2bin(Wire.read()) + 2000;
}

} // Namspace
