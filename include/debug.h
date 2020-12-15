#ifndef _DEBUG_H
#define _DEBUG_H



// count loops per second, print to serial
static unsigned int UpdatesPerSecond;
  

static inline void fps(const unsigned int seconds = 1){
  static unsigned long lastMillis;
  static unsigned long frameCount;
    
  unsigned long now = millis();
  frameCount ++;
  if ((now - lastMillis) >= seconds * 1000) {
    UpdatesPerSecond = frameCount / seconds;
#if !defined(SERIAL_CLI_ENABLED)
    Serial.println(UpdatesPerSecond);
#endif
    frameCount = 0;
    lastMillis = now;
  }
}


#endif