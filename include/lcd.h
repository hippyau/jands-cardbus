#ifndef _LCD_CARDBUS_H
#define _LCD_CARDBUS_H


#define LCD_COLUMNS 40
#define LCD_ROWS    2


/* LCD instruction set */
// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00





/*
██╗      ██████╗██████╗ 
██║     ██╔════╝██╔══██╗
██║     ██║     ██║  ██║
██║     ██║     ██║  ██║
███████╗╚██████╗██████╔╝
╚══════╝ ╚═════╝╚═════╝                                                          
*/            
class lcdModule : public Print
{
public:
  void init(uint8_t address);
  void setContrast(uint8_t value);  
  void setCursor(uint8_t col, uint8_t row);

  // can use lcd.print, lcd.println, lcd.printf() etc
  virtual size_t write(uint8_t); 
  using Print::write;

  lcdModule() // constructor
  {
   contrast = 0x05;
   lcd_addr = 0;
   numlines = LCD_ROWS;
   row_offset[0] = 0;
   row_offset[1] = 0x40;
  }

private:
  void write_cmd(uint8_t command);
  void write_data(uint8_t data);
  uint8_t contrast;
  uint8_t lcd_addr;
  uint8_t numlines;
  uint8_t row_offset[2];
};

// initialize LCD at given bus address and turn it on
void lcdModule::init(uint8_t address)
{
  lcd_addr = address;

  selectAddr(lcd_addr);
//#if defined(TESTING)
//  writeData(LCD_FUNCTIONSET | LCD_8BITMODE | LCD_1LINE );
//#else
  writeData(LCD_FUNCTIONSET | LCD_8BITMODE | LCD_2LINE );
//#endif
  delay(5);
  writeData(LCD_CLEARDISPLAY);
  delay(2);
  writeData(LCD_RETURNHOME);
  delay(2);
  writeData(0x0F); // display on, cursor on, blinking on
  setContrast(contrast);
}

// locate the cursor
void lcdModule::setCursor(uint8_t col, uint8_t row)
{
  if (row < LCD_ROWS)
    write_cmd(LCD_SETDDRAMADDR | (col + row_offset[row]));
}  

// write a cmd byte to the LCD
void inline lcdModule::write_cmd(uint8_t lcd_command)
{
  selectAddr(lcd_addr);
  writeDataDelay(lcd_command);
}

// write a data byte to the LCD
void inline lcdModule::write_data(uint8_t lcd_data)
{
  selectAddr(lcd_addr + 1);
  writeDataDelay(lcd_data);
}

// for printXX commands
inline size_t lcdModule::write(uint8_t value)
{
  write_data(value);
  return 1; // assume sucess
}

// sets the DAC to control LCD contrast.
void lcdModule::setContrast(uint8_t value)
{
  selectAddr(lcd_addr + 2);
  writeData(value);
}


#endif