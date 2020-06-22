#ifndef _HARDWARE_H
#define _HARDWARE_H



// comment out for using arduino pin mapping where, on boards where the data bus is not consective pins (like Leonardo)
#define NO_PIN_MAPPING // use direct port commands

#define PORT_DELAY 3 // delay before read and after write
#define LCD_DELAY 40 // delay on LCD commands


// arduino pin numbers - teensy pin names

// data bus are pins 0..7
#define BUS 0 // first of 8 consecutive bus pins                D0-D7 on teensy++ 2.0

// master bus transiever - 74hc245)
#define DIR 8 // bus direction, low = read, high = write        E0 on teensy++ 2.0
#define BUF 9 // bus enable, low is enabled (usually keep low)  E1 on teensy++ 2.0

// bus signals
#define MUX 10 // aka ALEH, normally high                C0 on teensy++2.0
#define ALE 11 // normally high                          C1 on teensy++2.0
#define DS 12  // pulse to select device                 C2 on teensy++2.0
#define RW 13  // low (normal) is write, high is read    C3 on teensy++2.0

/* 
   So we maps to the 0-13 pins to the correct processor pin for Arduino in use.

   If NO_PIN_MAPPING is defined we use direct port commands

   Note: pins 0 and 1 are used by typical Uno Arduinos etc as the serial UART.
   Some USB Arduinos don't use these pins, and these are the ones we prefer!

*/

// write data bus byte
void inline writed(uint8_t inb)
{
#ifndef NO_PIN_MAPPING
  digitalWrite(0, inb & (1 << 0));
  digitalWrite(1, inb & (1 << 1));
  digitalWrite(2, inb & (1 << 2));
  digitalWrite(3, inb & (1 << 3));
  digitalWrite(4, inb & (1 << 4));
  digitalWrite(5, inb & (1 << 5));
  digitalWrite(6, inb & (1 << 6));
  digitalWrite(7, inb & (1 << 7));
#else
  PORTD = inb;
  delayMicroseconds(PORT_DELAY);
#endif
}

// read data bus byte
uint8_t inline readd()
{
#ifndef NO_PIN_MAPPING
  int bin[8];
  bin[0] = digitalRead(0);
  bin[1] = digitalRead(1);
  bin[2] = digitalRead(2);
  bin[3] = digitalRead(3);
  bin[4] = digitalRead(4);
  bin[5] = digitalRead(5);
  bin[6] = digitalRead(6);
  bin[7] = digitalRead(7);
  return (128 * bin[7] + 64 * bin[6] + 32 * bin[5] + 16 * bin[4] + 8 * bin[3] + 4 * bin[2] + 2 * bin[1] + bin[0]);
#else
  delayMicroseconds(PORT_DELAY);
  return PIND;
#endif
}

// change bus port pin direction
// accept INPUT or OUTPUT
void inline dirb(int dir)
{
#ifndef NO_PIN_MAPPING
  for (int c = 0; c < 8; c++)
    pinMode(c, dir); // change to inputs
#else
  DDRD = (dir ? 0xff : 0x00);
  delayMicroseconds(PORT_DELAY);
#endif
}

// write bus control signals
void inline writec(uint8_t inb)
{
  digitalWrite(DIR, inb & (1 << 0));  // PORT E
  digitalWrite(BUF, inb & (1 << 1)); 
  digitalWrite(MUX, inb & (1 << 2));  // PORT C
  digitalWrite(ALE, inb & (1 << 3));
  digitalWrite(DS, inb & (1 << 4));
  digitalWrite(RW, inb & (1 << 5));
  delayMicroseconds(PORT_DELAY);
}

// mux address latch pulse
void inline clk_mux()
{
  digitalWrite(MUX, 0);
  delayMicroseconds(PORT_DELAY);
  digitalWrite(MUX, 1);
}

// address latch pulse
void inline clk_ale()
{
  digitalWrite(ALE, 0);
  delayMicroseconds(PORT_DELAY);
  digitalWrite(ALE, 1);
}

// device select pulse
void inline clk_ds()
{
  digitalWrite(DS, 1);
  delayMicroseconds(PORT_DELAY);
  digitalWrite(DS, 0);
}

// device select pulse with delay, primarily for the slow LCD chipset
void inline clk_ds_lcd()
{
  digitalWrite(DS, 1);
  delayMicroseconds(LCD_DELAY);
  digitalWrite(DS, 0);
}

// in order to make multiple writes to same address more efficient in loops, we use this to only
// latch an address if it has changed, otherwise do nothing.
// note: databus is always left in output state after this call
void inline selectAddr(uint8_t addr)
{
  static uint8_t reg_last_addr;

  dirb(OUTPUT); // output to bus
  if (addr != reg_last_addr)
  {               // only select address if not the same as last address selected.
    writec(0x0d); // DIR HIGH, MUX HIGH, ALE HIGH
    writed(addr); //
    clk_ale();
    reg_last_addr = addr;
  }
}

// write a byte to the device (at a previously selected address)
void inline writeData(uint8_t data)
{
  writed(data);
  clk_ds();
}

// write a byte to the bus and clock the ALEH (Mux) line
void inline writeMux(uint8_t data)
{
  writed(data);
  clk_mux();
}

// write a byte to the device at a previously selected address, with a small delay in the clock pulse
void inline writeDataDelay(uint8_t data)
{
  writed(data);
  clk_ds_lcd();
}

// flip the data bus direction, select the current device, and read it!
// always returns with bus in an output configuration
// RETURN 8-bit value read at current address/device
uint8_t inline readData()
{
  uint8_t result = 0;

  // read the 8 buttons at this address
  digitalWrite(DIR, 0); // DIR LOW // change bus direction to input
  digitalWrite(RW, 1);  // RW HIGH // change CardBus RW to Read
  dirb(INPUT);          // change pins to inputs

  digitalWrite(DS, 1); // DS high
  result = readd();    // read byte from card_address
  digitalWrite(DS, 0); //DS low

  digitalWrite(DIR, 1); // DIR HIGH // change bus direction to output
  digitalWrite(RW, 0);  // RW LOW   // change CardBus RW to Write
  dirb(OUTPUT);         // change data bus pins to outputs

  return result;
}

#endif // _HARDWARE_H
