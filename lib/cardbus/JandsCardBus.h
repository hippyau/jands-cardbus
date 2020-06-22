#pragma once
/*
     ██╗ █████╗ ███╗   ██╗██████╗ ███████╗     ██████╗ █████╗ ██████╗ ██████╗     ██████╗ ██╗   ██╗███████╗
     ██║██╔══██╗████╗  ██║██╔══██╗██╔════╝    ██╔════╝██╔══██╗██╔══██╗██╔══██╗    ██╔══██╗██║   ██║██╔════╝
     ██║███████║██╔██╗ ██║██║  ██║███████╗    ██║     ███████║██████╔╝██║  ██║    ██████╔╝██║   ██║███████╗
██   ██║██╔══██║██║╚██╗██║██║  ██║╚════██║    ██║     ██╔══██║██╔══██╗██║  ██║    ██╔══██╗██║   ██║╚════██║
╚█████╔╝██║  ██║██║ ╚████║██████╔╝███████║    ╚██████╗██║  ██║██║  ██║██████╔╝    ██████╔╝╚██████╔╝███████║
 ╚════╝ ╚═╝  ╚═╝╚═╝  ╚═══╝╚═════╝ ╚══════╝     ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═════╝     ╚═════╝  ╚═════╝ ╚══════╝
*/


//#define TESTING 
//#define LCD_TESTING
//#define FADER_TESTING
//#define BUTTON_TESTING


// bus driver
#include "hardware.h"
// components
#include "surface_buttons.h"
#include "lcd_module.h"
#include "keypad_input.h"
// individual card drivers
#include "card_preset.h"
#include "card_palette.h"
#include "card_assign.h"
#include "card_master.h"

// while we update buttons and LEDs every loop, only poll faders only every N milliseconds
#define FADER_POLL_SPEED  10  


// the whole control surface
class JandsCardBus
{
public:
  presetCard preset1;
  presetCard preset2;
  paletteCard palette;
  assignCard assign;
  masterCard master;

  SKeyboard keys;  

  bool update(); // update all cards
  void send();  // send desk state 
};


// Update the control surface elements
// Returns: true on change detected
bool inline JandsCardBus::update()
{  
  static unsigned long lastMillis;  
  unsigned long now = millis();
  bool check_faders_now = 1;

  if ((now - lastMillis) >= FADER_POLL_SPEED) {  // poll faders on occasion, every 6ms
    check_faders_now = 1;  
    lastMillis = now;
  }

  uint8_t fc = 0;
  fc += preset1.update(check_faders_now);
  fc += preset2.update(check_faders_now);
  fc += assign.update(check_faders_now);
  fc += master.update(check_faders_now);
  fc += palette.update();
  
  if (fc) {
    // pull buttons from individual bits out to an array

    uint8_t i = 0, b = 0;
    
    // preset card 1 buttons 1-12
    for ( b = 0 ; b < 8 ; b++) // first byte
     sbuttons[i+b] = preset1.buttons[0] & (1 << b);
    i+=b;
    for ( b = 0 ; b < 4 ; b++) // second byte, first nibble
     sbuttons[i+b] = preset1.buttons[1] & (1 << b);
    i+=b;

    // preset card 2 buttons 1-12
    for ( b = 0 ; b < 8 ; b++) // first byte
     sbuttons[i+b] = preset2.buttons[0] & (1 << b);
    i+=b;
    for ( b = 0 ; b < 4 ; b++) // second byte, first nibble
     sbuttons[i+b] = preset2.buttons[1] & (1 << b);
    i+=b;
      
    // assign card buttons 1-8
    for ( b = 0 ; b < 8 ; b++) // first byte
     sbuttons[i+b] = assign.buttons[0] & (1 << b);
    i+=b;
   
    // palette card buttons 1-20
    for ( b = 0 ; b < 8 ; b++) // first byte
     sbuttons[i+b] = palette.buttons[0] & (1 << b);
    i+=b;
    for ( b = 0 ; b < 8 ; b++) // second byte
     sbuttons[i+b] = palette.buttons[1] & (1 << b);
    i+=b;
    for ( b = 0 ; b < 4 ; b++) // third byte, first nibble
     sbuttons[i+b] = palette.buttons[2] & (1 << b);
    i+=b;

  // master card buttons 1-37
    for ( b = 0 ; b < 8 ; b++) // first byte
     sbuttons[i+b] = master.buttons[0] & (1 << b);
    i+=b;
    for ( b = 0 ; b < 8 ; b++) // second byte
     sbuttons[i+b] = master.buttons[1] & (1 << b);
    i+=b;
    for ( b = 0 ; b < 8 ; b++) // third byte
     sbuttons[i+b] = master.buttons[2] & (1 << b);
    i+=b;
    for ( b = 0 ; b < 8 ; b++) // fourth byte
     sbuttons[i+b] = master.buttons[3] & (1 << b);
    i+=b;
    for ( b = 0 ; b < 8 ; b++) // fifth byte 32-37
     sbuttons[i+b] = master.buttons[4] & (1 << b);
    i+=b;
  }

   // update the keyboard queue
   keys.update();

   check_faders_now = 0;

   return ((fc != 0) ? true : false);

}


 



