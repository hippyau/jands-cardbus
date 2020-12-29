#ifndef _JANDSCARDBUS_H_
#define _JANDSCARDBUS_H_
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
#define FADER_POLL_SPEED 25

// number of virtual fader pages to store values for
#define FADER_PAGES 10

/* ignore 0->1 transitions and changes less than 2 difference */
#define FADER_FILTERING // filtering fader values

/* when reading faders, takes 3 samples and average them for final fader values
   not entirely required if filtering is being done, but helps with jitter and noise */
#define FADER_AVERAGING
#define FADER_AVERAGING_DELAY 10 // delay in uS between samples - shorter the better

// Default Card Addresses

// - Event 408
#if defined(CONFIG_EVENT_408)
#define ADDR_PRESET_1 (0x00)
#define ADDR_PRESET_2 (0x10)
#define ADDR_MASTER (0x80)
#define ADDR_PALETTE (0x90)
#define ADDR_ASSIGN (0xC0)
#endif

#if defined(CONFIG_ECHELON_1K)
// - Echelon 1K
#define ADDR_PROGRAM_1K (0xF0)    // program card, only one allowed, always 0xF0
#define ADDR_MENU_1_1K (0x00)     // menu card 0
#define ADDR_MENU_2_1K (0x10)     // menu card 1
#define ADDR_PLAYBACK_1_1K (0x80) // playback card 1
#define ADDR_PLAYBACK_2_1K (0x90) // playback card 2
#endif

// hardware bus driver routines
#include "hardware.h"

// components
#include "surface_buttons.h"
#include "lcd_module.h"
#include "button_def.h"
#include "keypad_input.h"

// individual card drivers
#if defined(CONFIG_EVENT_408)
#include "card_preset.h"
#include "card_palette.h"
#include "card_assign.h"
#include "card_master.h"
#endif

#if defined(CONFIG_ECHELON_1K)
#include "card_program_1k.h"
#include "card_menu_1k.h"
#include "card_playback_1k.h"
#endif

// the whole control surface
class JandsCardBus
{
public:
  bool update(); // update all cards

#if defined(CONFIG_EVENT_408)
  presetCard preset1;
  presetCard preset2;
  paletteCard palette;
  assignCard assign;
  masterCard master;
#endif

#if defined(CONFIG_ECHELON_1K)
  programCard program;
  menuCard menu1;
  menuCard menu2;
  playbackCard playback1;
  playbackCard playback2;
#endif

  SKeyboard keys; // not harware, rather a key press input processor

  bool halt = false; // if true, update is not allowed.

  uint8_t FaderPage = 0; // current fader page
};

// pointer to our entire control surface
static JandsCardBus *Surface = NULL;

/* Implementation */

