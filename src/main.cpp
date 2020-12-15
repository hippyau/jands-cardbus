#include <Arduino.h>
/*  
       _                 _     
      | |               | |    
      | | __ _ _ __   __| |___ 
  _   | |/ _` | '_ \ / _` / __|
 | |__| | (_| | | | | (_| \__ \
  \____/ \__,_|_| |_|\__,_|___/
                               
 - Jands CardBus - Example App
  
 (c) hippy 2020
*/

#define APP_VERSION  ("0.61 (Event4, Echelon)")

// configuration

//#define USE_ETHERNET

// General Debug
#define TESTING
#define FADER_TESTING
//#define TEST_MENUS // testing menu system

#define SERIAL_CLI_ENABLED // serial command line interface

#define SURFACE_CLI_ENABLED // use the keys on the surface and form a command line interface. 

// Event 4xx
#define ASSIGN_CARD_LCD_TESTING (true) // fader values on LCD line 1
#define PRESET_LEDS_TESTING (true)     // LED's mimic faders
#define MASTER_CARD_TESTING (true)     // display info on the master card LCD

// Echelon 1K
#define PROGRAM_1K_CARD_TESTING (true)
#define PLAYBACK_1K_CARD_TESTING (true)
#define MENU_1K_CARD_TESTING (true)

// CardBus driver
#include <JandsCardBus.h>

#if defined(USE_ETHERNET)
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#define UDP_TX_PACKET_MAX_SIZE 256                           //increase UDP size to 256 bytes
static uint8_t mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; // Our MAC address
static uint16_t localPort = 8888;                            // Port to listen on
static IPAddress ip(192, 168, 1, 88);                        // Our IP address
static IPAddress trg(192, 168, 1, 49);                       // Where we are sending to
static EthernetUDP Udp;
#endif

#if defined(TEST_MENUS)
//  Menu system
#include "LiquidMenu.h"
#endif

#if defined(TESTING)
#include "debug.h"
#endif

#if defined(SERIAL_CLI_ENABLED)
#include "Cmd.h"
#endif

#if defined(SURFACE_CLI_ENABLED)
#include "surface_cmd_line.h" 
#endif

#if defined(TEST_MENUS)
// pointer to our menu system
static LiquidMenu *Menu;
#endif

// send surface state packet to host(s)
void inline sendSurfaceState()
{

#ifdef USE_ETHERNET

  // construct UDP frame with all the surface values
  Udp.beginPacket(trg, 8888);
  Udp.write("JCB0"); // header

  // faders x 64 bytes
  for (uint8_t cnt = 0; cnt < 24; cnt++)
    Udp.write(Surface->preset1.faders[cnt]);
  for (uint8_t cnt = 0; cnt < 24; cnt++)
    Udp.write(Surface->preset2.faders[cnt]);
  for (uint8_t cnt = 0; cnt < 8; cnt++)
    Udp.write(Surface->assign.faders[cnt]);
  for (uint8_t cnt = 0; cnt < 8; cnt++)
    Udp.write(Surface->master.faders[cnt]); // fader 8 is self test

  // buttons x 13 bytes - includes some Card ID
  for (uint8_t cnt = 0; cnt < 2; cnt++)
    Udp.write(Surface->preset1.buttons[cnt]);
  for (uint8_t cnt = 0; cnt < 2; cnt++)
    Udp.write(Surface->preset2.buttons[cnt]);
  for (uint8_t cnt = 0; cnt < 1; cnt++)
    Udp.write(Surface->assign.buttons[cnt]);
  for (uint8_t cnt = 0; cnt < 3; cnt++)
    Udp.write(Surface->palette.buttons[cnt]);
  for (uint8_t cnt = 0; cnt < 5; cnt++)
    Udp.write(Surface->master.buttons[cnt]);

  // wheels x 3 - up/down counters 0x00 -- 0x0F
  for (uint8_t cnt = 0; cnt < 3; cnt++)
    Udp.write(Surface->master.wheels[cnt]);

  Udp.endPacket(); // send frame
#endif
}

