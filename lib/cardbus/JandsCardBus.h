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

/* while we update buttons and LEDs every loop, only poll faders only every N milliseconds */
#define FADER_POLL_SPEED  25  

/* ignore 0->1 transitions and changes less than 2 difference */
#define FADER_FILTERING  // filtering fader values 

/* when reading faders, takes 3 samples and average them for final fader values
   not entirely required if filtering is being done, but helps with jitter and noise */
#define FADER_AVERAGING  
#define FADER_AVERAGING_DELAY  10  // delay in uS between samples - shorter the better



// card addresses
#define ADDR_PRESET_1 (0x00)
#define ADDR_PRESET_2 (0x10)
#define ADDR_MASTER   (0x80)
#define ADDR_PALETTE  (0x90)
#define ADDR_ASSIGN   (0xC0)


// hardware bus driver routines
#include "hardware.h"

// components
#include "surface_buttons.h"
#include "lcd_module.h"
#include "button_def.h"
#include "keypad_input.h"

// individual card drivers
#include "card_preset.h"
#include "card_palette.h"
#include "card_assign.h"
#include "card_master.h"
#include "card_program_1k.h"



// the whole control surface
class JandsCardBus
{
public:
  presetCard preset1;
  presetCard preset2;
  paletteCard palette;
  assignCard assign;
  masterCard master;
  programCard program;

  SKeyboard keys;  

  bool update(); // update all cards  
};



// pointer to our entire control surface
static JandsCardBus * Surface = NULL;



/* Implementation */

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
  fc += program.update();
   
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

 // program card buttons 1-76
    for ( b = 0 ; b < 8 ; b++) // first byte
     sbuttons[i+b] = program.buttons[0] & (1 << b);
    i+=b;
    for ( b = 0 ; b < 8 ; b++) // second byte
     sbuttons[i+b] = program.buttons[1] & (1 << b);
    i+=b;
    for ( b = 0 ; b < 8 ; b++) // third byte
     sbuttons[i+b] = program.buttons[2] & (1 << b);
    i+=b;
    for ( b = 0 ; b < 8 ; b++) // fourth byte
     sbuttons[i+b] = program.buttons[3] & (1 << b);
    i+=b;
    for ( b = 0 ; b < 8 ; b++) // fifth byte 32-37
     sbuttons[i+b] = program.buttons[4] & (1 << b);
    i+=b;
    for ( b = 0 ; b < 8 ; b++) // sixth byte 38 - 45
     sbuttons[i+b] = program.buttons[5] & (1 << b);
    i+=b;
    for ( b = 0 ; b < 8 ; b++) // seventh byte 46 - 53
     sbuttons[i+b] = program.buttons[6] & (1 << b);
    i+=b;
    for ( b = 0 ; b < 8 ; b++) // eighth byte 54 - 61
     sbuttons[i+b] = program.buttons[7] & (1 << b);
    i+=b;
    for ( b = 0 ; b < 8 ; b++) // ninth byte 62 - 69
     sbuttons[i+b] = program.buttons[8] & (1 << b);
    i+=b;
    for ( b = 0 ; b < 4 ; b++) // tenth byte 70 - 73
     sbuttons[i+b] = program.buttons[9] & (1 << b);
    i+=b;    
  }

   // update the keyboard queue
  keys.update();

  check_faders_now = 0;

  if (fc) return true;
  
  return false;

}


 



