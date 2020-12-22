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

#define APP_VERSION  ("0.61 (Event4, Echelon 1K)")

// configuration

#define USE_ETHERNET

#define SERIAL_CLI_ENABLED // serial command line interface
#define SERIAL_CLI_BUSCONTROL // commands to manipulate the card bus


// ONLY DEFINE ONE OF THESE
#define CONFIG_ECHELON_1K  // use the echelon 1k surface, as manufactured
//#define CONFIG_EVENT_408   // use the Event408 surface, as manufactured
//#define CONFIG_CUSTOM    // future: use a mix/mash custom surface


// General Debug
#define TESTING
#define FADER_TESTING


#if defined (CONFIG_EVENT_408)
// Event 4xx card testing code is embedded in each card class
#define ASSIGN_CARD_LCD_TESTING (true) // fader values on LCD line 1
#define PRESET_LEDS_TESTING (true)     // LED's mimic faders
#define MASTER_CARD_TESTING (true)     // display info on the master card LCD
#endif

#if defined (CONFIG_ECHELON_1K)
// Echelon 1K
#define PROGRAM_1K_CARD_TESTING (true)
#define PLAYBACK_1K_CARD_TESTING (true)
#define MENU_1K_CARD_TESTING (true)
#endif


#if defined (CONFIG_EVENT_408)
#define SURFACE_CLI_ENABLED // use the keys on the surface and form a command line interface. Currently only Event4xx. 
#endif



#include <JandsCardBus.h>


#if defined(USE_ETHERNET)
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#define UDP_TX_PACKET_MAX_SIZE 256                           //increase UDP size to 256 bytes
static uint8_t mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; // Our MAC address
static uint16_t localPort = 8888;                            // Port to listen on

//static IPAddress ip(192, 168, 1, 88);                        // Our IP address
//static IPAddress trg(192, 168, 1, 49);                       // Where we are sending to

static IPAddress ip(169, 254, 1, 2);                        // Our IP address -- 169.254.254.1 is host, 2..254 are possible surfaces
static IPAddress trg(169, 254, 1, 1);                       // host Where we are sending to


static EthernetUDP Udp;
static bool eth0_up = false;                                 // update / use the ethernet interface
static uint32_t eth0_stats_tx = 0;                           // bytes
static uint32_t eth0_stats_rx = 0;            
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



// send surface state packet to host(s)
void inline sendSurfaceState()
{

#ifdef USE_ETHERNET
 if (eth0_up) {

#if defined (CONFIG_EVENT_408)
  // construct UDP frame with all the surface values for an Event408
  Udp.beginPacket(trg, 8888);
  Udp.write("JCB0"); // header
  uint16_t tx_bytes = 80; // TODO: Fix this!

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

  eth0_stats_tx += tx_bytes;

  Udp.endPacket(); // send frame
 }
#endif

#if defined (CONFIG_ECHELON_1K)
  // construct UDP frame with all the surface values for an Echelon 1K
  Udp.beginPacket(trg, 8888);
  Udp.write("JCB1"); // header
  uint16_t tx_bytes = 0; // TODO: Fix this!

  // faders x 17 
  for (uint8_t cnt = 0; cnt < 8; cnt++) { 
    Udp.write(Surface->playback1.faders[cnt]);
    tx_bytes += 1;
  }
  for (uint8_t cnt = 0; cnt < 8; cnt++) {
    Udp.write(Surface->playback2.faders[cnt]);
    tx_bytes += 1;
  }
  Udp.write(Surface->playback1.faders[9]); // grand master
  tx_bytes += 1;


  // buttons x 5 bytes  
  for (uint8_t cnt = 0; cnt < 5; cnt++) {
    Udp.write(Surface->playback1.buttons[cnt]);
    tx_bytes += 1;
  }

  // buttons x 5 bytes  
  for (uint8_t cnt = 0; cnt < 5; cnt++) {
    Udp.write(Surface->playback2.buttons[cnt]);
    tx_bytes += 1;
  }

  // buttons x 5 bytes  
  for (uint8_t cnt = 0; cnt < 5; cnt++) {
    Udp.write(Surface->menu1.buttons[cnt]);
    tx_bytes += 1;
  }

  // buttons x 5 bytes  
  for (uint8_t cnt = 0; cnt < 5; cnt++) {
    Udp.write(Surface->menu2.buttons[cnt]);
    tx_bytes += 1;
  }

  // buttons x 10 bytes  
  for (uint8_t cnt = 0; cnt < 10; cnt++) {
    Udp.write(Surface->program.buttons[cnt]);
    tx_bytes += 1;
  }


  // wheels x 3 - up/down counters 0x00 -- 0x0F
  for (uint8_t cnt = 0; cnt < 3; cnt++) {
    Udp.write(Surface->program.wheels[cnt]);
    tx_bytes += 1;
  }

  eth0_stats_tx += tx_bytes;

  Udp.endPacket(); // send frame
 }
#endif


#endif
}

