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


// TODO: Make adjustable
constexpr uint16_t MSC_RX_PORT = 6004;
constexpr uint16_t MSC_TX_PORT = 6004;

constexpr uint8_t MSC_PACKET_SIZE = 36; // buffer size - not including UDP header

struct MSC_Header_t
{
  uint8_t id0;           // F0
  uint8_t id1;           // 7F
  uint8_t DeviceID;      //.b      ; 00 - 6F Individual Devices, 70 - 7E Device Groups (15), 7F All devices
  uint8_t id3;           //.b           ; 02  - MSC Subtype
  uint8_t CommandFormat; //.b ; 01 General , 02 MovingLight , 7F All
  uint8_t Command;       //       ;
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

struct MSC_Set_Header_t
{
  uint16_t Control;
  uint16_t Value;
};

//static MSC_Set_Header_t MSC_Command_Set;

class maMSC_t
{

public:
  void init(const char *ipAddress);

  uint16_t update(void); // process incoming MSC, call often

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
uint16_t maMSC_t::Send_Fader_Value(uint16_t page, uint16_t fader, uint8_t level, uint8_t command = MSC_SET)
{
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
  buffer[6] = fader - 1;
  buffer[7] = page & 0xFF;
  buffer[8] = lsb;
  buffer[9] = msb;

  // footer for an MSC message
  buffer[10] = 0xF7;

  // send to target
  return maMSC_t::send(&buffer[0], 11);
}

void maMSC_t::init(const char *ipAddress)
{
  target.fromString(ipAddress);
#if defined(TESTING)
  Serial.printf("GMA MSC Target: %d.%d.%d.%d \r\n", target[0], target[1], target[2], target[3]);
#endif
  dev_Udp.begin(MSC_RX_PORT);
  return;
}

uint16_t maMSC_t::send(const uint8_t *sysexBuffer, uint32_t len)
{
  if (len > 12)
  {
#if defined(TESTING)
    Serial.printf("GMA MSC: sysex too long: %d...\r\n", len);
#endif
    return 0;
  }
  static uint8_t buffer[12] = {0};
  sprintf((char *)&buffer, "GMA");
  sprintf((char *)&buffer[4], "MSC");
  memcpy((void *)&buffer[8], &len, sizeof(uint32_t));

  if (dev_Udp.beginPacket(target, MSC_TX_PORT))
  {
    dev_Udp.write(&buffer[0], 12);
    dev_Udp.write(sysexBuffer, len);
    dev_Udp.endPacket(); // send frame
    return 12 + len;
  }
  else
  {
#if defined(TESTING)
    Serial.printf("GMA MSC: beginPacket failed...\r\n");
#endif
  }
  return 0;
}

uint16_t maMSC_t::update(void)
{

  static uint8_t buffer[MSC_PACKET_SIZE] = {0};

  int packetSize = dev_Udp.parsePacket();
  while (packetSize > 0)
  {

    if (packetSize <= MSC_PACKET_SIZE)
    {
      memset(&buffer[0],0,MSC_PACKET_SIZE);
      dev_Udp.read(&buffer[0], MSC_PACKET_SIZE);

      // check is a GMA\0MSC\0 packet
      if ((buffer[0] == 'G') & (buffer[1] == 'M') & (buffer[2] == 'A') & (buffer[3] == 0) & (buffer[4] == 'M') & (buffer[5] == 'S') & (buffer[6] == 'C') & (buffer[7] == 0))
      {

        int32_t size;
        memcpy(&size, &buffer[8], sizeof(int32_t));

        if ((buffer[12]==0xF0) & (buffer[13]==0x7F))
        {
          uint8_t deviceID = buffer[14];
          uint8_t cmd_fmt = buffer[16];
          uint8_t cmd_type = buffer[17];
          // buffer[18] is start of parameters

          uint16_t exec = 0;
          uint16_t page = 0;
          uint16_t value = 0;

          switch (cmd_type)
          {
          case MSC_SET:

           exec = buffer[18] + 1;
           page = buffer[19];

           value = (buffer[21] << 7) | (buffer[20] & 0x7F);   

          //memcpy(&value, &buffer[20], sizeof(int16_t));

            /* code */
#if defined(DEBUG_TESTING)
            Serial.printf(" > MSC In: SET Value: %d Exec: %d Page: %d\r\n", value, exec, page);
#endif            
            break;

          case MSC_GO:{

          // TODO: string to integers or float
            char cue[8];            
            if (strlen(&buffer[18]) < 8){
              strcpy(&cue[0],&buffer[18]);
            } 

            uint8_t pos = 18+(strlen(&cue[0]))+1;

            if (buffer[pos+1] == 0x2e)// is a decimal in exec.page
            {                    
              buffer[pos+1] = 0x00; // turn decimal place into a null
            }
            if (buffer[pos+3] == 0xf7) // is the terminator
            {                    
              buffer[pos+3] = 0x00; // turn decimal place into a null
            }

            exec = atoi(&buffer[pos]);
            page = atoi(&buffer[pos+2]);

#if defined(DEBUG_TESTING)
            Serial.printf(" > MSC In: GO %s Exec: %d Page: %d\r\n", cue, exec, page);
#endif
          }
            break;

          case MSC_RESET:  {        
            /* code */
         // TODO: string to integers or float
            char cue[8];            
            if (strlen(&buffer[18]) < 8){
              strcpy(&cue[0],&buffer[18]);
            } 

            uint8_t pos = 18+(strlen(&cue[0]))+1;

            if (buffer[pos+1] == 0x2e)// is a decimal in exec.page
            {                    
              buffer[pos+1] = 0x00; // turn decimal place into a null
            }
            if (buffer[pos+3] == 0xf7) // is the terminator
            {                    
              buffer[pos+3] = 0x00; // turn decimal place into a null
            }

            exec = atoi(&buffer[pos]);
            page = atoi(&buffer[pos+2]);

#if defined(DEBUG_TESTING)
            Serial.printf(" > MSC In: RESET %s Exec: %d Page: %d\r\n", cue, exec, page);
#endif
          }
            break;

/*
          case MSC_STOP:
           
            break;

          case MSC_RESUME:
           
            break;

          case MSC_TIMED_GO:
            
            break;

          case MSC_LOAD:
            
            break;

          case MSC_GO_OFF:
            
            break;
*/

          default:

#if defined(DEBUG_TESTING)
            Serial.printf("MSC In: PktSize: %d FrmSize: %d \r\n", packetSize, size);
            for (uint8_t cntr = 12; cntr < packetSize ; cntr++){
              Serial.printf("0x%02x ", buffer[cntr]);
            }
            Serial.printf("\n\r");
#endif
            break;
          }

        } // 
        else
        {
#if defined(DEBUG_TESTING)
          Serial.printf(" > MSC In: invalid framing. 0x%02x 0x%02x 0x%02x 0x%02x\r\n", buffer[12],buffer[13],buffer[14],buffer[15]);
#endif
        }

      } // is GMA\0MSC\0 packet

    } // packetsize < max

    else
    {
#if defined(DEBUG_TESTING)
      Serial.printf(" > MSC In: packet to big: %d\r\n!", packetSize);
#endif
    }

    packetSize = dev_Udp.parsePacket(); // read next packet, if any
  }                                     // while

  return 0;
}

#endif // USE_ETHERNET

#endif // _INCLUDE_MA_MSC_H_