
#pragma once

#include <stdint.h>
#include "hardware.h"

#include "Keyboard.h"

/*

Echelon PROGRAM Card
                                                                                                                                                                                                               
*/

#define ADDR_PROGRAM_1K (0xF0)


// debugging 
#define PROGRAM_1K_CARD_TESTING  (true)


class programCard
{
public:
  bool update();

  // inputs - 2 bytes
  uint8_t leds[2];

  // outputs - 16 bytes
  uint8_t buttons[10]; // 76 buttons
  uint8_t wheels[3];  // 4 bit counter x 3


  programCard() // constructor
  {
    card_addr = ADDR_PROGRAM_1K;     // default base address for a master card, can change latter
    lcd_addr = card_addr | 0x00; // not supportive of multiple assign cards
    for (uint8_t cnt = 0; cnt < 2; cnt++)
      leds[cnt] = 0;
    for (uint8_t cnt = 0; cnt < 10; cnt++)
      buttons[cnt] = obuttons[cnt] = 0;
    for (uint8_t cnt = 0; cnt < 3; cnt++)
      wheels[cnt] = owheels[cnt] = 0;  
  }


  void setCardAddress(uint8_t addr)
  {
    card_addr = addr;
  };


private:
  uint8_t card_addr;
  uint8_t lcd_addr;

  uint8_t obuttons[10]; // for change comparisons...
  uint8_t owheels[3];
};



// Echelon 1K Program Card Driver
// Returns: true on change detected
bool programCard::update()
{
  // buttons and LEDS
  selectAddr(card_addr | 0x05); // 
  buttons[0] = readData();        // read 8 buttons
  selectAddr(card_addr | 0x06); // 
  buttons[1] = readData();        // read 8 buttons
  selectAddr(card_addr | 0x07); // 
  buttons[2] = readData();        // read 8 buttons
  selectAddr(card_addr | 0x08); // 
  buttons[3] = readData();        // read 8 buttons
  selectAddr(card_addr | 0x09); // 
  buttons[4] = readData();        // read 8 buttons
  selectAddr(card_addr | 0x0A); // 
  buttons[5] = readData();        // read 8 buttons
  selectAddr(card_addr | 0x0B); // 
  buttons[6] = readData();        // read 8 buttons
  selectAddr(card_addr | 0x0C); // 
  buttons[7] = readData();        // read 8 buttons
  selectAddr(card_addr | 0x0D); // LED5
  buttons[8] = readData();        // read 8 buttons
  writeData(leds[0]);             // write 8 LEDS
  selectAddr(card_addr | 0x0E); // LED6
  buttons[9] = readData();        // read 8 buttons
  writeData(leds[1]);             // write 8 LEDS
  selectAddr(card_addr | 0x0F); // LED7 / ENC1 - encoder 3
  writeData(leds[2]);             // write 8 LEDS
  
    // encoder wheels
  wheels[2] = 0x0f & buttons[9];//readData(); // top 4 bits are card ID, ignore
  buttons[9] = buttons[9] >> 4;
  selectAddr(card_addr | 0x00);  // ENC0 - encoder 1 + 2
  wheels[0] = readData();         // read encode 1+2
  wheels[1] = (wheels[0] >> 4) & 0x0f; // high nibble
  wheels[0] = wheels[0] & 0x0f;        // low nibble

  bool fc = false;
  // look for wheel changes
  for (uint8_t n = 0; n < 3; n++) 
  {
    if (owheels[n] != wheels[n])
      {
        fc = true;
        owheels[n] = wheels[n];
      }
  }

  // look for button changes
  for (uint8_t n = 0; n < 10; n++)
  {
    if (buttons[n] != obuttons[n])
      {
        fc = true;
        obuttons[n] = buttons[n];
      }
  }

#if defined(PROGRAM_1K_CARD_TESTING)
    // light up some leds when pressed buttons
    leds[0] = buttons[0];
    leds[1] = buttons[1];
    
    if (fc)
    {      
        Serial.printf("0B=0x%x 1B=0x%x 2B=0x%x 3B=0x%x 4B=0x%x 5B=0x%x 6B=0x%x 7B=0x%x 8B=0x%x 9B=0x%x W1=0x%x W2=0x%x W3=0x%x \n ", buttons[0], buttons[1], buttons[2], buttons[3], buttons[4], buttons[5], buttons[6], buttons[7],buttons[8], buttons[9], wheels[0], wheels[1], wheels[2]);      
    }
#endif
 
 
 return fc;
} 
