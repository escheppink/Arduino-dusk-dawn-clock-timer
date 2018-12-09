/*
 * Rotary encoder input handling
 *  
 *  This class uses the basic code from the Interrupt-based Rotary Encoder Sketch
 *  by Simon Merrett, https://gist.github.com/Xplorer001/b7bff3744fa10647b185ab4043fbcc93
 */
#include "rotaryencoder.h"

namespace dusk_dawn_timer {
    
// ISR and class do not work together;
// Use static code for ISR handling.
// Note that this implies that there can only be one instance of the class.

#define PIN_RIGHT 2 // Our first hardware interrupt pin is digital pin 2
#define PIN_LEFT 3 // Our second hardware interrupt pin is digital pin 3
#define BUTTON_PIN 4 // Button pushed switch pin

#define BUTTON_NONE 0
#define BUTTON_PRESSED 1
#define AWAIT_BUTTON_RELEASE 2

volatile byte aFlag = 0; // let's us know when we're expecting a rising edge on pinA to signal that the encoder has arrived at a detent
volatile byte bFlag = 0; // let's us know when we're expecting a rising edge on pinB to signal that the encoder has arrived at a detent (opposite direction to when aFlag is set)
volatile uint8_t encoderEvent = evNONE; 
#define TIME_UPDATE_DELAY 500  // Do not read the buttons to often
#define LONG_PRESS_TIME 2500

void PinA(){
  cli(); //stop interrupts happening before we read pin values
  byte reading = PIND & 0xC; // read all eight pin values then strip away all but pinA and pinB's values
  if(reading == B00001100 && aFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoderEvent = evRIGHT;
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00000100) bFlag = 1; //signal that we're expecting pinB to signal the transition to detent from free rotation
  sei(); //restart interrupts
}

void PinB(){
  cli(); //stop interrupts happening before we read pin values
  byte reading = PIND & 0xC; //read all eight pin values then strip away all but pinA and pinB's values
  if (reading == B00001100 && bFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoderEvent = evLEFT;
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00001000) aFlag = 1; //signal that we're expecting pinA to signal the transition to detent from free rotation
  sei(); //restart interrupts
}

RotaryEncoder::RotaryEncoder(OledControl* oled)
  : mOled(oled)
  , mButtonState(BUTTON_NONE)
{}

void RotaryEncoder::begin() {
  pinMode(PIN_RIGHT, INPUT_PULLUP); // set pinA as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  pinMode(PIN_LEFT, INPUT_PULLUP); // set pinB as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  attachInterrupt(0,PinA,RISING); // set an interrupt on PinA, looking for a rising edge signal and executing the "PinA" Interrupt Service Routine (below)
  attachInterrupt(1,PinB,RISING); // set an interrupt on PinB, looking for a rising edge signal and executing the "PinB" Interrupt Service Routine (below)
  pinMode(BUTTON_PIN, INPUT_PULLUP); // set pinB as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
}

void RotaryEncoder::update() {
  bool buttonPressed = digitalRead(BUTTON_PIN) == LOW;
  unsigned long now = millis();
 
  if (buttonPressed && mButtonState == BUTTON_NONE) // button pressed
  {
    if (now - mLastButtonChangeTime >= TIME_UPDATE_DELAY)
    {    
      mButtonState = BUTTON_PRESSED;
      mLastButtonChangeTime = now;
    }
  }
  else if (!buttonPressed && mButtonState != BUTTON_NONE) // Button released
  {
    mLastButtonChangeTime = now;
    bool pressed = (mButtonState == BUTTON_PRESSED);
    mButtonState = BUTTON_NONE;
    if (pressed) mOled->userEvent(evPRESS);
  }  
  else if (buttonPressed && mButtonState == BUTTON_PRESSED && // Button still pressed
           now - mLastButtonChangeTime >= LONG_PRESS_TIME)
  {
    mButtonState = AWAIT_BUTTON_RELEASE;
    mOled->userEvent(evLONGPRESS);
  }
  else if (encoderEvent != evNONE)
  {
    mOled->userEvent(encoderEvent);
  }
  encoderEvent = evNONE;
}

} // namespace
