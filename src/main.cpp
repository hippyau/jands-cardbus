/* 
 ██████╗ ██████╗ ███╗   ██╗███████╗██╗ ██████╗ 
██╔════╝██╔═══██╗████╗  ██║██╔════╝██║██╔════╝ 
██║     ██║   ██║██╔██╗ ██║█████╗  ██║██║  ███╗
██║     ██║   ██║██║╚██╗██║██╔══╝  ██║██║   ██║
╚██████╗╚██████╔╝██║ ╚████║██║     ██║╚██████╔╝
 ╚═════╝ ╚═════╝ ╚═╝  ╚═══╝╚═╝     ╚═╝ ╚═════╝                                               
*/

#define TESTING // buttons light their LED when pressed

// card addresses
#define ADDR_PRESET_1 (0x00)
#define ADDR_PRESET_2 (0x10)
#define ADDR_MASTER (0x80)

#define ADDR_PALETTE (0x9D) // fix these
#define ADDR_ASSIGN (0xCE)

/*
    ██╗███╗   ██╗ ██████╗██╗     ██╗   ██╗██████╗ ███████╗███████╗    
    ██║████╗  ██║██╔════╝██║     ██║   ██║██╔══██╗██╔════╝██╔════╝    
    ██║██╔██╗ ██║██║     ██║     ██║   ██║██║  ██║█████╗  ███████╗    
    ██║██║╚██╗██║██║     ██║     ██║   ██║██║  ██║██╔══╝  ╚════██║    
    ██║██║ ╚████║╚██████╗███████╗╚██████╔╝██████╔╝███████╗███████║    
    ╚═╝╚═╝  ╚═══╝ ╚═════╝╚══════╝ ╚═════╝ ╚═════╝ ╚══════╝╚══════╝                                                                       
*/

#include <Arduino.h>
#include <inttypes.h>
#include "Print.h" // we lever this into use in the LCD drivers

#include "hardware.h" // read and write to CardBus
#include "lcd.h"      // LCD modules

#ifdef TESTING
#include "debug.h"
#endif

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
  bool update();

  // inputs - 12 bytes
  uint8_t leds[12]; // 12 analog levels

  // outputs - 26 bytes
  uint8_t buttons[2]; // 12 buttons in 2 bytes
  uint8_t faders[24]; // 24 analog values

  presetCard(uint8_t base_addr = ADDR_PRESET_1) // constructor
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
  uint8_t obuttons[2];
  uint8_t ofaders[24]; // for comparisons
  uint8_t oleds[12];
};

static uint8_t ledtest = 0;

// Preset Card Driver
bool presetCard::update()
{

  // read the 24 faders.....
  /*
 faders layout 

 0xD8     0xB8         0x78
 1 3 5 7  9  11 13 15  17 19 21 23

 2 4 6 8  10 12 14 16  18 20 22 24
*/

  bool fc = 0; // flag a change has occured
  for (uint8_t cnt = 0; cnt < 8; cnt++)
  {
    writeMux(0xD8 + cnt);
    selectAddr(card_addr | 0x04);
    clk_ds();
    clk_ds();
    selectAddr(card_addr | 0x05);
    faders[cnt] = readData();
  }

  for (uint8_t cnt = 0; cnt < 8; cnt++)
  {
    writeMux(0xB8 + cnt);
    selectAddr(card_addr | 0x04);
    clk_ds();
    clk_ds();
    selectAddr(card_addr | 0x05);
    faders[cnt + 8] = readData();
  }

  for (uint8_t cnt = 0; cnt < 8; cnt++)
  {
    writeMux(0x78 + cnt);
    selectAddr(card_addr | 0x04);
    clk_ds();
    clk_ds();
    selectAddr(card_addr | 0x05);
    faders[cnt + 16] = readData();
  }

  // process two bytes of buttons
  for (uint8_t cnt = 0; cnt < 2; cnt++)
  {
    selectAddr(card_addr | (0x0E + cnt));
    buttons[cnt] = readData();
  }

  // TODO: SET LEDS FIX

  // leds[0] = 255; // -- testing only
  // leds[1] = 0;
  // leds[2] = 0;
  // leds[3] = 0;
  // leds[4] = 255;
  // leds[5] = 0;

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
#if defined(TESTING)
      // Serial.printf("P1-%d=0x%x\n", cnt, buttons[cnt]);
#endif
    }
  }

  for (uint8_t cnt = 0; cnt < 24; cnt++)
  {
    if (ofaders[cnt] != faders[cnt])
    {
      fc = true; // fader change
      ofaders[cnt] = faders[cnt];

#if defined(TESTING)
      // Serial.printf("F%02d:%02x ",cnt,faders[cnt]);

      //  if (cnt < 12) leds[cnt] = faders[cnt];
#endif
    }
  }

  return fc; // true if change
}

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
  bool update();

  uint8_t leds[2];
  uint8_t buttons[1];
  uint8_t faders[8];

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

