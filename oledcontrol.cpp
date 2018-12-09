/*
 * OLED display + menu handling
 */
#include "oledcontrol.h"
#include "rotaryencoder.h"
#include "dusk2dawn.h"
#include "persist.h"

// Using software SPI
// pin definitions
#define CS_PIN   12
#define RST_PIN  8
#define DC_PIN   11
#define MOSI_PIN  9
#define CLK_PIN  10

#define SCREEN_TIMEOUT 60000 // 1 minute

#define NONE_SCREEN 0
#define BLANK_SCREEN 1
#define DEFAULT_SCREEN 2
#define MENU_SCREEN 3
#define SET_TIME_SCREEN 4
#define SET_TIMER_SCREEN 5
#define SET_OPTIONS 6

#define MENU_OPTION_WEEK_TIMER 1
#define MENU_OPTION_WEEKEND_TIMER 2

namespace dusk_dawn_timer {
  
OledControl::OledControl(RtcControl* rtc, Dusk2Dawn* d2d, Timer* timer)
:   mRealTimeClock(rtc),
    mD2d(d2d),
    mTimer(timer),
    mCurrentScreen(DEFAULT_SCREEN)
{ }

void OledControl::begin()
{
  mOled.begin(&Adafruit128x64, CS_PIN, DC_PIN, CLK_PIN, MOSI_PIN, RST_PIN);
  mOled.setFont(System5x7);
  updateMenu(true);
}

void OledControl::userEvent(uint8_t event)
{
  mEvent = event;
}

static const char SU[] PROGMEM = "Su";
static const char MO[] PROGMEM = "Mo";
static const char TU[] PROGMEM = "Tu";
static const char WE[] PROGMEM = "We";
static const char TH[] PROGMEM = "Th";
static const char FR[] PROGMEM = "Fr";
static const char SA[] PROGMEM = "Sa";

const char* const sDaysOfTheWeek[] PROGMEM = {SU, MO, TU, WE, TH, FR, SA};

static const char TI[] PROGMEM = "Time ";
static const char SUP[] PROGMEM = "Dawn ";
static const char SDOWN[] PROGMEM = "Dusk ";

const char* const sTimerTypes[] PROGMEM = {TI, SUP, SDOWN};
  
void OledControl::updateMenu(bool forceupdate) {
  unsigned long now = millis();
  uint8_t newscreen = NONE_SCREEN;
  uint16_t minutesSinceMidnight = mRealTimeClock->getMinutesSinceMidnight();
  if (mCurrentScreen != DEFAULT_SCREEN && mEvent == evLONGPRESS) 
  {
    newscreen = DEFAULT_SCREEN;
  }
  else
  {
    switch (mCurrentScreen)
    {
      case BLANK_SCREEN:
      {
        // Any event switched screen on
        if (mEvent != evNONE) newscreen = DEFAULT_SCREEN;
        break;
      }
      case DEFAULT_SCREEN:
      {
        if (forceupdate)
        {
          mMenuData[0] = minutesSinceMidnight;
          mMenuData[1] = Persist::getScreenBlankTimeout();
        }
        if (mEvent == evNONE)
        {
            const uint8_t dayOfTheWeek = mRealTimeClock->getDayOfTheWeek();
            mOled.home();
            for (uint8_t i = 0; i<7; ++i)
            {
              mOled.setCol(i * 18);
              if (i == dayOfTheWeek)
              {                
                char day[3];
                strcpy_P(day, (char*) pgm_read_word( &sDaysOfTheWeek[dayOfTheWeek] ) );
                mOled.print(day);                
              }
              else
              {
                mOled.print(F("  "));
              }
            }
            mOled.print(F("\n"));        
            mOled.println();
            mOled.set2X();
            mOled.setCol(34);
            mOled.println(timeString(minutesSinceMidnight) );
            mOled.set1X();
            mOled.println();
            if (mTimer->isSwitchedManual())
            {
              mOled.setInvertMode(true);
              mOled.setCol(2);
              mOled.print(F("Manual"));
              mOled.setInvertMode(false);
            }
            else
            {
              mOled.setCol(2);
              mOled.print(F(" Timer "));
            }
            mOled.setCol(42);
            mOled.print(mTimer->isSwitchedOn() ? F("ON ") : F("OFF"));
            mOled.setCol(66);
            mOled.print(F("until"));
            mOled.setCol(99);
            mOled.println(timeString(mTimer->getNextSwitchTime()));
            mOled.println();
            printTimerType(1); // Dawn
            mOled.setCol(28);
            mOled.print(timeString(mD2d->mSunrise) );
            mOled.setCol(70);
            printTimerType(2); // Dusk
            mOled.setCol(99);            
            mOled.println(timeString(mD2d->mSunset) );
            if (mMenuData[1] != 0 &&
                ((mMenuData[0] < minutesSinceMidnight && (minutesSinceMidnight - mMenuData[0] >= mMenuData[1])) ||
                 (mMenuData[0] > minutesSinceMidnight && ((MINUTES_PER_DAY - mMenuData[0] + minutesSinceMidnight) >= mMenuData[1]))))
            {
              newscreen = BLANK_SCREEN;
            }
        }
        else if (mEvent == evLONGPRESS)
        {
          newscreen = MENU_SCREEN;
        }
        else if (mEvent == evPRESS)
        {
          mTimer->manualSwitch();
          mMenuData[0] = minutesSinceMidnight;
        }
        else
        {
          mMenuData[0] = minutesSinceMidnight;
        }
        break;
      }
      case MENU_SCREEN:
      {
        if (forceupdate)
        {
          mSelection = 0;
          mEvent = evNONE;
        }
        if (mEvent == evPRESS)
        {
          if (mSelection == 0) newscreen = DEFAULT_SCREEN;
          else if (mSelection == 1) newscreen = SET_TIME_SCREEN;
          else if (mSelection == 2)
          {
            mMenuOption = MENU_OPTION_WEEK_TIMER;
            newscreen = SET_TIMER_SCREEN;
          }
          else if (mSelection == 3)
          {
            mMenuOption = MENU_OPTION_WEEKEND_TIMER;
            newscreen = SET_TIMER_SCREEN;
          }
          else if (mSelection == 4) newscreen = SET_OPTIONS;
        }
        else if (mEvent == evLEFT && mSelection>0)
        {
          mSelection--;
        }
        else if (mEvent == evRIGHT && mSelection<4)
        {
          mSelection++;
        }
        mOled.home();
        mOled.println();
        printSelectable(mSelection == 0, F("Back"));
        mOled.println();
        printSelectable(mSelection == 1, F("Set time"));
        printSelectable(mSelection == 2, F("Week day program"));
        printSelectable(mSelection == 3, F("Weekend program"));
        mOled.println();
        printSelectable(mSelection == 4, F("Options"));
        break;
      }
      case SET_TIME_SCREEN:
      {
        if (forceupdate)
        {
            mSelection = 0;
            mMenuData[0] = mRealTimeClock->getYear();
            mMenuData[1] = mRealTimeClock->getMonth();
            mMenuData[2] = mRealTimeClock->getDay();
            mMenuData[3] = RtcControl::hours(minutesSinceMidnight);
            mMenuData[4] = RtcControl::minutes(minutesSinceMidnight);
        }
        if (mEvent == evPRESS)
        {
          if (mSelection < 4) mSelection++; // Next step;
          else 
          {
            // All set, update timer
            mRealTimeClock->setDateTime(mMenuData[0], mMenuData[1], mMenuData[2], mMenuData[3] * MINUTES_PER_HOUR + mMenuData[4]);
            newscreen = MENU_SCREEN;
          }
        }
        else if (mEvent == evLEFT && mMenuData[mSelection] > (mSelection < 3 ? 1 : 0))
        {
          mMenuData[mSelection]--;
        }
        else if (mEvent == evRIGHT && notMaxValue())
        {
          mMenuData[mSelection]++;
        }
        mOled.home();
        mOled.print(F("Year:    "));
        mOled.println(String(mMenuData[0]));
        if (mSelection > 0)
        {
          mOled.print(F("Month:    "));
          mOled.print(mMenuData[1] < 10 ? " " : "");
          mOled.println(String(mMenuData[1]));
        }
        if (mSelection > 1) 
        {
          mOled.print(F("Day:      "));
          mOled.print(mMenuData[2] < 10 ? "0" : "");
          mOled.println(String(mMenuData[2]));
        }
        if (mSelection > 2)
        {
          mOled.println();
          mOled.print(F("Time:  "));
          mOled.print(twoDigitString(mMenuData[3]));
        }
        if (mSelection > 3) 
        {
          mOled.print(F(" : "));
          mOled.print(twoDigitString(mMenuData[4]));
        }
        break;
      }
      case SET_TIMER_SCREEN:
      {
        if (forceupdate)
        {
            mSelection = 0;
            if (mMenuOption == MENU_OPTION_WEEK_TIMER)
            {
              mMenuData[0] = mTimer->getWeekDayOnType();
              timerTime(mMenuData[0], mTimer->getWeekDayOnTime(), mMenuData[1], mMenuData[2]);
              mMenuData[3] = mTimer->getWeekDayOffType();
              timerTime(mMenuData[3], mTimer->getWeekDayOffTime(), mMenuData[4], mMenuData[5]);
            }
            else
            {
              mMenuData[0] = mTimer->getWeekendOnType();
              timerTime(mMenuData[0], mTimer->getWeekendOnTime(), mMenuData[1], mMenuData[2]);
              mMenuData[3] = mTimer->getWeekendOffType();
              timerTime(mMenuData[3], mTimer->getWeekendOffTime(), mMenuData[4], mMenuData[5]);
            }
        }
        if (mEvent == evPRESS)
        {
          bool done = false;
          if (mSelection < 5)
          {
            mSelection++; // Next step;
            if ((mSelection == 2 || mSelection == 4) && 
                 mMenuData[mSelection - 2] != TIME) 
            {
              mSelection++; // skip unused variable
              done = mSelection >= 5;
            }
          }
          else
          {
            done = true;
          }
          if (done)
          {
            // All set, update timer
            if (mMenuOption == MENU_OPTION_WEEK_TIMER)
            {
              mTimer->setWeekTimer(mMenuData[0], timerTime(mMenuData[0], mMenuData[1], mMenuData[2]), mMenuData[3], timerTime(mMenuData[3], mMenuData[4], mMenuData[5]));
            }
            else
            {
              mTimer->setWeekendTimer(mMenuData[0], timerTime(mMenuData[0], mMenuData[1], mMenuData[2]), mMenuData[3], timerTime(mMenuData[3], mMenuData[4], mMenuData[5]));              
            }
            newscreen = MENU_SCREEN;
          }
        }
        else if (mEvent == evLEFT)
        {
          if (((mSelection == 0 || mSelection == 3) && mMenuData[mSelection] > 0) ||
              ((mSelection == 1 || mSelection == 4) && mMenuData[mSelection - 1] != TIME && mMenuData[mSelection] > -59) ||
              ((mSelection == 1 || mSelection == 4) && mMenuData[mSelection - 1] == TIME && mMenuData[mSelection] > 0) ||
              ((mSelection == 2 || mSelection == 5) && mMenuData[mSelection] > 0))
          {
            mMenuData[mSelection]--;
          }
          
        }
        else if (mEvent == evRIGHT)
        {
          if (((mSelection == 0 || mSelection == 3) && mMenuData[mSelection] < 2) ||
              ((mSelection == 1 || mSelection == 4) && mMenuData[mSelection - 1] != TIME && mMenuData[mSelection] < 59) ||
              ((mSelection == 1 || mSelection == 4) && mMenuData[mSelection - 1] == TIME && mMenuData[mSelection] < 23) ||
              ((mSelection == 2 || mSelection == 5) && mMenuData[mSelection] < 59))
          {
            mMenuData[mSelection]++;
          }
        }
        mOled.home();
        uint8_t begin = (mMenuOption == MENU_OPTION_WEEK_TIMER) ? 0 : 5;
        uint8_t end = (mMenuOption == MENU_OPTION_WEEK_TIMER) ? 4 : 6;
        for (int i = begin; i <= end; ++i)
        {
            mOled.setCol(i * 18);
            char day[3];
            strcpy_P(day, (char*) pgm_read_word( &sDaysOfTheWeek[i] ) );
            mOled.print(day);
        }
        mOled.println();
        mOled.println();
        mOled.print(F("Switch at:  "));
        printTimerType(mMenuData[0]);
        if (mSelection > 0)
        {
          mOled.println();
          mOled.print(F("Time on:    "));
          printTimerTime1(mMenuData[0], mMenuData[1]);
        }
        if (mSelection > 1) 
        {
          printTimerTime2(mMenuData[0], mMenuData[2]);
        }
        if (mSelection > 2)
        {
          mOled.println();
          mOled.println();
          mOled.print(F("Switch off: "));
          printTimerType(mMenuData[3]);
        }
        if (mSelection > 3)
        {
          mOled.println();
          mOled.print(F("Time off:   "));
          printTimerTime1(mMenuData[3], mMenuData[4]);
        }
        if (mSelection > 4) 
        {
          printTimerTime2(mMenuData[3], mMenuData[5]);
        }
        break;
      }
      case SET_OPTIONS:
      {
        if (forceupdate)
        {
          mSelection = Persist::getScreenBlankTimeout();
        }
        if (mEvent == evPRESS)
        {
          Persist::setScreenBlankTimeout(mSelection);
          newscreen = MENU_SCREEN;
        }
        else if (mEvent == evLEFT && mSelection>0)
        {
          mSelection--;
        }
        else if (mEvent == evRIGHT && mSelection<30)
        {
          mSelection++;
        }
        mOled.home();
        mOled.println();
        mOled.print(F("Screen timout: "));
        if (mSelection == 0) mOled.print(F("Never "));
        else
        {
          mOled.print(twoDigitString(mSelection) );
          mOled.print(F(" min"));
        }
        break; 
      }
    };
  }
  mEvent = evNONE;
  if (newscreen != NONE_SCREEN)
  {
    mOled.clear();
    mCurrentScreen = newscreen;
    updateMenu(true);
  }
}

bool OledControl::notMaxValue()
{
  switch (mSelection)
  {
    case 0: return mMenuData[mSelection] < 2070; // Restriction in RTC
    case 1: return mMenuData[mSelection] < 12; // Months
    case 2: return mMenuData[mSelection] < RtcControl::getDaysPerMonth(mMenuData[1], mMenuData[0]);
    case 3: return mMenuData[mSelection] < 23; // Hours
    case 4: return mMenuData[mSelection] < MINUTES_PER_HOUR-1; // Minutes, counting from 0
  }
}  

String OledControl::twoDigitString(const int16_t& value)
{
  if (value<10 && value > -10) return String("0") + String(value);
  return String(value);
}

String OledControl::timeString(const uint16_t& hour, const uint16_t&  minute)
{
  return twoDigitString(hour) + F(":") + twoDigitString(minute);
}

String OledControl::timeString(const uint16_t& minutesSinceMidnight)
{
  return timeString(RtcControl::hours(minutesSinceMidnight), RtcControl::minutes(minutesSinceMidnight));
}

void OledControl::printSelectable(bool selected, class __FlashStringHelper * line) const
{
    mOled.print(selected ? ">" : " ");
    mOled.println(line);
}

void OledControl::timerTime(const int16_t& type, const int16_t& time, int16_t& data1, int16_t& data2)
{
  if (type != TIME)
  {
    data1 = time;
  }
  else
  {
    data1 = RtcControl::hours(time);
    data2 = RtcControl::minutes(time);
  }
}

int16_t OledControl::timerTime(const int16_t& type, const int16_t& data1, const int16_t& data2)
{
  if (type != TIME)
  {
    return data1;
  }
  else
  {
    return data1 * MINUTES_PER_HOUR + data2;
  }
}

void OledControl::printTimerType(const int16_t& type)
{
  char timerType[8];
  strcpy_P(timerType, (char*) pgm_read_word( &sTimerTypes[type] ) );
  mOled.print(timerType);
}

void OledControl::printTimerTime1(const int16_t& type, const int16_t& data1)
{
  if (type != TIME)
  {
    mOled.print(data1 >= 0 ? "+" : "-");
    mOled.print(twoDigitString(data1 >= 0 ? data1 : data1*-1));
  }
  else
  {
    mOled.print(twoDigitString(data1));
  }
}

void OledControl::printTimerTime2(const int16_t& type, const int16_t& data2)
{
  if (type != TIME) return;
  mOled.print(F(":"));
  mOled.print(twoDigitString(data2));
}
  
} // namespace
