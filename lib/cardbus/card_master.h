#pragma once

#include <stdint.h>
#include "hardware.h"
#include "lcd_module.h"

/*
███╗   ███╗ █████╗ ███████╗████████╗███████╗██████╗      ██████╗ █████╗ ██████╗ ██████╗ 
████╗ ████║██╔══██╗██╔════╝╚══██╔══╝██╔════╝██╔══██╗    ██╔════╝██╔══██╗██╔══██╗██╔══██╗
██╔████╔██║███████║███████╗   ██║   █████╗  ██████╔╝    ██║     ███████║██████╔╝██║  ██║
██║╚██╔╝██║██╔══██║╚════██║   ██║   ██╔══╝  ██╔══██╗    ██║     ██╔══██║██╔══██╗██║  ██║
██║ ╚═╝ ██║██║  ██║███████║   ██║   ███████╗██║  ██║    ╚██████╗██║  ██║██║  ██║██████╔╝
╚═╝     ╚═╝╚═╝  ╚═╝╚══════╝   ╚═╝   ╚══════╝╚═╝  ╚═╝     ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═════╝                                                                                                                                                                                                                 
*/

class masterCard
{
public:
  bool update(bool check_faders_now);

  // inputs - 3 bytes
  uint8_t leds[3];

  // outputs - 16 bytes
  uint8_t buttons[5]; // 37 buttons
  uint8_t faders[8];  // 8th fader is for testing DAC
  uint8_t wheels[3];  // 4 bit counter x 3

  // special
  lcdModule lcd;

  masterCard() // constructor
  {
    card_addr = ADDR_MASTER;     // default base address for a master card, can change latter
    lcd_addr = card_addr | 0x00; // not supportive of multiple assign cards
    for (uint8_t cnt = 0; cnt < 3; cnt++)
      leds[cnt] = 0;
    for (uint8_t cnt = 0; cnt < 5; cnt++)
      buttons[cnt] = obuttons[cnt] = 0;
    for (uint8_t cnt = 0; cnt < 8; cnt++)
      faders[cnt] = ofaders[cnt] = 0;
    for (uint8_t cnt = 0; cnt < 3; cnt++)
      wheels[cnt] = owheels[cnt] = 0;
    lcd.init(lcd_addr);    
    lcd.print("Jands Master Card - V0.8 - Init OK ");
  }


  void setCardAddress(uint8_t addr)
  {
    card_addr = addr;
  };


private:
  uint8_t card_addr;
  uint8_t lcd_addr;

  uint8_t obuttons[5]; // for change comparisons...
  uint8_t ofaders[8]; 
  uint8_t owheels[3];
};


// Master Card Driver
// Returns: true on change detected
bool masterCard::update(bool check_faders_now = true)
{
  // buttons and LEDS
  selectAddr(card_addr | 0x0A); // SW2
  buttons[0] = readData();        // read 8 buttons
  selectAddr(card_addr | 0x0B); // SW3
  buttons[1] = readData();        // read 8 buttons
  selectAddr(card_addr | 0x0C); // SW4
  buttons[2] = readData();        // read 8 buttons
  selectAddr(card_addr | 0x0D); // SW5 / LED5
  buttons[3] = readData();        // read 8 buttons
  writeData(leds[0]);             // write 8 LEDS
  selectAddr(card_addr | 0x0E); // SW6 / LED6
  buttons[4] = readData();        // read 8 buttons
  writeData(leds[1]);             // write 8 LEDS
  selectAddr(card_addr | 0x0F); // LED7 / ENC1 - encoder 3
  writeData(leds[2]);             // write 8 LEDS
  // encoder wheels
  wheels[2] = 0x0f & readData(); // top 4 bits are card ID, ignore
  selectAddr(card_addr | 0x02);  // ENC0 - encoder 1 + 2
  wheels[0] = readData();         // read encode 1+2
  wheels[1] = (wheels[0] >> 4) & 0x0f; // high nibble
  wheels[0] = wheels[0] & 0x0f;        // low nibble

 bool fc = 0;

 if (check_faders_now){  // time to check faders
  // read the 8 faders.....
  selectAddr(card_addr | 0x04);
 
  for (uint8_t cnt = 0; cnt < 8; cnt++)
  {
    writeMux(0x78 + cnt);
    selectAddr(card_addr | 0x04);
    clk_ds();
    clk_ds();
    selectAddr(card_addr | 0x05);
    faders[cnt] = readData();
  }
 
    // look for changes
    for (uint8_t n = 0; n < 8; n++) // faders
    {
      if (ofaders[n] != faders[n])
      {
        fc = true; // flag change
        ofaders[n] = faders[n];
      }
    }
  } // end check faders

  for (uint8_t n = 0; n < 3; n++) // wheels
  {
    if (owheels[n] != wheels[n])
      {
        fc = true;
        owheels[n] = wheels[n];
      }
  }
  for (uint8_t n = 0; n < 5; n++) // buttons
  {
    if (buttons[n] != obuttons[n])
      {
        fc = true;
        obuttons[n] = buttons[n];
      }
  }

#if defined(TESTING)
    // light up some leds when pressed buttons
    leds[0] = buttons[0];
    leds[1] = buttons[1];
    leds[2] = buttons[2];

    if (fc)
    {
      // Serial.printf("AB=0x%x\n", buttons[0]);
      lcd.setCursor(0, 0);
      lcd.printf("Buttons: %x %x %x %x %x Wheels: %x %x %x ", buttons[0], buttons[1], buttons[2], buttons[3], buttons[4], wheels[0], wheels[1], wheels[2]);
      lcd.setCursor(0, 1);
      lcd.printf(" %03d  %03d  %03d  %03d  %03d  %03d  %03d  %03d", faders[0], faders[1], faders[2], faders[3], faders[4], faders[5], faders[6], faders[7]);
    }
#endif
 
 return fc;
} // end master card update


