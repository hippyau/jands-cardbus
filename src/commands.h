#ifndef _COMMANDS_CPP_
#define _COMMANDS_CPP_

#include "config.h"
#include "globals.h"
#include "Cmd.h"
#include "JandsCardBus.h"

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
  cmdGetStream()->println("lcd      - select [LCD number] and print [string]");   
  cmdGetStream()->println("menulbl  - set a menu card item label [0..63] to [string(5)]");   
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


// set a message on a LCD
void cmd_lcd(int arg_cnt, char **args) {
  if (Surface->halt == false) cmd_warn_bus_running(); 
  Stream *s = cmdGetStream();

  if (arg_cnt == 1){
    #if defined(CONFIG_ECHELON_1K)
      s->println("0: Menu 1 Left");
      s->println("1: Menu 1 Right");
      s->println("2: Menu 2 Left");
      s->println("3: Menu 2 Right");
      s->println("4: Playback 1");
      s->println("5: Playback 2");
    #endif

    return;    
  }
  if (arg_cnt > 3){
    s->println("lcd: TODO: more than one word.");
    return;    
  }

  uint8_t nLCD = strtol(args[1], NULL, 10);
  lcdModule * lcd;

  switch (nLCD)
  {
#if defined(CONFIG_ECHELON_1K)    
  case 0:
     lcd = &Surface->menu1.lcd[0];  
    break;
  case 1:
     lcd = &Surface->menu1.lcd[1];  
    break;  
  case 2:
     lcd = &Surface->menu2.lcd[0];  
    break;  
  case 3:
     lcd = &Surface->menu2.lcd[1];  
    break;  
  case 4:
     lcd = &Surface->playback1.lcd[0];  
    break;  
  case 5:
     lcd = &Surface->playback2.lcd[0];  
    break;  
  default:
     s->println("lcd: Invalid LCD number");
     return;    
    break;
#endif    
  }

  lcd->setCursor(0,0);
  lcd->printf("%s", args[2]);

}


// set a message on a LCD
void cmd_lcd_menu_label(int arg_cnt, char **args) {
  if (Surface->halt == false) cmd_warn_bus_running(); 
  Stream *s = cmdGetStream();

  if (arg_cnt == 1){
    #if defined(CONFIG_ECHELON_1K)
      s->println("Usage: [menu item] [name]");
    #endif
    return;    
  }
  if (arg_cnt > 3){
    s->println("menu_label: error, more than one word.");
    return;    
  }

  uint8_t nLCD = strtol(args[1], NULL, 10);
  uint8_t X = 0, Y = 0;
  lcdModule * lcd;

  if (nLCD >= 0 & nLCD <= 7) {
    lcd = &Surface->menu1.lcd[0];
    X = (nLCD * 5);
    Y = 0;
  } else if (nLCD >= 8 & nLCD <= 15) {
    lcd = &Surface->menu1.lcd[0];
    X = ((nLCD-8) * 5);
    Y = 1;
  } else if (nLCD >= 16 & nLCD <= 23) {
    lcd = &Surface->menu1.lcd[1];
    X = ((nLCD-16) * 5);
    Y = 0;
  } else if (nLCD >= 24 & nLCD <= 31) {
    lcd = &Surface->menu1.lcd[1];
    X = ((nLCD-24) * 5);
    Y = 1;
  } else if (nLCD >= 32 & nLCD <= 39) {
    lcd = &Surface->menu2.lcd[0];
    X = ((nLCD-32) * 5);
    Y = 0;
  } else if (nLCD >= 40 & nLCD <= 47) {
    lcd = &Surface->menu2.lcd[0];
    X = ((nLCD-40) * 5);
    Y = 1;
  } else if (nLCD >= 48 & nLCD <= 55) {
    lcd = &Surface->menu2.lcd[1];
    X = ((nLCD-48) * 5);
    Y = 0;
  }  else if (nLCD >= 56 & nLCD <= 63) {
    lcd = &Surface->menu2.lcd[1];
    X = ((nLCD-56) * 5);
    Y = 1;    
  } else {
    s->println("menu_label: invalid label number.");
    return;
  }
  
  lcd->setCursor(X,Y);
  lcd->print("     "); // clear old one
  lcd->setCursor(X,Y);
  lcd->printf("%.5s", args[2]); // print 5 chars max

}






#endif // SERIAL_CLI_BUSCONTROL


// print some stats
void cmd_stat(int arg_cnt, char **args){
  Stream *s = cmdGetStream();
  s->println("Stats: ");
#if defined (TESTING)
  s->print("  Card Bus Updates Now (hz): ");
  s->println(UpdatesPerSecond);
  s->print("  Card Bus Updates Min (hz): ");
  s->println(UpdatesPerSecondMin);
  s->print("  Card Bus Updates Max (hz): ");
  s->println(UpdatesPerSecondMax);

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
  s->println("Enter Bootloader...");  
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

// execute when going up or IP
void eth0_going_up(){
      eth0_up = true;
      Ethernet.begin(mac,ip);   

#if defined(MA_MSC_UDP) 
        char ipstr[16];
        sprintf((char*)&ipstr,"%d.%d.%d.%d",trg[0],trg[1],trg[2],trg[3]);
        maMSC.init(ipstr);   
#endif      
}

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
      
      eth0_going_up(); // go up
      if (eth0_up == true){      
        s->println("eth0: up.");
      } else {
        s->println("eth0: down.");
      }

    } else if (arg1 == "down") {
      eth0_up = false;
      s->println("eth0: down.");


    } else if (arg1 == "target") {      
      // args 2 is target ip address
      trg.fromString(String(args[2]));
      if (arg_cnt > 3){ 
       localPort = atoi(args[3]); // arg 3 is port (optional)
      }
      s->printf("eth0:  target: %d.%d.%d.%d:%d\n\r", trg[0],trg[1],trg[2],trg[3], localPort);
      eth0_going_up(); // change

    } else {
      // args 1 is interface ip address
      ip.fromString(arg1);
      s->printf("eth0:  ip: %d.%d.%d.%d\n\r",  ip[0],ip[1],ip[2],ip[3]);
      if (eth0_up)
        eth0_going_up(); // change        
    }  
  }
}
#endif

#endif // SERIAL_CLI_ENABLED


#endif // _COMMANDS_CPP_