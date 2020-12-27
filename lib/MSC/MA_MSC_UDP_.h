#ifndef _INCLUDE_MA2_MSC_H_
#define _INCLUDE_MA2_MSC_H_

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


// TODO: Make adjustable
constexpr uint16_t MSC_RX_PORT = 6004;
constexpr uint16_t MSC_TX_PORT = 6005;


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

    uint16_t Send_Fader_Value(uint16_t page, uint16_t fader, uint8_t level);
    
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
uint16_t maMSC_t::Send_Fader_Value(uint16_t page, uint16_t fader, uint8_t level) {
  static uint8_t buffer[11];

  uint16_t value = (level * 0x4000) / 256; // scale 0..255 to 0..16384
  uint8_t msb = (value >> 7) & 0x7F;
  uint8_t lsb = value & 0x7F;
 
  // for a MSC message
  buffer[0] = 0xf0; 
  buffer[1] = 0x7f;
  buffer[2] = 0x7F; // 00 - 6F Individual Devices, 70 - 7E Device Groups (15), 7F All devices
  buffer[3] = 0x02; // 02  - MSC Subtype
  buffer[4] = 0x7F; // 01 General , 02 MovingLight , 7F All 
  buffer[5] = MSC_SET; // MSC SET command
  buffer[6] = fader-1;
  buffer[7] = page & 0xFF;
  buffer[8] = lsb;
  buffer[9] = msb;
  buffer[10] = 0xF7;
  return maMSC_t::send(&buffer[0], 11);  
}


void maMSC_t::init (const char *ipAddress) {     
  target.fromString(ipAddress);

#if defined(TESTING) 
  Serial.printf("MSC Target: %d.%d.%d.%d \r\n", target[0], target[1], target[2], target[3] );
#endif
     dev_Udp.begin(MSC_RX_PORT);  
     return true;
}


uint16_t maMSC_t::send(const uint8_t *sysexBuffer, uint32_t len) {
  static uint8_t buffer[12];
  //memset(&buffer[0], 0, sizeof(buffer));
  sprintf((char *)&buffer, "GMA\0");
  sprintf((char *)&buffer[4], "MSC\0");
  memcpy((void *)&buffer[8],&len, sizeof(uint32_t));
  
  if (dev_Udp.beginPacket(target, MSC_TX_PORT)){
  dev_Udp.write(&buffer[0], 12);
  dev_Udp.write(sysexBuffer,len);
  dev_Udp.endPacket(); // send frame
  return 12+len;
  } else {
#if defined(TESTING) 
  Serial.printf("MSC: beginPacket failed...\r\n");
#endif
  }
}


#endif // USE_ETHERNET


#endif // _INCLUDE_MA2_MSC_H_