private:
  uint8_t card_addr;
  uint8_t lcd_addr;
  uint8_t obuttons[1]; // for comparisons
  uint8_t ofaders[8];
};

bool assignCard::update()
{
  // buttons and LEDS
  selectAddr(card_addr); // first byte of LED data (R)
  writeData(leds[0]);
  buttons[0] = readData();   // read 8 buttons at this address
  selectAddr(card_addr + 1); // second byte of LED data (G)
  writeData(leds[1]);

  // read 8 faders.....
  // selectAddr(0xC4);
  bool fc = 0;
  for (uint8_t cnt = 0; cnt < 8; cnt++)
  {
    writeMux(0x40 + cnt);
    selectAddr(0xC4);
    clk_ds();
    clk_ds();
    selectAddr(0xC5);
    faders[cnt] = readData();

    if (faders[cnt] != ofaders[cnt])
    {
      ofaders[cnt] = faders[cnt];
      fc = true; // fader change
    }
  }

  // send changes
  if (buttons[0] != obuttons[0])
  {
    fc = true;
    obuttons[0] = buttons[0];
  }

#if defined(TESTING)
  leds[1] = buttons[0]; // light up pressed button in green LEDS
  if (fc)
  {
    // Serial.printf("AB=0x%x\n", buttons[0]);
    lcd.setCursor(0, 0);
    lcd.printf("Buttons: %x  ", buttons[0]);
    lcd.setCursor(0, 1);
    lcd.printf(" %03d  %03d  %03d  %03d  %03d  %03d  %03d  %03d", faders[0], faders[1], faders[2], faders[3], faders[4], faders[5], faders[6], faders[7]);
  }
#endif

  return fc;
}

/*
██████╗  █████╗ ██╗     ███████╗████████╗████████╗███████╗     ██████╗ █████╗ ██████╗ ██████╗ 
██╔══██╗██╔══██╗██║     ██╔════╝╚══██╔══╝╚══██╔══╝██╔════╝    ██╔════╝██╔══██╗██╔══██╗██╔══██╗
██████╔╝███████║██║     █████╗     ██║      ██║   █████╗      ██║     ███████║██████╔╝██║  ██║
██╔═══╝ ██╔══██║██║     ██╔══╝     ██║      ██║   ██╔══╝      ██║     ██╔══██║██╔══██╗██║  ██║
██║     ██║  ██║███████╗███████╗   ██║      ██║   ███████╗    ╚██████╗██║  ██║██║  ██║██████╔╝
╚═╝     ╚═╝  ╚═╝╚══════╝╚══════╝   ╚═╝      ╚═╝   ╚══════╝     ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═════╝                                                                                                                                            
*/

/* PaletteCard is 20 buttons and 20 red LEDS, spread across 3 bytes
*/
class paletteCard
{
public:
  bool update();

  uint8_t leds[3];
  uint8_t buttons[3];

  paletteCard()
  {
    card_addr = ADDR_PALETTE; // default base address for a palette card, can change latter
    leds[0] = leds[1] = 0;
    buttons[0] = buttons[1] = buttons[2] = 0;
    old_buttons[0] = old_buttons[1] = old_buttons[2] = 0;
  }

private:
  uint8_t card_addr;
  uint8_t old_buttons[3];
};

// Palette Card Driver
// input - 3 bytes of LEDs (20 LED's all up)
// output - 3 bytes of buttons (20 buttons)
// return true on change detected
bool paletteCard::update()
{
  bool fc = 0; // flag a change has occured

  // process three banks of leds & buttons
  for (uint8_t cnt = 0; cnt < 3; cnt++)
  {
    selectAddr(card_addr + cnt);
    writeData(leds[cnt]);
    buttons[cnt] = readData();
  }

  // check for changes
  for (uint8_t cnt = 0; cnt < 3; cnt++)
  {
    if (buttons[cnt] != old_buttons[cnt])
    {
      fc = true;
      old_buttons[cnt] = buttons[cnt];

#if defined(TESTING)
      leds[cnt] = buttons[cnt]; /* feed the button data back to the LEDs */
                                //Serial.printf("PB_%d=0x%x\n", cnt, buttons[cnt]);
#endif
    }
  }

  return fc;
}

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
  bool update();

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
      wheels[cnt] = 0;
    lcd.init(lcd_addr);
    lcd.print("Master!");
  }

private:
  uint8_t card_addr;
  uint8_t lcd_addr;
  uint8_t obuttons[5];
  uint8_t ofaders[8]; // for comparisons
  uint8_t owheels[3];
};

