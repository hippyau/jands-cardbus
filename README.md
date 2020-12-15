# jands-cardbus
CPP/Arduino code to master the Jands Card Bus interface, and code to access several interface cards.  

*YouTube Video*---> 
[![Video](https://img.youtube.com/vi/OtjAWHLR7C0/0.jpg)](https://www.youtube.com/watch?v=OtjAWHLR7C0)


## Overview 

The Card Bus is common to several types of old (discontinued) Jands lighting consoles, including Jands EVENT4, Jands Hog, Stage, and ESP series consoles.

This code has so far been proven on the EVENT4 series, with more coming on the Jands Hog Echelon series cards.

This is a hack job, but it works...

![Jands EVENT4 Palette Card](https://github.com/hippyau/jands-cardbus/raw/master/docs/img/408-pallete-card.png)

The Jands EVENT4 Palette Card.


Here is a video showing the maunual toggling of bits on the Palette card during development...  [https://vimeo.com/419880334]


## Important to note 

Not all Jands Card Busses are created equal.  DO NOT directly connect an ESP-II card to a the Event 4 / Echelon series bus (the one I present here), it is a different pinout, but seems to be very similar operation and a simple adapter could be made. [tbc]

That there are different pinouts for some different Card Busses, depending on the series of console.  See Issue below.
https://github.com/hippyau/jands-cardbus/issues/1
Using the wrong pinout for the card you are working with will potentially damage the card!


## Firmware Command Line Interface

There is a CLI available, which helps in reading and working out the card bus...

```
JCB >> reboot
Soft Reboot...
Found ECHMENU2 Card @ 0x00
Not Found ECHMENU2 Card @ 0x10
Not Found - Playback Card @ 0x80
Not Found - Playback Card @ 0x90
Not Found - Program Card @ 0xF0
help
Available commands: (command with arguments provide usage help when no arguments given)

version - print firmware version
run     - enable cardbus update loop
stop    - disable cardbus update loop
stat    - print cardbus statistics
reboot  - reboot

Debug commands:

set     - select hex bus [address 0x00..0xFF] (aka ALEL) where high nibble is card addres, low nibble is device on the card
mux     - set a mux [hex address] (aka ALEH) for the currently selected card & device
write   - select hex bus [address] and write hex [data]
read    - read bus at current address, or option [hex address] also selects device 

------ Jands Card Bus Master CLI
JCB >> 
```

## Coverage

So far, the Event4xx series Assign, Pallete, Master and Preset cards are supported, with all LCDs, buttons, faders (with filtering) and encoders.
Also the Echelon 1000 Menu, Program, and Playback cards are supported, though be aware some addresses overlap the Event4 series.

Then Echelon 1K series also includes some USB Keyboard emulation support for the Program card. 

![Jands EVENT408](https://github.com/hippyau/jands-cardbus/raw/master/docs/img/event408.jpg)

Currently, on the EVENT Assign card, the levels of the faders are shown, along with a pseudo command-line display (which is pretty useless ATM) based on pressed Master card buttons and a state.

Entire surface state optionally gets transmitted over a WizNet Ethernet chip (```#define USE_ETHERNET```) on UDP port 8888 to (```static IPAddress trg(192,168, 1, 49);   // Where we are sending to```) in the packet format described in (```void inline sendSurfaceState()```)

There is a ```GUI``` app, based on Dear ImGui, which can recieve these packets.
![Jands EVENT4 GUI](https://github.com/hippyau/jands-cardbus/raw/master/docs/img/GUI-408.png)

The Arduino code provided is targeted at a Teensy2.0++ at the moment, but not hard to change.

Note the pinout uses pins 0,1 which are used on most Arduinos as RX and TX for code upload.
Again, not hard to change, but used here because the pins where on a consecutive port of the AT micro, which improved bus speed significantly over using digitalWrite() calls.

You can disable this behaviour here...

```
// comment out for using arduino pin mapping where, on boards where the data bus is not consective pins (like Leonardo)
#define NO_PIN_MAPPING // use direct port commands
```
https://github.com/hippyau/jands-cardbus/blob/38304a2e782aeb63bad4ccb7294a74088c640326/lib/cardbus/hardware.h#L7

You can change the PINS here... (but the D0..D7 must be consecutive)

```
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

```
https://github.com/hippyau/jands-cardbus/blob/38304a2e782aeb63bad4ccb7294a74088c640326/lib/cardbus/hardware.h#L15










