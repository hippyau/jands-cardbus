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
//#define USE_ETHERNET


#define TESTING
#define FADER_TESTING 
#define ASSIGN_CARD_LCD_TESTING // fader values on LCD line 1
#define PRESET_LEDS_TESTING // LED's mimic faders
#define MASTER_CARD_TESTING // display info on the master card LCD
//#define TEST_MENUS // testing menu system


// CardBus driver
#include <JandsCardBus.h>



#if defined (USE_ETHERNET)
  #include <SPI.h>        
  #include <Ethernet.h>
  #include <EthernetUdp.h>
  #define UDP_TX_PACKET_MAX_SIZE 256 //increase UDP size to 256 bytes
  static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // Our MAC address
  static uint16_t localPort = 8888;       // Port to listen on
  static IPAddress ip(192, 168, 1, 88);   // Our IP address 
  static IPAddress trg(192,168, 1, 49);   // Where we are sending to
  static EthernetUDP Udp;  
#endif


#if defined(TEST_MENUS)
//  Menu system
#include "LiquidMenu.h"
#endif


#if defined (TESTING)
  #include "debug.h"
#endif



#if defined(TEST_MENUS)
// pointer to our menu system
static LiquidMenu * Menu; 
#endif




// send surface state packet to host(s)
void inline sendSurfaceState()
{  

#ifdef USE_ETHERNET

  // construct UDP frame with all the surface values
  Udp.beginPacket(trg, 8888);
  Udp.write("JCB0"); // header

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

#if defined(USE_ETHERNET)
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

    if (strcmp(packetBuffer,"JCBL") == 0) {
      // legit LED packet

      //Serial.println("Contents:");
      //Serial.println(packetBuffer);

    }
    else if (strcmp(packetBuffer,"JCBD") == 0) {  
      // legit LCD packet - payload is char card,x,y,length,string+NULL     

      uint8_t card_address = packetBuffer[4];
      //uint8_t x = packetBuffer[5];
      //uint8_t y = packetBuffer[6];
      uint8_t len = packetBuffer[7];
      packetBuffer[8+len] = 0; // terminate the string
      char* message = &packetBuffer[8];
            
      if (len > 0){
        switch(card_address) {
         case ADDR_ASSIGN:
        
            Surface->assign.lcd.setCursor(packetBuffer[5],packetBuffer[6]);
            Surface->assign.lcd.print(message);
            break;

         case ADDR_MASTER:
            Surface->master.lcd.setCursor(packetBuffer[5],packetBuffer[6]);
            Surface->master.lcd.print(message);
            break;

          default:
            break;
        }       
      }

      else { // empty message means clear display    
             switch(card_address) {
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




  Surface = new JandsCardBus(); // create out new surface class
  Surface->preset1.setCardAddress(ADDR_PRESET_1);  // discriminate two preset cards by address
  Surface->preset2.setCardAddress(ADDR_PRESET_2);


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

  Surface->keys._usb_key[152] = KEY_SPACE;   // NEXT

  Surface->keys._usb_key[98] = KEY_ESC;   // clear/restore


  Surface->keys._usb_key[162] = 'i';
  Surface->keys._usb_key[162] = 'd';
  Surface->keys._usb_key[163] = 'k';
  Surface->keys._usb_key[166] = 'f';
  Surface->keys._usb_key[167] = 'a';

  


#if defined(TEST_MENUS)
  // master menu assigned to master card LCD
  Menu = new LiquidMenu(Surface->master.lcd);
  Menu->add_screen(welcome_screen);
  Menu->add_screen(secondary_screen);
#endif  

  Key.edit(&Surface->assign.lcd, (char*)"Channel", &input_value, 1024, 0, 0 );

}






static String command_line = "";
static String pwd = "Channel"; // current directory / object
static String preset = "1."; 
static bool expect_store_destination = false; 
static int last_button = 0;
static int exec_page = 1;
static int preset_number = 1;


/* 
presets 
=======
1 = dimmer
2 = position
3 = gobo
4 = color
5 = beam
6 = focus
7 = control
8 = shapers
9 = video
*/




void process_command_line(String& cmd){

 cmd = cmd.trim();

 // if a 'solo' command, indicated we are to change our default object prompt
 if (cmd == "Group") {
   pwd = "Group";
 } else
 if (cmd == "Fixture") {
   pwd = "Fixture";
 } else
 if (cmd == "Channel") {
   pwd = "Channel";
 } else
 if (cmd == "Group") {
   pwd = "Group";
 } 
 
 

 Serial.println(cmd); // send to host

 cmd = ""; // clear the command line

}




// removes the last word in the command line
void remove_last_keyword(String& cmd){
 cmd = cmd.trim();
 int lio = cmd.lastIndexOf(" ");
 cmd = cmd.remove(lio+1);
 // was  "Fixture 2 Thru 12"
 // now  "Fixture 2 Thru "
}


// compare last commane line entry to keyword, return true if same;
bool compare_last_keyword(const String cmd, String keyword){ 
 int lio = cmd.trim().lastIndexOf(" ");
 if (lio < 0) 
  lio = 0;
  String tmp1 = cmd.substring(lio).trim();

// Serial.printf("Last='%s' Key='%s'\n", tmp1.c_str(), keyword.c_str());
 if (tmp1 == keyword){
// Serial.println("FOUND KEYWORD!");
   return true;
 }
 return false;

}

// return true if more than one space in a string
bool more_than_one_space(String& cmd){
 int a = cmd.indexOf(" ");
 if (a){
   int b = cmd.indexOf(" ",a);
   if (b) {
   return true;
  }
 }
 return false;
}




// process key input into the command line...
void CmdLine(uint8_t key){


// if store was last solo command, we are now expecting a destination button on a preset, assign or palette card
if (expect_store_destination) {
  if ((key >= 0) & (key < 32)){
    command_line += "Exec ";  
    command_line += String(exec_page);  
    command_line += ".";  
    command_line += key+1;
  } else if (key < 52){ // palette buttons 32-52
    command_line += "Preset ";  
    command_line += String(preset_number);  
    command_line += ".";  
    command_line += String(key-32);
  }
  process_command_line(command_line); 
  expect_store_destination = false;
} else 

// if shift held and a flash button pressed, select that executor....
if (SHIFT_HELD) { 
  if ((key >= 0) & (key < 32)){
    command_line = "Select Exec ";
    command_line += String(exec_page);
    command_line += ".";   
    command_line += String(key+1);
    process_command_line(command_line);
  } 

} 
 


if (key == BTN_0) {
  command_line += "0";  
} else
if (key == BTN_1) {
 if (SHIFT_HELD) {
    if (command_line.length() == 0){
     command_line += "Copy ";  
    }
  } else
  command_line += "1";  
} else

if (key == BTN_2) { 
   if (SHIFT_HELD) {
    if (command_line.length() == 0){
     command_line += "Delete ";  
    }
  } else 
  command_line += "2";  
} else
if (key == BTN_3) { // 3 / Stack
  if (SHIFT_HELD) {
     command_line += "Sequence ";     
  } else {
   command_line += "3"; 
  }   
} else
if (key == BTN_4) {
  if (SHIFT_HELD){
  command_line = "Page ";  
  } else
  {
    command_line += "4";  
  }
} else
if (key == BTN_5) {
  if (SHIFT_HELD) {
    if (command_line.length() == 0){
     command_line += "Assign ";  
    }
  } else
  command_line += "5";  
}  else
if (key == BTN_6) {
   if (SHIFT_HELD) {
    if (command_line.length() == 0){
     command_line += "Backup";  
     process_command_line(command_line);
    }
  } else  
  command_line += "6";  
} else
if (key == BTN_7) {
  command_line += "7";  
} else
if (key == BTN_8) {
   if (SHIFT_HELD) {
    if (command_line.length() == 0){
     command_line += "Edit ";  
    }
  } else
  command_line += "8";  
} else
if (key == BTN_9) {
  command_line += "9";  
} else

if (key == BTN_PLUS) {
  if (command_line.length() == 0){
    command_line = "DefGoForward";
    process_command_line(command_line);
  } else 
  command_line += "+";  
} else
if (key == BTN_MINUS) {
  if (command_line.length() == 0){
    command_line = "DefGoBack";
    process_command_line(command_line);
  } else 
  command_line += "-";  
} else


if (key == BTN_RIGHT) {
  if (command_line.length() == 0){
    command_line = "Next";
    process_command_line(command_line);
  } else {    
    command_line += ".";  // right key is decimal
  } 
} else

if (key == BTN_LEFT) {
  if (command_line.length() == 0){
    command_line = "Prev";
    process_command_line(command_line);
  } else {    
    remove_last_keyword(command_line);
  } 
} else

if (key == BTN_FADER) { // multi-stacked button  -  both Fader and Exec 
  if (last_button == key){
    remove_last_keyword(command_line);
    command_line += "Fader ";
    key = 255; // reset key, so that last key will not be the same as this key, forcing a switch to Exec on next press   
  } else 
  command_line += "Exec ";  
} else


// example of stacking more than one keyword on a button
if (key == BTN_POSITION) {
 if (command_line.length() == 0){
   command_line = "Position";
   command_line += " ";
 } else 
 {
  if (compare_last_keyword(command_line, "Position")) {    
      remove_last_keyword(command_line);
      command_line += "Focus";
      command_line += " ";
  } else 
  if (compare_last_keyword(command_line, "Focus")) {
      remove_last_keyword(command_line);
      command_line += "Prism";
      command_line += " ";
  } else
  if (compare_last_keyword(command_line, "Prism")) {
      remove_last_keyword(command_line);
      command_line += "Position";
      command_line += " ";
  } else
   command_line += "Position";
   command_line += " ";
 }
} 

else if (key == BTN_GROUP) {
  command_line += "Group ";  
} else if (key == BTN_FIXTURE) {
  command_line += "Fixture ";    
} else if (key == BTN_SCROLLER) {
  command_line += "Channel ";    
} else
if (key == BTN_HALT) {
  if (command_line.length() == 0){
    command_line = "Thru ";
  } else {    
    command_line += " Thru "; 
  } 
} 
else if (key == BTN_EXIT) {
  command_line = "";  
} 

else if (key == BTN_RELEASE) { // Release / @
  if (command_line.length() == 0){
    command_line = "At ";
  } else { 
    if (last_button == BTN_RELEASE) {      
        // second press of @ is to default at "normal" (100) full
        key = 255; // clear last button
        command_line += "100";
        process_command_line(command_line);    
      }
      else if (more_than_one_space(command_line)){
       command_line += " At ";
      } else {
        process_command_line(command_line);    
       command_line = "At "; 
      }
  }
}


else if (key == BTN_CLEAR) {
  if (command_line.length() == 0){
    command_line = "Clear";
    process_command_line(command_line); // send clear command
  } else {
    command_line = ""; // erase line
  }
} 

else if (key == BTN_RECORD) {
  if (!SHIFT_HELD){

    if (command_line.length() == 0){
    command_line += "Store ";  
    expect_store_destination = true;
    } else {
      // enter - please
      process_command_line(command_line);
    }
  }

    else { // shift_help

      if (command_line.length() != 0){
        command_line = "Update";
        process_command_line(command_line);
      }

    }

} 


last_button = key;

// update display
Surface->assign.lcd.setCursor(0,0);
Surface->assign.lcd.print("                                        ");
Surface->assign.lcd.setCursor(0,0);
Surface->assign.lcd.printf("[%s]> ",pwd.c_str());
Surface->assign.lcd.print(command_line);

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
  if (Surface->update()){  // has something on the surface changed?      
      sendSurfaceState(); 
    
  }
// process incomming host commands (if any)  
  hostSetSurfaceState(); 






// testing keypad input
  while (Surface->keys.isKeyAvailable()){
    uint8_t keyp = Surface->keys.getKey();
    Serial.printf("Key: %i\n",(int)keyp);
    CmdLine(keyp);   



    //Key.key(keyp);
  }

  //if (Key.check()){ // enter or exit pressed
  //    Serial.printf("Val: %i\n",Key.currentValue());
  //    //Key.edit(&Surface->assign.lcd, (char*)"Level: ", &input_value, 1024, 0, line );
  //    Key.input(&Surface->assign.lcd, (char*)"Level", 0, 0 );
  //}







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
    fps(); // measure updates per second 
#endif
}
