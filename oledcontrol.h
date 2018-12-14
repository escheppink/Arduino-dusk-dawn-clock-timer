/*
 * OLED display + menu handling
 */

#ifndef OLEDCONTROL_H
#define OLEDCONTROL_H

#include "SSD1306Ascii.h"
#include "SSD1306AsciiSoftSpi.h"

#include "rtccontrol.h"
#include "dusk2dawn.h"
#include "timer.h"

namespace dusk_dawn_timer {

#define evNONE 0
#define evPRESS 1
#define evLONGPRESS 2
#define evLEFT 3
#define evRIGHT 4

class OledControl {
public:
  OledControl(RtcControl* rtc, Dusk2Dawn* d2d, Timer* timer);
  void begin();
  void userEvent(uint8_t event);
  void updateMenu(bool forceUpdate = false);

 private:
  String twoDigitString(const int16_t& value);
  String timeString(const uint16_t& hour, const uint16_t& minute);
  String timeString(const uint16_t& minutesSinceMidnight);
  void printSelectable(bool selected, class __FlashStringHelper* line) const;
  void timerTime(const int16_t& type, const int16_t& time, int16_t& data1, int16_t& data2);
  int16_t timerTime(const int16_t& type, const int16_t& data1, const int16_t& data2);
  void printTimerType(const int16_t& type);
  void printTimerTime1(const int16_t& type, const int16_t& data2);
  void printTimerTime2(const int16_t& type, const int16_t& data2);
  
  SSD1306AsciiSoftSpi mOled;
  RtcControl* mRealTimeClock;
  Dusk2Dawn* mD2d;
  Timer* mTimer;

  uint8_t mCurrentScreen;
  uint8_t mEvent = evNONE;  
  
  bool notMaxValue();
  uint8_t mMenuOption = 0;
  int16_t mMenuData[6];
  byte mSelection;
};

} // namespace
#endif // OLEDCONTROL_H
