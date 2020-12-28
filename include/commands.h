#ifndef _COMMANDS_H_
#define _COMMANDS_H_


#include "Arduino.h"
#include "config.h"


#if defined(SERIAL_CLI_ENABLED)

void cmd_help(int arg_cnt, char **args);

// Scan the card bus for cards, high nibble is card address, low nibble is 0x0F (card ID)
void cmd_scan_bus(int arg_cnt, char **args);


#if defined (SERIAL_CLI_BUSCONTROL)
// ----  bus control CLI

// warning that other code is playing with the bus addresses...
void cmd_warn_bus_running();

// set bus direction
void cmd_dirb(int arg_cnt, char **args);

// write to bus
void cmd_buswrite(int arg_cnt, char **args);


// write to bus mux
void cmd_busmux(int arg_cnt, char **args);

// read from bus - if no arg suppled, current address is used
void cmd_busread(int arg_cnt, char **args);

// set bus address
void cmd_set(int arg_cnt, char **args);

#endif


// print some stats
void cmd_stat(int arg_cnt, char **args);


// restart teensy
void cmd_restart(int arg_cnt, char **args);

// reboot teensy
void cmd_reboot(int arg_cnt, char **args);

// stop updates
void cmd_stop(int arg_cnt, char **args);

// resume updates
void cmd_run(int arg_cnt, char **args);


#if defined (USE_ETHERNET)

// execute when going up or IP
void eth0_going_up();

void cmd_ifconfig(int arg_cnt, char **args);

#endif

#endif // SERIAL_CLI_ENABLED


#endif // _COMMANDS_H_