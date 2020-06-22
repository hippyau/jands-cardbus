#pragma once


#include <stdint.h>
#include "hardware.h"


/*
██████╗ ██████╗ ███████╗███████╗███████╗████████╗     ██████╗ █████╗ ██████╗ ██████╗ 
██╔══██╗██╔══██╗██╔════╝██╔════╝██╔════╝╚══██╔══╝    ██╔════╝██╔══██╗██╔══██╗██╔══██╗
██████╔╝██████╔╝█████╗  ███████╗█████╗     ██║       ██║     ███████║██████╔╝██║  ██║
██╔═══╝ ██╔══██╗██╔══╝  ╚════██║██╔══╝     ██║       ██║     ██╔══██║██╔══██╗██║  ██║
██║     ██║  ██║███████╗███████║███████╗   ██║       ╚██████╗██║  ██║██║  ██║██████╔╝
╚═╝     ╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝   ╚═╝        ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═════╝                                                                                                                                                                                                            
*/

class presetCard
{
public:
  bool update(bool check_faders_now);

  // inputs - 12 bytes
  uint8_t leds[12]; // 12 analog levels

  // outputs - 26 bytes
  uint8_t buttons[2]; // 12 buttons in 2 bytes
  uint8_t faders[24]; // 24 analog values

  presetCard(uint8_t base_addr = ADDR_PRESET_1)
  {
    card_addr = base_addr; // default base address for an assign card, can change latter
    for (uint8_t cnt = 0; cnt < 12; cnt++)
      leds[cnt] = 0;
    for (uint8_t cnt = 0; cnt < 24; cnt++)
      faders[cnt] = 0;
    for (uint8_t cnt = 0; cnt < 24; cnt++)
      ofaders[cnt] = 0;
    for (uint8_t cnt = 0; cnt < 2; cnt++)
      obuttons[cnt] = buttons[cnt] = 0;
    for (uint8_t cnt = 0; cnt < 12; cnt++)
      oleds[cnt] = 255;
  }

  void setCardAddress(uint8_t addr)
  {
    card_addr = addr;
  };

private:
  uint8_t card_addr;
  uint8_t obuttons[2]; // for change comparisons
  uint8_t ofaders[24]; 
  uint8_t oleds[12];
};



// Preset Card Driver
bool presetCard::update(bool check_faders_now = true)
{

  // read the 24 faders.....
  /*
 faders layout 

 0xD8     0xB8         0x78
 1 3 5 7  9  11 13 15  17 19 21 23

 2 4 6 8  10 12 14 16  18 20 22 24
*/

// 1..8
  bool fc = 0; // flag a change has occured

if (check_faders_now) {  

  for (uint8_t cnt = 0; cnt < 8; cnt++)
  {
    writeMux(0xD8 + cnt);
    selectAddr(card_addr | 0x04);
    clk_ds();
    clk_ds();
    selectAddr(card_addr | 0x05);
    faders[cnt] = readData();
  }

// 9..16
  for (uint8_t cnt = 0; cnt < 8; cnt++)
  {
    writeMux(0xB8 + cnt);
    selectAddr(card_addr | 0x04);
    clk_ds();
    clk_ds();
    selectAddr(card_addr | 0x05);
    faders[cnt + 8] = readData();
  }

// 17..24
  for (uint8_t cnt = 0; cnt < 8; cnt++)
  {
    writeMux(0x78 + cnt);
    selectAddr(card_addr | 0x04);
    clk_ds();
    clk_ds();
    selectAddr(card_addr | 0x05);
    faders[cnt + 16] = readData();
  }

  for (uint8_t cnt = 0; cnt < 24; cnt++)
  {
    if (ofaders[cnt] != faders[cnt])
    {
      fc = true; // fader change
      ofaders[cnt] = faders[cnt];
#if defined(FADER_TESTING)
      Serial.printf("F%02d:%02x ",cnt,faders[cnt]); // fader changes
      // if (cnt < 12) leds[cnt] = faders[cnt]; // set leds to faders
#endif
    }
  }

} // check faders


  // process two bytes of buttons
  for (uint8_t cnt = 0; cnt < 2; cnt++)
  {
    selectAddr(card_addr | (0x0E + cnt));
    buttons[cnt] = readData();
  }

  // TODO: SET LEDS FIX
  //  uint8_t level = 255;
  //  leds[0] = level; 
  //  leds[1] = level -= 21; // 255
  //  leds[2] = level -= 21;
  //  leds[3] = level -= 21;
  //  leds[4] = level -= 21;
  //  leds[5] = level -= 21;
  //  leds[6] = level -= 22;
  //  leds[7] = level -= 24;
  //  leds[8] = level -= 26;
  //  leds[9] = level -= 27;
  //  leds[10] = level -= 30;
  //  leds[11] = level -= 21; // 0 

  // 1..8
  for (uint8_t cnt = 0; cnt < 8; cnt++)
  {
    if (leds[cnt] != oleds[cnt])
    {
      selectAddr(card_addr | 0x06);
      writeData(leds[cnt]);
      writeMux(0xf0 + cnt);
      oleds[cnt] = leds[cnt];
    }
  }

  // 9..12
  for (uint8_t cnt = 0; cnt < 4; cnt++)
  {
    if (leds[cnt + 8] != oleds[cnt + 8])
    {
      selectAddr(card_addr | 0x06);
      writeData(leds[cnt + 8]);
      writeMux(0xe8 + cnt);
    }
  }

  // process changes
  for (uint8_t cnt = 0; cnt < 2; cnt++)
  {
    if (buttons[cnt] != obuttons[cnt])
    {
      fc = true;
      obuttons[cnt] = buttons[cnt];
#if defined(BUTTON_TESTING)
      Serial.printf("P1-%d=0x%x\n", cnt, buttons[cnt]);
#endif
    }
  }

  return fc; // true if change
}
