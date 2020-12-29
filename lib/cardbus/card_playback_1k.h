#pragma once

#include <stdint.h>
#include "hardware.h"

#include "Keyboard.h"
#include "lcd_module.h"

/*

Echelon PLAYBACK Card (ECHPBK2.SC1 Oct 1, 1999, D.Timmins)
                                                                                                                                                                                                               
*/

namespace PLAYBACK_CARD {
constexpr uint8_t NUM_OF_FADERS = 9;
constexpr uint8_t NUM_OF_LCDS = 1;

constexpr uint8_t NUM_OF_BUTTON_BYTES = 5;
constexpr uint8_t NUM_OF_LED_BYTES = 5;

constexpr uint8_t NUM_OF_CONTRAST_CONTROLS = 1;
constexpr uint8_t DEFAULT_CONTRAST = 0x08;
}


class playbackCard
{
public:
  bool init(uint8_t nAddr); // init / reset card given address, return true if detected
  bool update(bool check_faders_now); // update card / check for updates
  void setContrast(uint8_t val, uint8_t lcd = 0); // set from 0..15 , 0 = both lcds, otherwise 1 or 2

  lcdModule lcd[PLAYBACK_CARD::NUM_OF_LCDS];      // [rw] one LCD module [LCD0A]
  uint8_t leds[PLAYBACK_CARD::NUM_OF_LED_BYTES] = {0}; // [w] 33 leds - 4 bytes - [IC18 - IC22 = 74HC573]
  uint8_t buttons[PLAYBACK_CARD::NUM_OF_BUTTON_BYTES] = {0}; // [r] 34 buttons - 5 bytes [IC12 - IC16 = 74HC245]
  uint8_t card_id = 0xFF;   // [r] [IC17 = 74HC254] one byte. should be %01100010 for a Echelon Playback card
  uint8_t faders[PLAYBACK_CARD::NUM_OF_FADERS] = {0};     // [r] [MUX IC11 - IC 10] 8 faders + GM if fitted;
  
  bool detected; // is this card present?

private:
  
  uint8_t card_addr; 
  uint8_t lcd1_addr;
  uint8_t contrast_addr;  // IC8 - latch 2 x 4bit resistor-ladder, 1 x low res DAC   
  
  uint8_t obuttons[PLAYBACK_CARD::NUM_OF_BUTTON_BYTES]; // for change comparison...
  uint8_t ocontrast[PLAYBACK_CARD::NUM_OF_CONTRAST_CONTROLS]; 
  uint8_t ofaders[PLAYBACK_CARD::NUM_OF_FADERS]; 
  
  uint8_t read_card_mux_fader(uint8_t mux, uint8_t cnt);
  void setCardAddress(uint8_t addr); 
  uint8_t getCardId(void);
};



bool playbackCard::init(uint8_t nAddr) {

    setCardAddress(nAddr);
    detected = false;
    getCardId();
    if (card_id == CARD_TYPES::PLAYBACK_CARD){
#if defined(PLAYBACK_1K_CARD_TESTING)
      Serial.printf("Found Playback Card @ 0x%02X\n\r",card_addr);
      detected = true;
#endif        
    } else {
      Serial.printf("Not Found - Playback Card @ 0x%02X\n\r",card_addr);
      detected = false;
      return detected; 
    }
    // zero out leds, buttons and faders
    for (uint8_t cnt = 0; cnt < PLAYBACK_CARD::NUM_OF_LED_BYTES; cnt++)
      leds[cnt] = 0;
    for (uint8_t cnt = 0; cnt < PLAYBACK_CARD::NUM_OF_BUTTON_BYTES; cnt++)
      buttons[cnt] = obuttons[cnt] = 0; 
    for (uint8_t cnt = 0; cnt < PLAYBACK_CARD::NUM_OF_FADERS; cnt++)
      faders[cnt] = ofaders[cnt] = 0; 
       
    setContrast(PLAYBACK_CARD::DEFAULT_CONTRAST); // set all displays to default constrast

    lcd[0].init(lcd1_addr);    
  
#if defined(PLAYBACK_1K_CARD_TESTING)
    lcd[0].print("Jands Playback Card - LCD 1 - Init OK ");      
#endif

    return detected; // success

}

