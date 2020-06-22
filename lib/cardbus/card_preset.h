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
  uint8_t read_card_mux_fader(uint8_t mux, uint8_t cnt);   
  uint8_t card_addr;
  uint8_t obuttons[2]; // for change comparisons
  uint8_t ofaders[24]; 
  uint8_t oleds[12];
};


// read fader number cnt from mux address
uint8_t presetCard::read_card_mux_fader(uint8_t mux, uint8_t cnt){
#if defined(FADER_AVERAGING)    
  uint8_t avg[3];
  for (uint8_t c2 = 0; c2 < 3 ; c2++){
#endif      
    writeMux(mux + cnt);    
    selectAddr(card_addr | 0x04);
    clk_ds();
    selectAddr(card_addr | 0x05);     
#if defined(FADER_AVERAGING)       
    avg[c2] = readData();
    delayMicroseconds(FADER_AVERAGING_DELAY);
  }
#else    
    return (readData());     
#endif            
#if defined(FADER_AVERAGING)       
    return ((avg[0]+avg[1]+avg[2])/3);
#endif    
}






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
    faders[cnt] = read_card_mux_fader(0xD8, cnt);


// #if defined(FADER_AVERAGING)    
//     uint8_t avg[3];
//     for (uint8_t c2 = 0; c2 < 3 ; c2++){
// #endif      
//     writeMux(0xD8 + cnt);    
//     selectAddr(card_addr | 0x04);
//     clk_ds();
//     selectAddr(card_addr | 0x05); 
// #if defined(FADER_AVERAGING)       
//     avg[c2] = readData();
// #else
//     faders[cnt] = readData();     
// #endif    
//     delayMicroseconds(10);
//     }

// #if defined(FADER_AVERAGING)       
//     faders[cnt] = ((avg[0]+avg[1]+avg[2])/3);
// #endif    
  }

// 9..16
  for (uint8_t cnt = 0; cnt < 8; cnt++)
  {
    faders[cnt+8] = read_card_mux_fader(0xB8, cnt);
    
  // uint8_t avg[3];
  //   for (uint8_t c2 = 0; c2 < 3 ; c2++){
  //   writeMux(0xB8 + cnt);
  //   selectAddr(card_addr | 0x04);
  //   clk_ds();
  //   selectAddr(card_addr | 0x05); 
  // avg[c2] = readData();
  // delayMicroseconds(10);
  //   }
  //   //faders[cnt + 8] = readData();
  //   faders[cnt+8] = ((avg[0]+avg[1]+avg[2])/3);


  }

// 17..24
  for (uint8_t cnt = 0; cnt < 8; cnt++)
  {
    faders[cnt+16] = read_card_mux_fader(0x78, cnt);
    // uint8_t avg[3];
    // for (uint8_t c2 = 0; c2 < 3 ; c2++){  
    // writeMux(0x78 + cnt);
    // selectAddr(card_addr | 0x04);
    // clk_ds();
    // selectAddr(card_addr | 0x05);        
    // //faders[cnt + 16] = readData();
    //  avg[c2] = readData();
    //  delayMicroseconds(10);
    // }
    //   //faders[cnt + 8] = readData();
    // faders[cnt+16] = ((avg[0]+avg[1]+avg[2])/3);
  }


  for (uint8_t cnt = 0; cnt < 24; cnt++)
  {

#if defined (FADER_FILTERING)    
    if ((ofaders[cnt] == 0) & (faders[cnt] == 1)){
      // ignore 0-1-0-0-0-1-0 glitches 
    }
    else if (  (abs(ofaders[cnt] - faders[cnt]) <= 1) & (faders[cnt] != 255) & (faders[cnt] != 0) ) {
      // ignore differences of 1, filters our edge cases
    }
    else 
#endif    
    
    // look for dirferences
    if ((ofaders[cnt] != faders[cnt] ))
    {
      fc = true; // fader change
      ofaders[cnt] = faders[cnt];
#if defined(FADER_TESTING)
      Serial.printf("P%x F%02d:%02x ",card_addr, cnt ,faders[cnt]); // fader changes      
#endif
    }

    swap(1,12);


  }



} 


  // process two bytes of buttons
  for (uint8_t cnt = 0; cnt < 2; cnt++)
  {
    selectAddr(card_addr | (0x0E + cnt));
    buttons[cnt] = readData();
  }


// set LEDS
  // 1..8
  for (uint8_t cnt = 0; cnt < 8; cnt++)
  {
   
#if defined(PRESET_LEDS_TESTING)
      // force leds to mimic faders
    leds[cnt] = faders[cnt];
#endif
    selectAddr(card_addr | 0x06);
    writeData(leds[cnt]);
    writeMux(0xf0 + cnt);
    delayMicroseconds(1);
    writeMux(0xf8 + cnt);
    oleds[cnt] = leds[cnt];
  
  }
  // 9..12
  for (uint8_t cnt = 0; cnt < 4; cnt++)
  {
#if defined(PRESET_LEDS_TESTING)
      // force leds to mimic faders
      leds[cnt+8] = faders[cnt+8];
#endif
      selectAddr(card_addr | 0x06);
      writeData(leds[cnt+8]);
      writeMux(0xe8 + cnt);
      delayMicroseconds(1);
      writeMux(0xf8 + cnt);
  }

  // process button changes
  for (uint8_t cnt = 0; cnt < 2; cnt++)
  {
    if (buttons[cnt] != obuttons[cnt])
    {
      obuttons[cnt] = buttons[cnt];
#if defined(BUTTON_TESTING)
      Serial.printf("P%d:%d=0x%x\n", card_addr, cnt, buttons[cnt]);
#endif
     fc = true;
    }
  }
  return fc; // true if change
}
