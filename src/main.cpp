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

// configuration
#include "config.h"
#include "globals.h"
#include <JandsCardBus.h>


#if defined(USE_ETHERNET)
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>    
  #if defined (MA_MSC_UDP)
  #include "MA_MSC_UDP_.h"
  #endif
#endif

#if defined(TESTING)
#include "debug.h"
#endif

#if defined(SERIAL_CLI_ENABLED)
#include "Cmd.h"
#include "commands.h" // frowned upon
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
  Udp.beginPacket(trg, localPort);
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
  Udp.beginPacket(trg, localPort);
  Udp.write("JCB1"); // header
  uint16_t tx_bytes = 0;

  // faders x 17 
  for (uint8_t cnt = 0; cnt < 8; cnt++) { 
    Udp.write(Surface->playback1.faders[cnt]);
    tx_bytes += 1;
  }
  for (uint8_t cnt = 0; cnt < 8; cnt++) {
    Udp.write(Surface->playback2.faders[cnt]);
    tx_bytes += 1;
  }
  Udp.write(Surface->playback1.faders[8]); // grand master
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

#endif // USE_ETHERNET
}



// send updates when things change.
void hostUpdate(){

  static uint8_t ofaders[FADER_PAGES][16];
  static uint8_t omaster;
  uint8_t FaderPage = Surface->FaderPage;

  for (int c = 0; c < 16 ; c++){
    uint8_t val;
    if (c > 7) {
      val = Surface->playback2.faders[c-8];
    } else {
      val = Surface->playback1.faders[c];
    }

    if (val != ofaders[FaderPage][c]){  // fader has changed

#if defined (MA_MSC_UDP)
      eth0_stats_tx += maMSC.Send_Fader_Value(FaderPage+1,c+1,val);   
#endif
      ofaders[FaderPage][c] = val;
    }
  }


  // grand master is a special case, always pinned to page 1 fader 30
  {
    uint8_t val = Surface->playback1.faders[8];
    if (val != omaster){  // fader has changed

#if defined (MA_MSC_UDP)
      eth0_stats_tx += maMSC.Send_Fader_Value(1,30,val);   
#endif

      omaster = val;
    }
  }




} // hostUpdate()




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

} // eth0_up is true
#endif
}


#if defined(FADER_TESTING)
  #if defined(CONFIG_EVENT_408)
  static uint8_t val = 0;
  static uint8_t val2 = 0;
 #endif
#endif 




#if defined (CONFIG_EVENT_408)
// modifier 'Shift' button is down?
#define buttonShift (_sbuttons[BTN_SHIFT])
static SButton buttonSetup(BTN_Setup);
static SButton buttonLeft(BTN_LEFT);
static SButton buttonRight(BTN_RIGHT);
static SButton buttonPlus(BTN_PLUS);
static SButton buttonMinus(BTN_MINUS);

static keypad_input Key;
static unsigned int input_value = -1;
#endif








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
  while (Serial.read() >= 0) ; // flush serial input buffers

  for (int c = 0; c < 14; c++)    
    pinMode(c, OUTPUT);// setup pins as outputs

#ifdef USE_ETHERNET
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);  
  #if defined (MA_MSC_UDP)
    char ipstr[16];
    #if defined (TESTING)  
      sprintf((char*)&ipstr,"%d.%d.%d.%d",trg[0],trg[1],trg[2],trg[3]);
    #endif // TESTING 
    maMSC.init(ipstr); // configure the MSC sender
  #endif // MA_MSC_UDP
#endif // USE_ETHERNET


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
  cmdAdd("if",cmd_ifconfig); // shortcut
#endif  
#if defined (SERIAL_CLI_BUSCONTROL)
  cmdAdd("dirb",cmd_dirb);
  cmdAdd("set",cmd_set);
  cmdAdd("write",cmd_buswrite);
  cmdAdd("read",cmd_busread);
  cmdAdd("mux",cmd_busmux);
#endif
#endif


  Surface = new JandsCardBus(); // create our new surface class


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





    sendSurfaceState(); // has something on the surface changed? send the surface state...
    hostUpdate(); // send changes
  }
  
  hostSetSurfaceState();// process incomming host commands (if any)


  // testing keypad input
  while (Surface->keys.isKeyAvailable())
  {
    int keyp = Surface->keys.getKey();

#if defined(TESTING)    
    Serial.printf("Key: %i\n\r", (int)keyp);
#endif    



#if defined (SURFACE_CLI_ENABLED)
    SurfaceCmdLine(keyp);
#endif     

  }


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
