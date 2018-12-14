/*
 * Rotary encoder input handling
 *  
 *  This class uses the basic code from the Interrupt-based Rotary Encoder Sketch
 *  by Simon Merrett, https://gist.github.com/Xplorer001/b7bff3744fa10647b185ab4043fbcc93
 */
#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

#include <Arduino.h>
#include "oledcontrol.h"

namespace dusk_dawn_timer {
  
class RotaryEncoder {
public:
  RotaryEncoder(OledControl* oled);
  void begin();
  void update();

private:
  OledControl* mOled;
  uint8_t mButtonState;
  unsigned long mLastButtonChangeTime;
};

} // namespace
#endif  // ROTARY_ENCODER_H
