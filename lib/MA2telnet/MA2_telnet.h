#ifndef _INCLUDE_MA2_TELNET_H_
#define _INCLUDE_MA2_TELNET_H_

#include "config.h"

#include "Arduino.h"
#include "stddef.h"

#if defined(USE_ETHERNET)
#include <SPI.h>
#include <Ethernet.h>

/* 
    Connect to an MA2 telnet service

    Unfinished as not sure if worth doing, there is not enough ram to process enough useful things...

*/

constexpr uint16_t MA2_TELNET_PORT = 30000;


class ma2_telnet_t {

   public: 
    
    bool connect (const char * ipAddress);     

    bool isConnected() { return conn; }

    void sendCmd(const char * command);

    uint8_t update();

    private:

    bool conn = false;  // true if connected
    
    EthernetClient client;
    IPAddress target; 
};


bool ma2_telnet_t::connect(const char * ipAddress) {
    
    if (client.connected() | conn == true )  {
        client.stop();  // disconnected if connected
        conn = false;
    }

    target.fromString(ipAddress);

    if (client.connect(target, MA2_TELNET_PORT)) {

        delay(50); // short delay

        while (client.available()) { 
            uint8_t junk = client.read(); 
#if defined (TESTING)
            Serial.print(junk);
#endif            
        } 

        client.println("login administrator admin");

        // TODO: test!
        conn = true;

    } else { 
        conn = false;
    }

    return conn;
}


void ma2_telnet_t::sendCmd(const char * command) {
    if (conn == true && client.connected()){
        client.write(command);
    }
}



#endif // USE_ETHERNET



#endif // _INCLUDE_MA2_TELNET_H_