// configure card address, and addresses of devices on this card
void playbackCard::setCardAddress(uint8_t addr) {
    card_addr = addr; // IC3 & IC5
    lcd1_addr = card_addr | 0x00;     // [rw] LCD0A      
    contrast_addr = card_addr | 0x05; // [w] IC8 -- upper bits are something else?    
};


// lower bits on IC8 
void playbackCard::setContrast(uint8_t val, uint8_t lcd = 0) {
    if (!detected) {
        return;  // card is not present, don't update...
    }
    val = val & 0b00001111;  //clamp
    if (lcd == 0) {  // main lcd      
      selectAddr(contrast_addr); 
      ocontrast[0]= 0x0F & val;       
      writeData(ocontrast[0]);  // default contrast      
    } else {
      // invalid input
#if defined(playback_1K_CARD_TESTING)
      Serial.println("playbackCard::setContrast: Invalid LCD specified.");
#endif       
    }
};


// read fader number cnt from mux address
uint8_t playbackCard::read_card_mux_fader(uint8_t mux, uint8_t cnt){
#if defined(FADER_AVERAGING)    
  uint8_t avg[3];
  for (uint8_t c2 = 0; c2 < 3 ; c2++){
#endif      
    writeMux(mux + cnt);    
    selectAddr(card_addr | 0x04);  // adc start conversion
    clk_ds();
    selectAddr(card_addr | 0x04);  // adc read
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


// Echelon Playback Card IC17 should return %01100010
uint8_t playbackCard::getCardId(void) {
    selectAddr(card_addr | 0x0F); // Card ID
    card_id = readData();
    return card_id;    
}

// Echelon 1K playback Card Driver
// Returns: true on change detected
bool playbackCard::update(bool check_faders_now = true)
{
  if (!detected) {
      return detected;  // card is not present, don't update...
  }

  // buttons and LEDS
  selectAddr(card_addr | 0x0A); //
  buttons[0] = readData();        // read 8 buttons
  writeData(leds[0]);           // IC18

  selectAddr(card_addr | 0x0B); //
  buttons[1] = readData();        // read 8 buttons
  writeData(leds[1]);           // IC19

  selectAddr(card_addr | 0x0C); //
  buttons[2] = readData();        // read 8 buttons
  writeData(leds[2]);           // IC20

  selectAddr(card_addr | 0x0D); //
  buttons[3] = readData();        // read 8 buttons
  writeData(leds[3]);           // IC21

  selectAddr(card_addr | 0x0E); //
  buttons[4] = readData();        // read 4 buttons
  writeData(leds[4]);           // IC22

  bool fc = false;

 if (check_faders_now){
  // read 8 faders.,,
      for (uint8_t cnt = 0; cnt < PLAYBACK_CARD::NUM_OF_FADERS; cnt++)
    {      
      if (cnt < 8){
      faders[cnt] = read_card_mux_fader(0x00,cnt); // faders 1-8
      } else {
      faders[cnt] = read_card_mux_fader(0x08,0); // grand master fader (if present)      
      }

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

  // look for button changes
  for (uint8_t n = 0; n < PLAYBACK_CARD::NUM_OF_BUTTON_BYTES; n++)
  {
    if (buttons[n] != obuttons[n])
      {
        fc = true;
        obuttons[n] = buttons[n];
      }
  }

#if defined(PLAYBACK_1K_CARD_TESTING)
    // light up some leds when pressed buttons
    leds[0] = buttons[0];
    leds[1] = buttons[1];
    leds[2] = buttons[2];
    leds[3] = buttons[3];
    leds[4] = buttons[4];
        
    if (fc)
    {    
        lcd[0].setCursor(0, 1);
        lcd[0].printf(" %03d  %03d  %03d  %03d  %03d  %03d  %03d  %03d", faders[0], faders[1], faders[2], faders[3], faders[4], faders[5], faders[6], faders[7]);        
//      Serial.printf("B0=0x%2x B1=0x%2x B2=0x%2x B3=0x%2x B4=0x%2x\n\r", buttons[0], buttons[1], buttons[2], buttons[3], buttons[4]);      
    }
#endif
 
 return fc; // return true if something changed
} 

