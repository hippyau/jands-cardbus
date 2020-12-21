
#pragma once

#include <stdint.h>
#include "hardware.h"

#include "Keyboard.h"
#include "lcd_module.h"


/*

Echelon MENU Card
                                                                                                                                                                                                               
*/


#define MENU_1K_DEFAULT_CONSTRAST   (0x08)


class menuCard
{
public:
  bool init(uint8_t nAddr); // init / reset card given address
  bool update(); // update card / check for updates
  void setContrast(uint8_t val, uint8_t lcd = 0); // set from 0..15 , 0 = both lcds, otherwise 1 or 2

  uint8_t leds[4] = {0}; // [w] 32 leds - 4 bytes 
  uint8_t buttons[5];    // [r] 36 buttons - 5 bytes
  lcdModule lcd[2];      // [rw] two LCD modules
  uint8_t card_id;       // [r] one byte. should be %01100001 for a ECHMENU2 card 
  
  bool detected; // is card present?

private:

  uint8_t card_addr;  // 0, 1, 2
  uint8_t lcd1_addr;
  uint8_t lcd2_addr; 
  uint8_t contrast_addr;  // IC8 - latch 2 x 4bit resistor-ladder, 2 x low res DAC   
  uint8_t obuttons[5]; // for change comparison...
  uint8_t ocontrast[2]; 

  void setCardAddress(uint8_t addr); 
  uint8_t getCardId(void);
};



bool menuCard::init(uint8_t nAddr) {

    setCardAddress(nAddr);
    detected = false;
    getCardId();
    if (card_id == 0b01100001){
#if defined(MENU_1K_CARD_TESTING)
      Serial.printf("Found ECHMENU2 Card @ 0x%02X\n\r",card_addr);
      detected = true;
#endif        
    } else {
      Serial.printf("Not Found ECHMENU2 Card @ 0x%02X\n\r",card_addr);
      detected = false;
      return detected; 
    }

    // zero out leds and buttons
    for (uint8_t cnt = 0; cnt < 4; cnt++)
      leds[cnt] = 0;
    for (uint8_t cnt = 0; cnt < 5; cnt++)
      buttons[cnt] = obuttons[cnt] = 0;  

    setContrast(MENU_1K_DEFAULT_CONSTRAST); // set all displays to default constrast

    lcd[0].init(lcd1_addr);    
#if defined(MENU_1K_CARD_TESTING)
    lcd[0].print("Jands Menu Card - LCD 1 - Init OK ");      
#endif

    lcd[1].init(lcd2_addr);    
#if defined(MENU_1K_CARD_TESTING)
    lcd[1].print("Jands Menu Card - LCD 2 - Init OK ");
#endif

  return detected; // card detected
}


void menuCard::setCardAddress(uint8_t addr) {
    card_addr = addr; // IC3 & IC5
    lcd1_addr = card_addr | 0x00; // [rw] LCD0A  
    lcd2_addr = card_addr | 0x02; // [rw] LCD1A
    contrast_addr = card_addr | 0x05;// [w] IC8
};


// ECHMENU2 IC8 
void menuCard::setContrast(uint8_t val, uint8_t lcd = 0) {
    if (!detected) {
        return;  // card is not present, don't update...
    }

    val = val & 0b00001111;  //clamp
    if (lcd == 0) {  // do both      
      selectAddr(contrast_addr); 
      writeData((val << 4) | val);  // default contrast
      ocontrast[0]=val;
      ocontrast[1]=val;
    } else 
    if (lcd == 2) {  // do 1      
      selectAddr(contrast_addr);  
      writeData((val << 4) | ocontrast[0]);  // default contrast
      ocontrast[1] = val;
    } else 
    if (lcd == 1) {  // do both      
      selectAddr(contrast_addr);  
      writeData((ocontrast[1]) | val);  // default contrast
      ocontrast[0] = val;
    } else {
      // invalid input
#if defined(MENU_1K_CARD_TESTING)
      Serial.println("menuCard::setContrast: Invalid LCD specified.");
#endif       
    }
};


// ECHMENU2 IC17 should return ca
uint8_t menuCard::getCardId(void) {
    selectAddr(card_addr | 0x0F); // Card ID
    card_id = readData();
    return card_id;    
}

// Echelon 1K Menu Card Driver
// Returns: true on change detected
bool menuCard::update()
{
  if (!detected) {
      return detected;  // card is not present, don't update...
  }

  // buttons and LEDS
  selectAddr(card_addr | 0x0A); // IC12 
  buttons[0] = readData();        // read 8 buttons
  writeData(leds[0]);           // IC18

  selectAddr(card_addr | 0x0B); // IC13
  buttons[1] = readData();        // read 8 buttons
  writeData(leds[1]);           // IC19

  selectAddr(card_addr | 0x0C); // IC14
  buttons[2] = readData();        // read 8 buttons
  writeData(leds[2]);           // IC20

  selectAddr(card_addr | 0x0D); // IC15
  buttons[3] = readData();        // read 8 buttons
  writeData(leds[3]);           // IC21

  selectAddr(card_addr | 0x0E); // IC14
  buttons[4] = readData();        // read 4 buttons

  // look for button changes
  bool fc = false;
  for (uint8_t n = 0; n < 4; n++)
  {
    if (buttons[n] != obuttons[n])
      {
        fc = true;
        obuttons[n] = buttons[n];
      }
  }

#if defined(MENU_1K_CARD_TESTING)
    // light up some leds when pressed buttons
    leds[0] = buttons[0];
    leds[1] = buttons[1];
    leds[2] = buttons[2];
    leds[3] = buttons[3];
        
    if (fc)
    {    
        lcd[0].printf("0B=0x%x 1B=0x%x 2B=0x%x 3B=0x%x 4B=0x%x\n ", buttons[0], buttons[1], buttons[2], buttons[3], buttons[4]);        
        lcd[1].printf("0B=0x%x 1B=0x%x 2B=0x%x 3B=0x%x 4B=0x%x\n ", buttons[0], buttons[1], buttons[2], buttons[3], buttons[4]);        
        Serial.printf("0B=0x%x 1B=0x%x 2B=0x%x 3B=0x%x 4B=0x%x\n ", buttons[0], buttons[1], buttons[2], buttons[3], buttons[4]);      
    }
#endif
 
 return fc;
} 

