#pragma once

#include <Arduino.h>
#include "config.h"
#include "button_def.h"
#include "fifo.h"

// updated every loop, register of each surface button state
// of buttons defined in button_def.h
static uint16_t sbuttons[TOTAL_BUTTONS];

#if defined(CONFIG_ECHELON_1K)
#define SHIFT_HELD (sbuttons[BTN_PIG_LEFT] != 0 | sbuttons[BTN_PIG_RIGHT] != 0)
#else
#define SHIFT_HELD (sbuttons[BTN_SHIFT] != 0 |)
#endif

// Surface Button, press detection with debounce
class SButton
{
public:
  SButton(uint16_t sbutton_number, uint16_t debounceDelay = 0)
  {
    _sbutton = sbutton_number;
    _lastDebounceTime = _lastMillis = 0;
    _debounceDelay = debounceDelay;
    _state = _lastState = LOW;
  }

  bool check(bool triggerState);

private:
  uint16_t _sbutton;
  uint8_t _state, _lastState;
  uint16_t _debounceDelay;
  uint32_t _lastMillis, _lastDebounceTime;
};

// Check if button was pressed, with debouncing.
// Returns 1 if when buttons was pressed and then released
bool inline SButton::check(bool triggerState = 1)
{
  uint32_t ms = millis();
  uint8_t reading = sbuttons[_sbutton];

  if (reading != _lastState)
  { // Checks if the button has changed state
    _lastDebounceTime = ms;
  }
  if ((_debounceDelay == 0) | ((ms - _lastDebounceTime) > _debounceDelay))
  { // Checks if the buttons hasn't changed state for '_debounceDelay' milliseconds.
    if (reading != _state)
    { // Checks if the buttons has changed state
      _state = reading;
      return _state;
    }
  }
  _lastState = reading;
  if (triggerState == true)
  { // If this code is reached, it returns the normal state of the button.
    return false;
  }
  else
  {
    return true;
  }
}

class SKeyboard
{
public:
  void update();

  bool isKeyAvailable() { return (keyQueue.size() > 0); };
  uint16_t getKey() { return keyQueue.pop(); };
  void flush()
  {
    while (keyQueue.size())
      keyQueue.pop();
  };

#ifdef USB_KEYBOARD
  uint16_t _usb_key[TOTAL_BUTTONS] = {0};
#endif
private:
  uint16_t laststate[TOTAL_BUTTONS];
  FIFO keyQueue;
};

void SKeyboard::update()
{
  for (uint16_t cnt = BUTTONS_START; cnt < BUTTONS_END; cnt++)
  {

    if ((laststate[cnt] != 0) & (sbuttons[cnt] == 0))
    {
      keyQueue.push(0 - cnt); // released is negative
    }
    else if ((laststate[cnt] == 0) & (sbuttons[cnt] != 0))
    {
      keyQueue.push(cnt); // pressed is positive
    }

    if (laststate[cnt] != sbuttons[cnt])
    {
      if (_usb_key[cnt] > 0)
      { // if a key assigned
        if (laststate[cnt] == 0)
        {
#ifdef USB_KEYBOARD
          Keyboard.press(_usb_key[cnt]);
#endif
        }
        else
        {
#ifdef USB_KEYBOARD
          Keyboard.release(_usb_key[cnt]);
#endif
        }
      }
    }

    laststate[cnt] = sbuttons[cnt]; // update old state
  }
}
