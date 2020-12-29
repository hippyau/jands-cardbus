#ifndef _CONFIG_H_
#define _CONFIG_H_

#define APP_VERSION ("0.63 (Echelon 1K)")

// debug/testing
#define DEBUG_TESTING // confirm surface without a host

#if defined(DEBUG_TESTING)
#define TESTING       // General Debug
#define FADER_TESTING // Display fader levels on LCDs
#endif

#define USE_ETHERNET // Use an ethernet interface

#if defined(USE_ETHERNET)

#define UDP_TX_PACKET_MAX_SIZE 256 //increase UDP size to 256 bytes

#define MA_MSC_UDP // Send GMA MSC over Ethernet (UDP)

#endif

#define SERIAL_CLI_ENABLED // enable the serial/uart/usb command line interface

#if defined(SERIAL_CLI_ENABLED)
#define SERIAL_CLI_BUSCONTROL // enable serial commands to manipulate the card bus
#endif

// Only define one of these surfaces....
#define CONFIG_ECHELON_1K // use the Jands Echelon 1k surface, as manufactured
//#define CONFIG_EVENT_408   // use the Jands Event 408 surface, as manufactured

#if defined(CONFIG_EVENT_408)          // Event 4xx card testing code is embedded in each card class
#define ASSIGN_CARD_LCD_TESTING (true) // fader values on LCD line 1
#define PRESET_LEDS_TESTING (true)     // LED's mimic faders
#define MASTER_CARD_TESTING (true)     // display info on the master card LCD

#define SURFACE_CLI_ENABLED // use the keys on the surface and form a command line interface. Currently only Event4xx.
#endif

#if defined(CONFIG_ECHELON_1K) // Echelon 1K
#define PROGRAM_1K_CARD_TESTING (true)
#define PLAYBACK_1K_CARD_TESTING (true)
#define MENU_1K_CARD_TESTING (true)

#define USB_KEYBOARD // act as a USB keyboard on teensy

#endif

#endif // _CONFIG_H