// take input from host(s)
void hostSetSurfaceState()
{

#if defined(USE_ETHERNET)
  static char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet,
  static int packetSize;
  static IPAddress remote;

  packetSize = Udp.parsePacket();
  if (packetSize)
  {
    // Serial.print("Received packet of size ");
    // Serial.println(packetSize);
    // Serial.print("From ");

    remote = Udp.remoteIP();
    // for (int i =0; i < 4; i++)
    // {
    //   Serial.print(remote[i], DEC);
    //   if (i < 3)
    //   {
    //     Serial.print(".");
    //   }
    // }
    //Serial.print(", port ");
    //Serial.println(Udp.remotePort());

    // read the packet into packetBufffer
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);

    if (strcmp(packetBuffer, "JCBL") == 0)
    {
      // legit LED packet

      //Serial.println("Contents:");
      //Serial.println(packetBuffer);
    }
    else if (strcmp(packetBuffer, "JCBD") == 0)
    {
      // legit LCD packet - payload is char card,x,y,length,string+NULL

      uint8_t card_address = packetBuffer[4];
      //uint8_t x = packetBuffer[5];
      //uint8_t y = packetBuffer[6];
      uint8_t len = packetBuffer[7];
      packetBuffer[8 + len] = 0; // terminate the string
      char *message = &packetBuffer[8];

      if (len > 0)
      {
        switch (card_address)
        {
        case ADDR_ASSIGN:

          Surface->assign.lcd.setCursor(packetBuffer[5], packetBuffer[6]);
          Surface->assign.lcd.print(message);
          break;

        case ADDR_MASTER:
          Surface->master.lcd.setCursor(packetBuffer[5], packetBuffer[6]);
          Surface->master.lcd.print(message);
          break;

        default:
          break;
        }
      }

      else
      { // empty message means clear display
        switch (card_address)
        {
        case ADDR_ASSIGN:
          Surface->assign.lcd.clear();
          break;

        case ADDR_MASTER:
          Surface->master.lcd.clear();
          break;

        default:
          break;
        }
      };

      //Serial.println("Contents:");
      //Serial.println(packetBuffer);
    }
  }

#endif
}

static uint8_t val = 0;
static uint8_t val2 = 0;

#if defined(TEST_MENUS)
// menu system
/*
 * LiquidLine objects represent a single line of text and/or variables
 * on the display. The first two parameters are the column and row from
 * which the line starts, the rest of the parameters are the text and/or
 * variables that will be printed on the display. They can be up to four.
 */
// Here the line is set to column 1, row 0 and will print the passed
// string and the passed variable.
LiquidLine welcome_line1(1, 0, "Jands CardBus Control Surface ", "V0.2");
// Here the column is 3, the row is 1 and the string is "Hello Menu".
LiquidLine welcome_line2(3, 1, "Setup");

/*
 * LiquidScreen objects represent a single screen. A screen is made of
 * one or more LiquidLine objects. Up to four LiquidLine objects can
 * be inserted from here, but more can be added later in setup() using
 * welcome_screen.add_line(someLine_object);.
 */
// Here the LiquidLine objects are the two objects from above.
LiquidScreen welcome_screen(welcome_line1, welcome_line2);

// Here there is not only a text string but also a changing integer variable.
LiquidLine analogReading_line(0, 0, "Analog: 1 ", val);
LiquidLine analogReading_line2(0, 1, "Analog: 2 ", val2);
LiquidLine analogReading_line3(0, 2, "Analog: 2 ", val2);

LiquidScreen secondary_screen(analogReading_line, analogReading_line2, analogReading_line3);
#endif

// modifier 'Shift' button is down?
#define buttonShift (_sbuttons[BTN_SHIFT])

static SButton buttonSetup(BTN_Setup);
static SButton buttonLeft(BTN_LEFT);
static SButton buttonRight(BTN_RIGHT);
static SButton buttonPlus(BTN_PLUS);
static SButton buttonMinus(BTN_MINUS);

static keypad_input Key;
static unsigned int input_value = 123;


