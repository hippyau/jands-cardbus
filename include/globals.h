#ifndef _APP_GLOBALS_H_
#define _APP_GLOBALS_H_

#include "config.h"

#if defined(USE_ETHERNET)

#include <Ethernet.h>
#include <EthernetUdp.h>

static uint8_t mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; // Our MAC address
static uint16_t localPort = 8888;                            // Port to listen on

static IPAddress ip(192, 168, 1, 190);  // Our IP address
static IPAddress trg(192, 168, 1, 255); // Where we are sending to
//static IPAddress ip(169, 254, 1, 2);                        // Our IP address -- 169.254.254.1 is host, 2..254 are possible surfaces
//static IPAddress trg(169, 254, 1, 1);                       // host Where we are sending to

static EthernetUDP Udp;
static bool eth0_up = false;       // update / use the ethernet interface
static uint32_t eth0_stats_tx = 0; // bytes
static uint32_t eth0_stats_rx = 0;

#if defined(MA_MSC_UDP)
#include "MA_MSC_UDP_.h"
static maMSC_t maMSC;
#endif

#endif

#endif