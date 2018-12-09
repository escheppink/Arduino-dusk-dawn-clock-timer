#include <avr/wdt.h> // watchdog

#include "rtccontrol.h"
#include "oledcontrol.h"
#include "dusk2dawn.h"
#include "timer.h"
#include "rotaryencoder.h"

using namespace dusk_dawn_timer;

RtcControl rtcControl;
Dusk2Dawn dusk2dawn;
Timer timer = Timer(&rtcControl, &dusk2dawn);
OledControl oledControl(&rtcControl, &dusk2dawn, &timer);
RotaryEncoder rotary(&oledControl);

/*
 * Setup
 */
void setup() {
  rtcControl.begin();

  timer.begin();

  oledControl.begin();

  rotary.begin();
  
  // Watchdog
  wdt_enable(WDTO_1S);

  pinMode(13, OUTPUT);
  digitalWrite(13, LOW); // Switch off the annoying red led.
}

/*
 * Main loop
 */
void loop() {
  rtcControl.update();

  dusk2dawn.update(rtcControl.getYear(), rtcControl.getMonth(), rtcControl.getDay(), rtcControl.dayLightSaving());
 
  timer.update();    

  rotary.update();
  
  oledControl.updateMenu();
  
  delay(50);
  // Keep the watchdog happy
  wdt_reset();
}