#if defined(SERIAL_CLI_ENABLED)

void cmd_help(int arg_cnt, char **args)
{
  cmdGetStream()->println("Available commands: (command with arguments provide usage help when no arguments given)");
  cmdGetStream()->println();
  cmdGetStream()->println("version - print firmware version");
  cmdGetStream()->println("run     - enable cardbus update loop");
  cmdGetStream()->println("stop    - disable cardbus update loop");
  cmdGetStream()->println("stat    - print cardbus statistics");
  cmdGetStream()->println("reboot  - reboot");        
  cmdGetStream()->println();

  cmdGetStream()->println("Debug commands:");
  cmdGetStream()->println();
  cmdGetStream()->println("set     - select hex bus [address 0x00..0xFF] (aka ALEL) where high nibble is card addres, low nibble is device on the card");
  cmdGetStream()->println("mux     - set a mux [hex address] (aka ALEH) for the currently selected card & device");
  cmdGetStream()->println("write   - select hex bus [address] and write hex [data]");
  cmdGetStream()->println("read    - read bus at current address, or option [hex address] also selects device ");
     
}


// ----  bus control CLI

// warning that other code is playing with the bus addresses...
void cmd_warn_bus_running() {
  cmdGetStream()->println("Warning: Bus running in background.  Use 'stop' to halt bus background updates.");
}

// set bus direction
void cmd_dirb(int arg_cnt, char **args) {
  if (Surface->halt == false) cmd_warn_bus_running();

  Stream *s = cmdGetStream();
  if (arg_cnt < 2){
    s->println("Usage: dirb [io] - set bus master direction to (i)n or (o)ut.");
    return;    
  }

  if (args[1][0] == 'i') {
    dirb(INPUT);    
  } else 
  if (args[1][0] == 'o') {
    dirb(OUTPUT);    
  } else
  {
    s->println("dirb: Invalid Argument.");
  }
}

// write to bus
void cmd_buswrite(int arg_cnt, char **args) {
  if (Surface->halt == false) cmd_warn_bus_running();

  Stream *s = cmdGetStream();
  if (arg_cnt < 3){
    s->println("Usage: write [hex data] [hex address] - write data to a bus address");
    s->println("       if [hex address] is not specified, last selected is used");
    return;    
  }

  uint8_t data = strtol(args[1], NULL, 16);

  if (arg_cnt < 2) {  
    s->printf("Write 0x%02X @ 0x%02X\n",data, reg_last_addr);
    writeData(data);
    return;
  }

  uint8_t addr = strtol(args[2], NULL, 16);
  
  s->printf("Write 0x%02X @ 0x%02X\n", data, addr);
  selectAddr(addr);
  writeData(data);
}


// write to bus mux
void cmd_busmux(int arg_cnt, char **args) {
  if (Surface->halt == false) cmd_warn_bus_running();

  Stream *s = cmdGetStream();
  if (arg_cnt < 2){
    s->println("Usage: mux [hex address] - select a mux address for the currently selected address");
    return;    
  }
  uint8_t data = strtol(args[1], NULL, 16);
  
  s->printf("mux 0x%02X @ 0x%02X\n",data, reg_last_addr);

  writeMux(data);

}

// read from bus - if no arg suppled, current address is used
void cmd_busread(int arg_cnt, char **args) {
  if (Surface->halt == false) cmd_warn_bus_running();

  Stream *s = cmdGetStream();
  if (arg_cnt == 1){
    s->printf("read: 0x%02X @ 0x%02X\n", readData(), reg_last_addr);  
    return;    
  }

 if (arg_cnt == 2){
    uint8_t addr = strtol(args[1], NULL, 16);
    selectAddr(addr);
    s->printf("read: 0x%02X @ 0x%02X\n", readData(), reg_last_addr);  
    return;    
 }

  s->println("read: Invalid Arguments.");
  s->println("Usage: read [hex address]  - read data from data, with optional address where last used is default.");
  
}


