/*
 * Persistent data (EEPROM) abstraction
 */
#ifndef PERSIST_H
#define PERSIST_H

#include "Arduino.h"

namespace dusk_dawn_timer {
  
class Persist {
public:
  static void clearmem();
  static void setScreenBlankTimeout(const uint8_t& timeout);
  static uint8_t getScreenBlankTimeout();
  static void setWeekTimer(const uint8_t& start_type, const int16_t& start_time, const uint8_t& stop_type, const int16_t& stop_time);  
  static void getWeekTimer(uint8_t& start_type, int16_t& start_time, uint8_t& stop_type, int16_t& stop_time);  
  static void setWeekendTimer(const uint8_t& start_type, const int16_t& start_time, const uint8_t& stop_type, const int16_t& stop_time);  
  static void getWeekendTimer(uint8_t& start_type, int16_t& start_time, uint8_t& stop_type, int16_t& stop_time);  
private:
  static void write16(int address, const int16_t& value);
  static int16_t read16(int address);
};

} // Namespace
#endif //PERSIST_H