// take input from host(s)
void hostSetSurfaceState()
{

#if defined(USE_ETHERNET)
if (eth0_up){

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


#if defined (CONFIG_EVENT_408)
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
#endif


    eth0_stats_rx += packetSize;
  }



} // eth0_up is trus
#endif
}

static uint8_t val = 0;
static uint8_t val2 = 0;



// modifier 'Shift' button is down?
#define buttonShift (_sbuttons[BTN_SHIFT])

#if defined (CONFIG_EVENT_408)
static SButton buttonSetup(BTN_Setup);
static SButton buttonLeft(BTN_LEFT);
static SButton buttonRight(BTN_RIGHT);
#endif
static SButton buttonPlus(BTN_PLUS);
static SButton buttonMinus(BTN_MINUS);


static keypad_input Key;
static unsigned int input_value = 255;



#if defined(SERIAL_CLI_ENABLED)

void cmd_help(int arg_cnt, char **args)
{
  cmdGetStream()->println("Available commands: (commands with arguments provide usage help when no arguments given)");
  cmdGetStream()->println();
  cmdGetStream()->println("version  - print firmware version");
  cmdGetStream()->println("run      - enable cardbus update loop");
  cmdGetStream()->println("stop     - disable cardbus update loop");
  cmdGetStream()->println("scan     - scan bus for cards");
  cmdGetStream()->println("stat     - print cardbus statistics");
#if defined (USE_ETHERNET)  
  cmdGetStream()->println("ifconfig - configure ethernet adapter [up]/[down]/[ip address] or [target]+[ip address](+[port])");
#endif
  cmdGetStream()->println("restart  - restart firmware");        
  cmdGetStream()->println("bootload - enter programming mode");        

#if defined (SERIAL_CLI_BUSCONTROL)
  cmdGetStream()->println();
  cmdGetStream()->println("Jands Card Bus commands:");
  cmdGetStream()->println();
  cmdGetStream()->println("set      - select hex bus [address 0x00..0xFF] (aka ALEL) where high nibble is card addres, low nibble is device on the card");
  cmdGetStream()->println("mux      - set a mux [hex address] (aka ALEH) for the currently selected card & device");
  cmdGetStream()->println("write    - select hex bus [address] and write hex [data]");
  cmdGetStream()->println("read     - read bus at current address, or option [hex address] also selects device ");   
#endif  
}

// Scan the card bus for cards, high nibble is card address, low nibble is 0x0F (card ID)
void cmd_scan_bus(int arg_cnt, char **args) {
  cmdGetStream()->println("Card Bus Scan: ");
  uint8_t cntr = 0;
  for (uint8_t cnt = 0; cnt < 16 ; cnt ++){
    uint8_t addr = (cnt << 4) | 0x0F;
    selectAddr(addr);
    uint8_t result = readData();
    if (addr == result) continue; 
    cmdGetStream()->printf("\tCard type %2x @ %2x\n\r", result, (cnt << 4));
    cntr++;
  }
  cmdGetStream()->printf("Scan complete, %d cards found.\n\r", cntr);
}


#if defined (SERIAL_CLI_BUSCONTROL)
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
  if (arg_cnt < 2){
    s->println("Usage: write [hex data] [hex address] - write data to a bus address");
    s->println("       if [hex address] is not specified, last selected is used");
    return;    
  }

  uint8_t data = strtol(args[1], NULL, 16);

  if (arg_cnt <= 2) {  
    s->printf("Write 0x%02X @ 0x%02X\n\r",data, reg_last_addr);
    writeData(data);
    return;
  }

  uint8_t addr = strtol(args[2], NULL, 16);
  
  s->printf("Write 0x%02X @ 0x%02X\n\r", data, addr);
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
  
  s->printf("mux 0x%02X @ 0x%02X\n\r",data, reg_last_addr);

  writeMux(data);

}

// read from bus - if no arg suppled, current address is used
void cmd_busread(int arg_cnt, char **args) {
  if (Surface->halt == false) cmd_warn_bus_running();

  Stream *s = cmdGetStream();
  if (arg_cnt == 1){
    s->printf("read: 0x%02X @ 0x%02X\n\r", readData(), reg_last_addr);  
    return;    
  }

 if (arg_cnt == 2){
    uint8_t addr = strtol(args[1], NULL, 16);
    selectAddr(addr);
    s->printf("read: 0x%02X @ 0x%02X\n\r", readData(), reg_last_addr);  
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
    s->printf("set: @ 0x%02X\n\r", reg_last_addr);  
    return;    
  }

 if (arg_cnt > 2){
    s->println("set: Invalid Arguments.");
    return;    
  }

 uint8_t addr = strtol(args[1], NULL, 16);
 reg_last_addr = !addr;
 selectAddr(addr);
 
 s->printf("set: selected @ 0x%02X\n\r", reg_last_addr);  
}
#endif


// print some stats
void cmd_stat(int arg_cnt, char **args){
  Stream *s = cmdGetStream();
  s->println("Stats: ");
#if defined (TESTING)
  s->print("  Card Bus Updates/Second: ");
  s->println(UpdatesPerSecond);
#endif
}