// Update the control surface elements
// Returns: true on change detected
bool inline JandsCardBus::update()
{
  if (halt == true)
  {
    return false;
  } // stop updates

  static unsigned long lastMillis;
  unsigned long now = millis();

  bool check_faders_now = 1;
  if ((now - lastMillis) >= FADER_POLL_SPEED)
  { // poll faders on occasion, every 6ms
    check_faders_now = 1;
    lastMillis = now;
  }

  uint8_t fc = 0;

#if defined(CONFIG_EVENT_408)
  fc += preset1.update(check_faders_now);
  fc += preset2.update(check_faders_now);
  fc += assign.update(check_faders_now);
  fc += master.update(check_faders_now);
  fc += palette.update();
#endif

#if defined(CONFIG_ECHELON_1K)
  fc += program.update();
  fc += menu1.update();
  fc += menu2.update();
  fc += playback1.update(check_faders_now);
  fc += playback2.update(check_faders_now);
#endif

  if (fc)
  {
    // pull buttons from individual bits out to an array

    uint16_t i = 0, b = 0;

#if defined(CONFIG_EVENT_408)
    // preset card 1 buttons 1-12
    for (b = 0; b < 8; b++) // first byte
      sbuttons[i + b] = preset1.buttons[0] & (1 << b);
    i += b;                 //
    for (b = 0; b < 4; b++) // second byte, first nibble
      sbuttons[i + b] = preset1.buttons[1] & (1 << b);
    i += b; //

    // preset card 2 buttons 1-12
    for (b = 0; b < 8; b++) // first byte
      sbuttons[i + b] = preset2.buttons[0] & (1 << b);
    i += b;                 //
    for (b = 0; b < 4; b++) // second byte, first nibble
      sbuttons[i + b] = preset2.buttons[1] & (1 << b);
    i += b; //

    // assign card buttons 1-8
    for (b = 0; b < 8; b++) // first byte
      sbuttons[i + b] = assign.buttons[0] & (1 << b);
    i += b; //

    // palette card buttons 1-20
    for (b = 0; b < 8; b++) // first byte
      sbuttons[i + b] = palette.buttons[0] & (1 << b);
    i += b;
    for (b = 0; b < 8; b++) // second byte
      sbuttons[i + b] = palette.buttons[1] & (1 << b);
    i += b;
    for (b = 0; b < 4; b++) // third byte, first nibble
      sbuttons[i + b] = palette.buttons[2] & (1 << b);
    i += b;

    // master card buttons 1-37
    for (b = 0; b < 8; b++) // first byte
      sbuttons[i + b] = master.buttons[0] & (1 << b);
    i += b;
    for (b = 0; b < 8; b++) // second byte
      sbuttons[i + b] = master.buttons[1] & (1 << b);
    i += b;
    for (b = 0; b < 8; b++) // third byte
      sbuttons[i + b] = master.buttons[2] & (1 << b);
    i += b;
    for (b = 0; b < 8; b++) // fourth byte
      sbuttons[i + b] = master.buttons[3] & (1 << b);
    i += b;
    for (b = 0; b < 8; b++) // fifth byte 32-37
      sbuttons[i + b] = master.buttons[4] & (1 << b);
//    i+=b;
#endif

#if defined(CONFIG_ECHELON_1K)
    i = 92;                 //167;
                            // program card buttons 1-76
    for (b = 0; b < 8; b++) // first byte
      sbuttons[i + b] = program.buttons[0] & (1 << b);
    i += b;
    for (b = 0; b < 8; b++) // second byte
      sbuttons[i + b] = program.buttons[1] & (1 << b);
    i += b;
    for (b = 0; b < 8; b++) // third byte
      sbuttons[i + b] = program.buttons[2] & (1 << b);
    i += b;
    for (b = 0; b < 8; b++) // fourth byte
      sbuttons[i + b] = program.buttons[3] & (1 << b);
    i += b;
    for (b = 0; b < 8; b++) // fifth byte 32-37
      sbuttons[i + b] = program.buttons[4] & (1 << b);
    i += b;
    for (b = 0; b < 8; b++) // sixth byte 38 - 45
      sbuttons[i + b] = program.buttons[5] & (1 << b);
    i += b;
    for (b = 0; b < 8; b++) // seventh byte 46 - 53
      sbuttons[i + b] = program.buttons[6] & (1 << b);
    i += b;
    for (b = 0; b < 8; b++) // eighth byte 54 - 61
      sbuttons[i + b] = program.buttons[7] & (1 << b);
    i += b;
    for (b = 0; b < 8; b++) // ninth byte 62 - 69
      sbuttons[i + b] = program.buttons[8] & (1 << b);
    i += b;
    for (b = 0; b < 4; b++) // tenth byte 70 - 73
      sbuttons[i + b] = program.buttons[9] & (1 << b);
    i += b;

    // menu card 1 buttons 1-36
    for (b = 0; b < 8; b++) // first byte
      sbuttons[i + b] = menu1.buttons[0] & (1 << b);
    i += b;
    for (b = 0; b < 8; b++) // second byte
      sbuttons[i + b] = menu1.buttons[1] & (1 << b);
    i += b;
    for (b = 0; b < 8; b++) // third byte
      sbuttons[i + b] = menu1.buttons[2] & (1 << b);
    i += b;
    for (b = 0; b < 8; b++) // fourth byte
      sbuttons[i + b] = menu1.buttons[3] & (1 << b);
    i += b;
    for (b = 0; b < 4; b++) // fifth byte 32-37
      sbuttons[i + b] = menu1.buttons[4] & (1 << b);
    i += b;

    // menu card 2 buttons 1-36
    for (b = 0; b < 8; b++) // first byte
      sbuttons[i + b] = menu2.buttons[0] & (1 << b);
    i += b;
    for (b = 0; b < 8; b++) // second byte
      sbuttons[i + b] = menu2.buttons[1] & (1 << b);
    i += b;
    for (b = 0; b < 8; b++) // third byte
      sbuttons[i + b] = menu2.buttons[2] & (1 << b);
    i += b;
    for (b = 0; b < 8; b++) // fourth byte
      sbuttons[i + b] = menu2.buttons[3] & (1 << b);
    i += b;
    for (b = 0; b < 4; b++) // fifth byte 32-37
      sbuttons[i + b] = menu2.buttons[4] & (1 << b);
    i += b;

    // playback card 1 buttons 1-34
    for (b = 0; b < 8; b++) // first byte
      sbuttons[i + b] = playback1.buttons[0] & (1 << b);
    i += b;
    for (b = 0; b < 8; b++) // second byte
      sbuttons[i + b] = playback1.buttons[1] & (1 << b);
    i += b;
    for (b = 0; b < 8; b++) // third byte
      sbuttons[i + b] = playback1.buttons[2] & (1 << b);
    i += b;
    for (b = 0; b < 8; b++) // fourth byte
      sbuttons[i + b] = playback1.buttons[3] & (1 << b);
    i += b;
    for (b = 0; b < 2; b++) // fifth byte 32-33
      sbuttons[i + b] = playback1.buttons[4] & (1 << b);
    i += b;

    // playback card 2 buttons 1-34
    for (b = 0; b < 8; b++) // first byte
      sbuttons[i + b] = playback2.buttons[0] & (1 << b);
    i += b;
    for (b = 0; b < 8; b++) // second byte
      sbuttons[i + b] = playback2.buttons[1] & (1 << b);
    i += b;
    for (b = 0; b < 8; b++) // third byte
      sbuttons[i + b] = playback2.buttons[2] & (1 << b);
    i += b;
    for (b = 0; b < 8; b++) // fourth byte
      sbuttons[i + b] = playback2.buttons[3] & (1 << b);
    i += b;
    for (b = 0; b < 2; b++) // fifth byte 32-33
      sbuttons[i + b] = playback2.buttons[4] & (1 << b);
//    i+=b;
#endif
  }

  // update the keyboard queue
  keys.update();

  check_faders_now = 0;

  if (fc)
    return true;

  return false;
}

#endif // _JANDSCARDBUS_H_
