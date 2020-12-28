#ifndef _INCLUDE_MA_MSC_H_
#define _INCLUDE_MA_MSC_H_

#include "config.h"

#include "Arduino.h"
#include "stddef.h"

#if defined(USE_ETHERNET)
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

/* 

;---- MIDI Show Control for MA over UDP

;- MIDI Show Control
;                                                Number
;                                                of Data  Recomm'd
;  Hex command                                   bytes    Min Sets
;  ---------------------------------------------------------------
;  00  reserved For extensions
;  01  GO                                        variable 123
;  02  STOP                                      variable 123
;  03  RESUME                                    variable 123
;  04  TIMED_GO                                  variable -23
;  05  LOAD                                      variable -23
;  06  SET                                       4 Or 9   -23
;  07  FIRE                                      1        -23
;  08  ALL_OFF                                   0        -23
;  09  Restore                                   0        -23
;  0A  RESET                                     0        -23
;  0B  GO_OFF                                    variable -23 
 */


// 0000   ff ff ff ff ff ff 00 23 24 82 f1 77 08 00 45 00   ÿÿÿÿÿÿ.#$.ñw..E.
// 0010   00 38 f4 a5 00 00 80 11 c0 c3 c0 a8 01 fc c0 a8   .8ô¥....ÀÃÀ¨.üÀ¨
// 0020   01 ff 17 75 17 75 00 24 2e f5 47 4d 41 00 4d 53   .ÿ.u.u.$.õGMA.MS
// 0030   43 00 1c 00 00 00 f0 7f 7f 02 7f 01 31 2e 30 30   C.....ð.....1.00
// 0040   30 00 35 00 32 f7                go  1 .   0 0    0.5.2÷
//         0   exec  page                      cue number


// TODO: Make adjustable
constexpr uint16_t MSC_RX_PORT = 6004;
constexpr uint16_t MSC_TX_PORT = 6004;


struct MSC_Header_t {
  uint8_t id0;    // F0
  uint8_t id1;    // 7F
  uint8_t DeviceID;//.b      ; 00 - 6F Individual Devices, 70 - 7E Device Groups (15), 7F All devices
  uint8_t id3;//.b           ; 02  - MSC Subtype
  uint8_t CommandFormat;//.b ; 01 General , 02 MovingLight , 7F All 
  uint8_t Command;//       ;
};

//static MSC_Header_t MSC_Header;

// MSC Commands
constexpr uint8_t MSC_SET = 0x06;
constexpr uint8_t MSC_GO = 0x01;
constexpr uint8_t MSC_RESET = 0x0A;
constexpr uint8_t MSC_STOP = 0x02;
constexpr uint8_t MSC_RESUME = 0x03;
constexpr uint8_t MSC_TIMED_GO = 0x04;
constexpr uint8_t MSC_LOAD = 0x05;
constexpr uint8_t MSC_GO_OFF = 0x0B;


struct MSC_Set_Header_t {
  uint16_t Control;
  uint16_t Value; 
};

//static MSC_Set_Header_t MSC_Command_Set;


class maMSC_t {

   public: 
    
    void init (const char * ipAddress);     

    uint16_t Send_Fader_Value(uint16_t page, uint16_t fader, uint8_t level, uint8_t command = MSC_SET);
    
    private:

    // send MSC command, as a MIDI sysex buffer input of len, via UDP to an MA2
    uint16_t send(const uint8_t *sysexBuffer, uint32_t len);
    EthernetUDP dev_Udp;
    IPAddress target; 
};


 

//;--- Send MIDI show control Set commands (executor fader commands)
//; page 1 - 256
//; fader 1 - 999
//; value 0 - 255
uint16_t maMSC_t::Send_Fader_Value(uint16_t page, uint16_t fader, uint8_t level, uint8_t command = MSC_SET) {
  static uint8_t buffer[11];

  uint16_t value = (level * 0x4000) / 256; // scale 0..255 to 0..16384
  uint8_t msb = (value >> 7) & 0x7F;
  uint8_t lsb = value & 0x7F;
 
  // header for an MSC message
  buffer[0] = 0xf0; 
  buffer[1] = 0x7f;

  // TODO: make configurable
  buffer[2] = 0x7F; // 00 - 6F Individual Devices, 70 - 7E Device Groups (15), 7F All devices
  buffer[3] = 0x02; // 02  - MSC Subtype
  buffer[4] = 0x7F; // 01 General , 02 MovingLight , 7F All 

  // the MSC command and data
  buffer[5] = command; // MSC SET command
  buffer[6] = fader-1;
  buffer[7] = page & 0xFF;
  buffer[8] = lsb;
  buffer[9] = msb;

  // footer for an MSC message
  buffer[10] = 0xF7;

  // send to target
  return maMSC_t::send(&buffer[0], 11);  
}





void maMSC_t::init (const char *ipAddress) {     
  target.fromString(ipAddress);
#if defined(TESTING) 
  Serial.printf("GMA MSC Target: %d.%d.%d.%d \r\n", target[0], target[1], target[2], target[3] );
#endif
  dev_Udp.begin(MSC_RX_PORT);  
  return;
}



uint16_t maMSC_t::send(const uint8_t *sysexBuffer, uint32_t len) {
  if (len > 12) {
#if defined(TESTING) 
    Serial.printf("GMA MSC: sysex too long: %d...\r\n", len);
#endif  
    return 0;
  }
  static uint8_t buffer[12] = {0};
  sprintf((char *)&buffer, "GMA");
  sprintf((char *)&buffer[4], "MSC");
  memcpy((void *)&buffer[8],&len, sizeof(uint32_t));
  
  if (dev_Udp.beginPacket(target, MSC_TX_PORT)){
  dev_Udp.write(&buffer[0], 12);
  dev_Udp.write(sysexBuffer,len);
  dev_Udp.endPacket(); // send frame
  return 12+len;
  } else {
#if defined(TESTING) 
  Serial.printf("GMA MSC: beginPacket failed...\r\n");
#endif  
  }
  return 0;
}


#endif // USE_ETHERNET



#endif // _INCLUDE_MA_MSC_H_