// set bus address
void cmd_set(int arg_cnt, char **args) {
  if (Surface->halt == false) cmd_warn_bus_running(); 

  Stream *s = cmdGetStream();

  if (arg_cnt == 1){
    s->printf("set: @ 0x%02X\n", reg_last_addr);  
    return;    
  }

 if (arg_cnt > 2){
    s->println("set: Invalid Arguments.");
    return;    
  }

 uint8_t addr = strtol(args[1], NULL, 16);
 reg_last_addr = !addr;
 selectAddr(addr);
 
 s->printf("set: selected @ 0x%02X\n", reg_last_addr);  
 
}

// print some stats
void cmd_stat(int arg_cnt, char **args){
  Stream *s = cmdGetStream();
  s->println("Stats: ");
#if defined (TESTING)
  s->print("  Card Bus Updates/Second: ");
  s->println(UpdatesPerSecond);
#endif
}

// reboot teensy
void cmd_reboot(int arg_cnt, char **args){
  Stream *s = cmdGetStream();
  s->println("Soft Reboot...");  
  _restart_Teensyduino_();
}

// stop updates
void cmd_stop(int arg_cnt, char **args){
  Surface->halt = true;
  Stream *s = cmdGetStream();
  s->println("Surface updates halted.");  
}

// resume updates
void cmd_run(int arg_cnt, char **args){
  Surface->halt = false;
  Stream *s = cmdGetStream();
  s->println("Surface updates resumed.");  
}

#endif // SERIAL_CLI_ENABLED



/*
███████╗███████╗████████╗██╗   ██╗██████╗ 
██╔════╝██╔════╝╚══██╔══╝██║   ██║██╔══██╗
███████╗█████╗     ██║   ██║   ██║██████╔╝
╚════██║██╔══╝     ██║   ██║   ██║██╔═══╝ 
███████║███████╗   ██║   ╚██████╔╝██║     
╚══════╝╚══════╝   ╚═╝    ╚═════╝ ╚═╝     
*/

