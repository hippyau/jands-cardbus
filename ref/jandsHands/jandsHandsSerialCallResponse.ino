
/* no consecutive ports on Leonardo board */

// write data bus
void writed(uint8_t inb){
      digitalWrite(0, inb & (1 << 0));
      digitalWrite(1, inb & (1 << 1));
      digitalWrite(2, inb & (1 << 2));
      digitalWrite(3, inb & (1 << 3));
      digitalWrite(4, inb & (1 << 4));
      digitalWrite(5, inb & (1 << 5));
      digitalWrite(6, inb & (1 << 6));
      digitalWrite(7, inb & (1 << 7));
}

// read data bus
uint8_t readd(){
      int bin[8];
      bin[0] = digitalRead(0);
      bin[1] = digitalRead(1);
      bin[2] = digitalRead(2);
      bin[3] = digitalRead(3);
      bin[4] = digitalRead(4);
      bin[5] = digitalRead(5);
      bin[6] = digitalRead(6);
      bin[7] = digitalRead(7);
      return (128 * bin[7] + 64 * bin[6] + 32 * bin[5] + 16 * bin[4] + 8 * bin[3] + 4 * bin[2] + 2 * bin[1] + bin[0]);      
}

// change bus pin direction
void dirb(int dir){
    for (int c = 0 ; c < 8 ; c++) {
    pinMode(c, dir); // change to inputs          
  }
}


// write bus control signals
void writec(uint8_t inb){
     digitalWrite(8, inb & (1 << 0));
     digitalWrite(9, inb & (1 << 1));
     digitalWrite(10, inb & (1 << 2));
     digitalWrite(11, inb & (1 << 3));
     digitalWrite(12, inb & (1 << 4));
     digitalWrite(13, inb & (1 << 5));
}



void setup() {
  Serial.begin(57600);
  //while (!Serial) {
  //  ; // wait for serial port to connect. Needed for native USB port only
  //}

  for (int c = 0 ; c < 14 ; c++) {
    pinMode(c, OUTPUT);
  }

 // while (Serial.read() >= 0); // flush serial input

}


bool readmode = 1;
long previousMillis = 0;
uint8_t dbyt;

void loop() {
  unsigned long currentMillis = millis();

  if (readmode) {   // if readmode, we send the state of the bus back     
    if (currentMillis - previousMillis > 100) { // sample every 100ms
      previousMillis = currentMillis;      
      dbyt = readd(); // read it
      Serial.write(dbyt); // send it
    }
  }

  if (Serial.available() > 1) {   // if we get two bytes, write them to the output
    // get two incoming bytes
    uint8_t inByte = Serial.read();
    uint8_t inByte2 = Serial.read();

    writec(inByte2); // write bus control signals

    if (inByte2 & (1 << 0)) {  // bus direction control - bit 0 byte 2 - pin 8 - output high

      if (readmode){ // change direction?       
        dirb(OUTPUT);  // change to outputs                  
        readmode = 0;
      }
      writed(inByte); // write data bus             
    } 
     else // input
    {
      if (!readmode){ // change direction?       
        dirb (INPUT); // change to inputs                 
        readmode = 1;
      }
    }
  }

}
