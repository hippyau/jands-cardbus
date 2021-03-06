#pragma once

#include <stdint.h>
#include "hardware.h"
#include "lcd_module.h"

/*
 █████╗ ███████╗███████╗██╗ ██████╗ ███╗   ██╗     ██████╗ █████╗ ██████╗ ██████╗ 
██╔══██╗██╔════╝██╔════╝██║██╔════╝ ████╗  ██║    ██╔════╝██╔══██╗██╔══██╗██╔══██╗
███████║███████╗███████╗██║██║  ███╗██╔██╗ ██║    ██║     ███████║██████╔╝██║  ██║
██╔══██║╚════██║╚════██║██║██║   ██║██║╚██╗██║    ██║     ██╔══██║██╔══██╗██║  ██║
██║  ██║███████║███████║██║╚██████╔╝██║ ╚████║    ╚██████╗██║  ██║██║  ██║██████╔╝
╚═╝  ╚═╝╚══════╝╚══════╝╚═╝ ╚═════╝ ╚═╝  ╚═══╝     ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═════╝                                                                                                                          
*/

class assignCard
{
public:
  bool update(bool check_faders_now);

  // input
  uint8_t leds[2];

  // output
  uint8_t buttons[1];
  uint8_t faders[8];

  // special
  lcdModule lcd;

  assignCard() // constructor
  {
    card_addr = ADDR_ASSIGN; // default base address for an assign card, can change latter
    lcd_addr = 0xc0;         // not supportive of multiple assign cards
    for (uint8_t cnt = 0; cnt < 2; cnt++)
      leds[cnt] = 0;
    for (uint8_t cnt = 0; cnt < 8; cnt++)
      faders[cnt] = ofaders[cnt] = 0;
    buttons[0] = obuttons[0] = 0;
    lcd.init(lcd_addr);
  }


  void setCardAddress(uint8_t addr)
  {
    card_addr = addr;
  };

private:
  uint8_t read_card_mux_fader(uint8_t mux, uint8_t cnt);

  uint8_t card_addr;
  uint8_t lcd_addr;
  uint8_t obuttons[1]; // for comparisons
  uint8_t ofaders[8];
};


// read fader number cnt from mux address
uint8_t assignCard::read_card_mux_fader(uint8_t mux, uint8_t cnt){
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




// process the assign card
bool assignCard::update(bool check_faders_now = true)
{
  bool fc = 0;
  // buttons and LEDS
  selectAddr(card_addr | (0x0E)); // first byte of LED data (R)
  writeData(leds[0]);
  buttons[0] = readData();   // read 8 buttons at this address
  selectAddr(card_addr | (0x0F)); // second byte of LED data (G)
  writeData(leds[1]); 

 if (check_faders_now){
  // read 8 faders.,,
      for (uint8_t cnt = 0; cnt < 8; cnt++)
    {
      faders[cnt] = read_card_mux_fader(0x40,cnt);

#if defined (FADER_FILTERING)    
    if ((ofaders[cnt] == 0) & (faders[cnt] == 1)){
      // ignore 0-1-0-0-0-1-0 glitches 
    }
    else if (  (abs(ofaders[cnt] - faders[cnt]) <= 1) & (faders[cnt] != 255) & (faders[cnt] != 0) ) {
      // ignore differences of 1, filters our edge cases
    }
    else 
#endif  
      // detect changes
      if (faders[cnt] != ofaders[cnt])
      {
        ofaders[cnt] = faders[cnt];
        fc = true; // fader change
      }
    }
  } // check faders

  // detect changes
  if (buttons[0] != obuttons[0])
  {
    fc = true;
    obuttons[0] = buttons[0];
  }

#if defined(ASSIGN_CARD_LCD_TESTING)
  leds[1] = buttons[0]; // light up pressed button in green LEDS
  if (fc)
  {
    // Serial.printf("AB=0x%x\n", buttons[0]);
    lcd.setCursor(0, 1);
    lcd.printf(" %03d  %03d  %03d  %03d  %03d  %03d  %03d  %03d", faders[0], faders[1], faders[2], faders[3], faders[4], faders[5], faders[6], faders[7]);
  }
#endif

  return fc;
}