// Master Card Driver
// returns true on a change
bool masterCard::update()
{

  // buttons and LEDS
  selectAddr(card_addr | 0x0A); // SW2
  buttons[0] = readData();      // read 8 buttons
  selectAddr(card_addr | 0x0B); // SW3
  buttons[1] = readData();
  selectAddr(card_addr | 0x0C); // SW4
  buttons[2] = readData();
  selectAddr(card_addr | 0x0D); // SW5 / LED5
  buttons[3] = readData();
  writeData(leds[0]);           // write 8 LEDS
  selectAddr(card_addr | 0x0E); // SW6 / LED6
  buttons[4] = readData();      // read 8 buttons at this address
  writeData(leds[1]);
  selectAddr(card_addr | 0x0F); // LED7
  writeData(leds[2]);

  // encoder wheels
  wheels[2] = 0x0f & readData(); // top 4 bits are card ID
  selectAddr(card_addr | 0x02);  // wheels 1 and 2
  wheels[0] = readData();
  wheels[1] = (wheels[0] >> 4) & 0x0f; // high nibble
  wheels[0] = wheels[0] & 0x0f;        // low nibble

  // read the 8 faders.....
  selectAddr(card_addr | 0x04);
  bool fc = 0;
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
  for (uint8_t n = 0; n < 8; n++)
  {
    if (ofaders[n] != faders[n])
    {
      fc = true; // flag change
      ofaders[n] = faders[n];
    }
    for (uint8_t n = 0; n < 3; n++)
    {
      if (owheels[n] != wheels[n])
      {
        fc = true;
        owheels[n] = wheels[n];
      }
    }
    for (uint8_t n = 0; n < 5; n++)
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
  }

} // end master card update

/*
     ██╗ █████╗ ███╗   ██╗██████╗ ███████╗     ██████╗ █████╗ ██████╗ ██████╗     ██████╗ ██╗   ██╗███████╗
     ██║██╔══██╗████╗  ██║██╔══██╗██╔════╝    ██╔════╝██╔══██╗██╔══██╗██╔══██╗    ██╔══██╗██║   ██║██╔════╝
     ██║███████║██╔██╗ ██║██║  ██║███████╗    ██║     ███████║██████╔╝██║  ██║    ██████╔╝██║   ██║███████╗
██   ██║██╔══██║██║╚██╗██║██║  ██║╚════██║    ██║     ██╔══██║██╔══██╗██║  ██║    ██╔══██╗██║   ██║╚════██║
╚█████╔╝██║  ██║██║ ╚████║██████╔╝███████║    ╚██████╗██║  ██║██║  ██║██████╔╝    ██████╔╝╚██████╔╝███████║
 ╚════╝ ╚═╝  ╚═╝╚═╝  ╚═══╝╚═════╝ ╚══════╝     ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═════╝     ╚═════╝  ╚═════╝ ╚══════╝
*/

// the whole control surface
class JandsCardBus
{
public:
  presetCard preset1;
  presetCard preset2;
  paletteCard palette;
  assignCard assign;
  masterCard master;

  void update();
};

// update each card in turn,
void inline JandsCardBus::update()
{
  
  if (preset1.update()){
    // send update
  }

  preset2.update();
  palette.update();
  assign.update();
  master.update();  
}




/*
███████╗███████╗████████╗██╗   ██╗██████╗ 
██╔════╝██╔════╝╚══██╔══╝██║   ██║██╔══██╗
███████╗█████╗     ██║   ██║   ██║██████╔╝
╚════██║██╔══╝     ██║   ██║   ██║██╔═══╝ 
███████║███████╗   ██║   ╚██████╔╝██║     
╚══════╝╚══════╝   ╚═╝    ╚═════╝ ╚═╝     
*/

// pointer to our entire control surface
static JandsCardBus *surface = NULL;

void setup()
{
  Serial.begin(115200);
  for (int c = 0; c < 14; c++)
    pinMode(c, OUTPUT);
  while (Serial.read() >= 0)
    ; // flush serial input buffers

  surface = new JandsCardBus(); // create the new surface class
  surface->preset1.setCardAddress(ADDR_PRESET_1);
  surface->preset2.setCardAddress(ADDR_PRESET_2);
}

/* 
███╗   ███╗ █████╗ ██╗███╗   ██╗
████╗ ████║██╔══██╗██║████╗  ██║
██╔████╔██║███████║██║██╔██╗ ██║
██║╚██╔╝██║██╔══██║██║██║╚██╗██║
██║ ╚═╝ ██║██║  ██║██║██║ ╚████║
╚═╝     ╚═╝╚═╝  ╚═╝╚═╝╚═╝  ╚═══╝                               
 */
void loop()
{

  surface->update();

  surface->preset1.leds[0] = surface->assign.faders[0];
  surface->preset1.leds[1] = surface->assign.faders[1];

#ifdef TESTING
  fps();
#endif
}
