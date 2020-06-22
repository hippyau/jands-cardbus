#include <Arduino.h>
/*  
       _                 _     
      | |               | |    
      | | __ _ _ __   __| |___ 
  _   | |/ _` | '_ \ / _` / __|
 | |__| | (_| | | | | (_| \__ \
  \____/ \__,_|_| |_|\__,_|___/
                               
 - Jands CardBus - Example App
*/

// configuration
//#define USE_ETHERNET
#define TESTING


#include <JandsCardBus.h>


#if defined(TEST_MENUS)
//  Menu system
#include "LiquidMenu.h"
#endif

#if defined (TESTING)
  #include "debug.h"
#endif

#if defined (USE_ETHERNET)
  #include <SPI.h>        
  #include <Ethernet.h>
  #include <EthernetUdp.h>
  #define UDP_TX_PACKET_MAX_SIZE 256 //increase UDP size to 256 bytes
  static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // Our MAC address
  static uint16_t localPort = 8888;       // Port to listen on
  static IPAddress ip(192, 168, 1, 88);   // Our IP address 
  static IPAddress trg(192,168, 1, 255);   // Where we are sending to
  static EthernetUDP Udp;  
#endif



// pointer to our entire control surface
static JandsCardBus * Surface = NULL;

#if defined(TEST_MENUS)
// pointer to our menu system
static LiquidMenu * Menu; 
#endif







// send surface state packet to host(s)
void inline sendSurfaceState()
{  

#ifdef USE_ETHERNET
  Udp.beginPacket(trg, 8888);
  Udp.write("JCB\0"); // header

  // faders x 64 bytes
  for (uint8_t cnt = 0 ; cnt < 24 ; cnt++)
    Udp.write(Surface->preset1.faders[cnt]);
  for (uint8_t cnt = 0 ; cnt < 24 ; cnt++)
    Udp.write(Surface->preset2.faders[cnt]);
  for (uint8_t cnt = 0 ; cnt < 8 ; cnt++)
    Udp.write(Surface->assign.faders[cnt]);
  for (uint8_t cnt = 0 ; cnt < 8 ; cnt++)
    Udp.write(Surface->master.faders[cnt]); // fader 8 is self test
  
  // buttons x 13 bytes - includes some Card ID
  for (uint8_t cnt = 0 ; cnt < 2 ; cnt++)
    Udp.write(Surface->preset1.buttons[cnt]);
  for (uint8_t cnt = 0 ; cnt < 2 ; cnt++)
    Udp.write(Surface->preset2.buttons[cnt]);
  for (uint8_t cnt = 0 ; cnt < 1 ; cnt++)
    Udp.write(Surface->assign.buttons[cnt]);
  for (uint8_t cnt = 0 ; cnt < 3 ; cnt++)
    Udp.write(Surface->palette.buttons[cnt]);  
  for (uint8_t cnt = 0 ; cnt < 5 ; cnt++)
    Udp.write(Surface->master.buttons[cnt]);

  // wheels x 3 - up/down counters 0x00 -- 0x0F
  for (uint8_t cnt = 0 ; cnt < 3 ; cnt++)
    Udp.write(Surface->master.wheels[cnt]);

  Udp.endPacket(); // send frame
#endif

}



// take input from host(s)
void hostSetSurfaceState()
{  

#ifdef USE_ETHERNET
static char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet,
static int packetSize;
static IPAddress remote;

packetSize =  Udp.parsePacket();
  if(packetSize)
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
    Udp.read(packetBuffer,UDP_TX_PACKET_MAX_SIZE);

    if (strcmp(packetBuffer,"JCB") == 0) {
      // legit packet

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
  Ethernet.begin(mac,ip);
  Udp.begin(localPort);
#endif      


  Surface = new JandsCardBus(); // create out new surface class
  Surface->preset1.setCardAddress(ADDR_PRESET_1);  // discriminate two preset cards by address
  Surface->preset2.setCardAddress(ADDR_PRESET_2);


#if defined(TEST_MENUS)
  // master menu assigned to master card LCD
  Menu = new LiquidMenu(Surface->master.lcd);
  Menu->add_screen(welcome_screen);
  Menu->add_screen(secondary_screen);
#endif  

  Key.edit(&Surface->assign.lcd, (char*)"Channel", &input_value, 1024, 0,0 );

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

  if (Surface->update()){  // has something on the surface changed?      
      sendSurfaceState(); 
  }
  hostSetSurfaceState(); // process any incomming commands



  while (Surface->keys.isKeyAvailable()){
    uint8_t keyp = Surface->keys.getKey();
    Serial.printf("Key: %i\n",(int)keyp);
    Key.key(keyp);
  }

  if (Key.check()){
      static bool line = 0;
      line = !line;
      Serial.printf("Val: %i\n",Key.currentValue());
      //Key.edit(&Surface->assign.lcd, (char*)"Level: ", &input_value, 1024, 0, line );
      Key.input(&Surface->assign.lcd, (char*)"Level", 0, line );
  }


#if defined(TEST_MENUS)
  if (buttonRight.check()){
      Menu->next_screen();
      Menu->update();
  } 

  if (buttonLeft.check()){
        Menu->previous_screen();
        Menu->update();
  } 

  if (buttonPlus.check()){
        Menu->switch_focus(1);
        //Menu->call_function(1);
        Menu->update();
  } 

  if (buttonMinus.check()){
        Menu->switch_focus(0);
        //Menu->call_function(2);
        Menu->update();
  } 
#endif


val = Surface->preset1.faders[0];
val2 = Surface->preset1.faders[2];




#ifdef TESTING
  Surface->preset1.leds[0] = Surface->assign.faders[0];
  Surface->preset1.leds[1] = Surface->assign.faders[1];

  fps(); // measure updates per second
#endif
}