void setup()
{

  Serial.begin(115200);
  while (Serial.read() >= 0)
    ; // flush serial input buffers

  for (int c = 0; c < 14; c++)
    // setup pins as outputs
    pinMode(c, OUTPUT);

#ifdef USE_ETHERNET
    Ethernet.begin(mac, ip);
    Udp.begin(localPort);
    delay(100);
#endif


#if defined(SERIAL_CLI_ENABLED)
  cmdInit(&Serial);

  cmdAdd("help", cmd_help);
  cmdAdd("run", cmd_run);
  cmdAdd("stop", cmd_stop);    
  cmdAdd("stat",cmd_stat);    
  cmdAdd("version", [](int argc, char **argv){ cmdGetStream()->print("Version "); cmdGetStream()->println(APP_VERSION); });
  cmdAdd("reboot",cmd_reboot);

  cmdAdd("dirb",cmd_dirb);
  cmdAdd("set",cmd_set);
  cmdAdd("write",cmd_buswrite);
  cmdAdd("read",cmd_busread);
  cmdAdd("mux",cmd_busmux);

#endif



  Surface = new JandsCardBus(); // create out new surface class
  // configure surface

  //  Event 408
  //  Surface->preset1.setCardAddress(ADDR_PRESET_1);  // discriminate two preset cards by address
  //  Surface->preset2.setCardAddress(ADDR_PRESET_2);

  // Echelon 1K -- detect and initialize cards
  Surface->menu1.init(ADDR_MENU_1_1K);
  Surface->menu2.init(ADDR_MENU_2_1K);
  Surface->playback1.init(ADDR_PLAYBACK_1_1K);
  Surface->playback2.init(ADDR_PLAYBACK_2_1K);
  Surface->program.init(ADDR_PROGRAM_1K);

  // map 1K program card keys to USB-HID keyboard
  if (Surface->program.detected)
  {
    Surface->keys._usb_key[140] = KEY_LEFT;
    Surface->keys._usb_key[141] = KEY_UP;
    Surface->keys._usb_key[142] = KEY_DOWN;
    Surface->keys._usb_key[143] = KEY_RIGHT;

    Surface->keys._usb_key[133] = KEYPAD_0;
    Surface->keys._usb_key[132] = KEYPAD_1;
    Surface->keys._usb_key[134] = KEYPAD_2;
    Surface->keys._usb_key[136] = KEYPAD_3;
    Surface->keys._usb_key[125] = KEYPAD_4;
    Surface->keys._usb_key[127] = KEYPAD_5;
    Surface->keys._usb_key[129] = KEYPAD_6;
    Surface->keys._usb_key[124] = KEYPAD_7;
    Surface->keys._usb_key[126] = KEYPAD_8;
    Surface->keys._usb_key[128] = KEYPAD_9;
    Surface->keys._usb_key[135] = KEYPAD_PERIOD;
    Surface->keys._usb_key[138] = '@';
    Surface->keys._usb_key[123] = KEYPAD_PLUS;
    Surface->keys._usb_key[121] = KEYPAD_MINUS;
    Surface->keys._usb_key[117] = KEY_BACKSPACE;
    Surface->keys._usb_key[130] = '>';
    Surface->keys._usb_key[131] = 'f';

    Surface->keys._usb_key[153] = MODIFIERKEY_LEFT_CTRL;
    Surface->keys._usb_key[99] = MODIFIERKEY_RIGHT_CTRL;

    Surface->keys._usb_key[137] = KEY_ENTER;
    Surface->keys._usb_key[97] = KEY_HOME;
    Surface->keys._usb_key[144] = KEY_END;
    Surface->keys._usb_key[145] = KEY_PAGE_UP;
    Surface->keys._usb_key[146] = KEY_PAGE_DOWN;

    Surface->keys._usb_key[152] = KEY_SPACE; // NEXT

    Surface->keys._usb_key[98] = KEY_ESC; // clear/restore

    Surface->keys._usb_key[162] = 'i';
    Surface->keys._usb_key[162] = 'd';
    Surface->keys._usb_key[163] = 'k';
    Surface->keys._usb_key[166] = 'f';
    Surface->keys._usb_key[167] = 'a';
  }

#if defined(TEST_MENUS)
  // master menu assigned to master card LCD
  Menu = new LiquidMenu(Surface->master.lcd);
  Menu->add_screen(welcome_screen);
  Menu->add_screen(secondary_screen);
#endif

  Key.edit(&Surface->assign.lcd, (char *)"Channel", &input_value, 1024, 0, 0);
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

  // update surface,  send whole state if there is a change
  if (Surface->update())
  {
    sendSurfaceState(); // has something on the surface changed?
  }
  
  hostSetSurfaceState();// process incomming host commands (if any)


  // testing keypad input
  while (Surface->keys.isKeyAvailable())
  {
    uint8_t keyp = Surface->keys.getKey();
    Serial.printf("Key: %i\n", (int)keyp);
    SurfaceCmdLine(keyp);
  }

  //if (Key.check()){ // enter or exit pressed
  //    Serial.printf("Val: %i\n",Key.currentValue());
  //    //Key.edit(&Surface->assign.lcd, (char*)"Level: ", &input_value, 1024, 0, line );
  //    Key.input(&Surface->assign.lcd, (char*)"Level", 0, 0 );
  //}

#if defined(TEST_MENUS)
  if (buttonRight.check())
  {
    Menu->next_screen();
    Menu->update();
  }

  if (buttonLeft.check())
  {
    Menu->previous_screen();
    Menu->update();
  }

  if (buttonPlus.check())
  {
    Menu->switch_focus(1);
    //Menu->call_function(1);
    Menu->update();
  }

  if (buttonMinus.check())
  {
    Menu->switch_focus(0);
    //Menu->call_function(2);
    Menu->update();
  }
#endif

// update the serial command line interface
#if defined(SERIAL_CLI_ENABLED)
  cmdPoll();
#endif

#if defined(TESTING)
  fps(); // measure updates per second

#if defined(FADER_TESTING)
  val = Surface->preset1.faders[0];
  val2 = Surface->preset1.faders[2];
#endif

#endif
}
