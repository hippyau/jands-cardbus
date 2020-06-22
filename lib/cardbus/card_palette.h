#pragma once

#include <stdint.h>
#include "hardware.h"



/*
██████╗  █████╗ ██╗     ███████╗████████╗████████╗███████╗     ██████╗ █████╗ ██████╗ ██████╗ 
██╔══██╗██╔══██╗██║     ██╔════╝╚══██╔══╝╚══██╔══╝██╔════╝    ██╔════╝██╔══██╗██╔══██╗██╔══██╗
██████╔╝███████║██║     █████╗     ██║      ██║   █████╗      ██║     ███████║██████╔╝██║  ██║
██╔═══╝ ██╔══██║██║     ██╔══╝     ██║      ██║   ██╔══╝      ██║     ██╔══██║██╔══██╗██║  ██║
██║     ██║  ██║███████╗███████╗   ██║      ██║   ███████╗    ╚██████╗██║  ██║██║  ██║██████╔╝
╚═╝     ╚═╝  ╚═╝╚══════╝╚══════╝   ╚═╝      ╚═╝   ╚══════╝     ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═════╝                                                                                                                                            
*/

/* 
PaletteCard is 20 buttons and 20 red LEDS, spread across 3 bytes, with CARD ID at last nibble
The most boring of all the cards....
*/
class paletteCard
{
public:
  bool update(); 

  // input 
  uint8_t leds[3];

  // output 
  uint8_t buttons[3];

  paletteCard()
  {
    card_addr = ADDR_PALETTE; // default base address for a palette card, can change latter
    leds[0] = leds[1] = 0;
    buttons[0] = buttons[1] = buttons[2] = 0;
    old_buttons[0] = old_buttons[1] = old_buttons[2] = 0;
  }

  void setCardAddress(uint8_t addr)
  {
    card_addr = addr;
  };


private:
  uint8_t card_addr;

  uint8_t old_buttons[3];  // for change comparisons...
};


// Palette Card Driver
// Returns: true on change detected
bool paletteCard::update()
{
  bool fc = 0; // flag a change has occured

  // process three banks of leds & buttons
  for (uint8_t cnt = 0; cnt < 3; cnt++)
  {
    selectAddr(card_addr + cnt);
    writeData(leds[cnt]);
    buttons[cnt] = readData();
  }

  // check for changes
  for (uint8_t cnt = 0; cnt < 3; cnt++)
  {
    if (buttons[cnt] != old_buttons[cnt])
    {
      fc = true;
      old_buttons[cnt] = buttons[cnt];

#if defined(BUTTON_TESTING)
      leds[cnt] = buttons[cnt]; /* feed the button data back to the LEDs */
      Serial.printf("PB_%d=0x%x\n", cnt, buttons[cnt]);
#endif
    }
  }
  return fc;
}