// restart teensy
void cmd_restart(int arg_cnt, char **args){
  Stream *s = cmdGetStream();
  s->println("Soft Reboot...");  
  _restart_Teensyduino_();  
}

// reboot teensy
void cmd_reboot(int arg_cnt, char **args){
  Stream *s = cmdGetStream();
  s->println("Soft Reboot...");  
  _reboot_Teensyduino_();  
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


#if defined (USE_ETHERNET)
void cmd_ifconfig(int arg_cnt, char **args){

  Stream *s = cmdGetStream();
  if (arg_cnt == 1){
    s->printf("eth0:  ip: %d.%d.%d.%d  target: %d.%d.%d.%d:%d  %s\n\r",  ip[0],ip[1],ip[2],ip[3], trg[0],trg[1],trg[2],trg[3], localPort, eth0_up == true ? "UP":"DOWN");
    s->printf("       MAC: %02X:%02X:%02X:%02X:%02X:%02X\t tx: %d bytes\trx: %d bytes\n\r", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5], eth0_stats_tx, eth0_stats_rx);
    return;    
  }

  if (arg_cnt >= 2){
    // arg 2 is ip address, or "up" or "down" or "target" in which case arg 3 is ip address of target and optional arg 4 is udp port

    String arg1(args[1]);

    if (arg1 == "up") {
      eth0_up = true;
      s->println("eth0: up.");
    } else if (arg1 == "down"){
      eth0_up = false;
      s->println("eth0: down.");
    } else if (arg1 == "target"){      
      // args 2 is target ip address
      trg.fromString(String(args[2]));
      if (arg_cnt > 3){ 
       localPort = atoi(args[3]); // arg 3 is port (optional)
      }
      s->printf("eth0:  target: %d.%d.%d.%d:%d\n\r", trg[0],trg[1],trg[2],trg[3], localPort);
    } else {
      // args 1 is interface ip address
      ip.fromString(arg1);
      s->printf("eth0:  ip: %d.%d.%d.%d\n\r",  ip[0],ip[1],ip[2],ip[3]);
      Ethernet.begin(mac,ip);      
    }  
  }
}
#endif

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
    pinMode(c, OUTPUT);// setup pins as outputs

#ifdef USE_ETHERNET
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);    
  delay(100);
#endif

#if defined(SERIAL_CLI_ENABLED)
  cmdInit(&Serial); // use default Serial. device
  cmdAdd("help", cmd_help);
  cmdAdd("run", cmd_run);
  cmdAdd("stop", cmd_stop); 
  cmdAdd("scan", cmd_scan_bus);   
  cmdAdd("stat", cmd_stat);    
  cmdAdd("version", [](int argc, char **argv){ Stream *s=cmdGetStream(); s->print("Version "); s->println(APP_VERSION); });
  cmdAdd("uptime", [](int argc, char **argv){ Stream *s=cmdGetStream(); s->print("Uptime: "); s->print(millis());s->println("ms"); });
  cmdAdd("restart",cmd_restart);
  cmdAdd("bootload",cmd_reboot);

#if defined (USE_ETHERNET)
  cmdAdd("ifconfig",cmd_ifconfig);
#endif  
#if defined (SERIAL_CLI_BUSCONTROL)
  cmdAdd("dirb",cmd_dirb);
  cmdAdd("set",cmd_set);
  cmdAdd("write",cmd_buswrite);
  cmdAdd("read",cmd_busread);
  cmdAdd("mux",cmd_busmux);
#endif
#endif


  Surface = new JandsCardBus(); // create out new surface class
    
#if defined (CONFIG_EVENT_408)
  Surface->preset1.setCardAddress(ADDR_PRESET_1);  // discriminate two preset cards by address
  Surface->preset2.setCardAddress(ADDR_PRESET_2);
#endif


#if defined (CONFIG_ECHELON_1K)
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

#endif


#if defined (CONFIG_EVENT_408)
  Key.edit(&Surface->assign.lcd, (char *)"Channel", &input_value, 1024, 0, 0);
#endif


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
    uint16_t keyp = Surface->keys.getKey();
    Serial.printf("Key: %i\n\r", (int)keyp);

#if defined (SURFACE_CLI_ENABLED)
    SurfaceCmdLine(keyp);
#endif     

  }

  //if (Key.check()){ // enter or exit pressed
  //    Serial.printf("Val: %i\n",Key.currentValue());
  //    //Key.edit(&Surface->assign.lcd, (char*)"Level: ", &input_value, 1024, 0, line );
  //    Key.input(&Surface->assign.lcd, (char*)"Level", 0, 0 );
  //}

// update the serial command line interface
#if defined(SERIAL_CLI_ENABLED)
  cmdPoll();
#endif

#if defined(TESTING)
  fps(); // measure updates per second

#if defined(FADER_TESTING)
  #if defined(CONFIG_EVENT_408)
    val = Surface->preset1.faders[0];
    val2 = Surface->preset1.faders[2];
  #endif
#endif

#endif
}
