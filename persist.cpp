/*
 * Persistent data (EEPROM) abstraction
 */
#include "persist.h"
#include <EEPROM.h>

namespace dusk_dawn_timer {

// Defines for EEPROM memory addresses used to store the data
#define SCREEN_BLANK_TIMEOUT 1

#define WEEK_TIMER1_START_TYPE 10
#define WEEK_TIMER1_START_TIME 11  // 2 byte -> Also occupies 12
#define WEEK_TIMER1_STOP_TYPE 20
#define WEEK_TIMER1_STOP_TIME 21  // 2 byte -> Also occupies 22

#define WEEKEND_TIMER1_START_TYPE 30
#define WEEKEND_TIMER1_START_TIME 31 // 2 byte -> Also occupies 32
#define WEEKEND_TIMER1_STOP_TYPE 40
#define WEEKEND_TIMER1_STOP_TIME 41 // 2 byte -> Also occupies 42

void Persist::clearmem()
{
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }  
}

void Persist::setScreenBlankTimeout(const uint8_t& timeout)
{
    EEPROM.write(SCREEN_BLANK_TIMEOUT, timeout);
}
uint8_t Persist::getScreenBlankTimeout()
{
  return EEPROM.read(SCREEN_BLANK_TIMEOUT);
}

void Persist::setWeekTimer(const uint8_t& start_type, const int16_t& start_time, const uint8_t& stop_type, const int16_t& stop_time)
{
  EEPROM.write(WEEK_TIMER1_START_TYPE, start_type);
  write16(WEEK_TIMER1_START_TIME, start_time);
  EEPROM.write(WEEK_TIMER1_STOP_TYPE, stop_type);
  write16(WEEK_TIMER1_STOP_TIME, stop_time);
}
void Persist::getWeekTimer(uint8_t& start_type, int16_t& start_time, uint8_t& stop_type, int16_t& stop_time)
{
  start_type = EEPROM.read(WEEK_TIMER1_START_TYPE);
  start_time = read16(WEEK_TIMER1_START_TIME);
  stop_type = EEPROM.read(WEEK_TIMER1_STOP_TYPE);
  stop_time = read16(WEEK_TIMER1_STOP_TIME);
}
void Persist::setWeekendTimer(const uint8_t& start_type, const int16_t& start_time, const uint8_t& stop_type, const int16_t& stop_time)
{
  EEPROM.write(WEEKEND_TIMER1_START_TYPE, start_type);
  write16(WEEKEND_TIMER1_START_TIME, start_time);
  EEPROM.write(WEEKEND_TIMER1_STOP_TYPE, stop_type);
  write16(WEEKEND_TIMER1_STOP_TIME, stop_time);
}
void Persist::getWeekendTimer(uint8_t& start_type, int16_t& start_time, uint8_t& stop_type, int16_t& stop_time)
{
  start_type = EEPROM.read(WEEKEND_TIMER1_START_TYPE);
  start_time = read16(WEEKEND_TIMER1_START_TIME);
  stop_type = EEPROM.read(WEEKEND_TIMER1_STOP_TYPE);
  stop_time = read16(WEEKEND_TIMER1_STOP_TIME);
}

void Persist::write16(int address, const int16_t& value)
{
  EEPROM.write(address, value & 0xFF);
  EEPROM.write(address + 1, (value >> 8) & 0xFF);
}

int16_t Persist::read16(int address)
{
  int16_t result = 0;
  result = EEPROM.read(address+1);
  result = result << 8;
  result += EEPROM.read(address);
  return result;
}


} // Namespace
