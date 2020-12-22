#ifndef _DEBUG_H
#define _DEBUG_H



// count loops per second, print to serial
static unsigned int UpdatesPerSecond = 0;
static unsigned int UpdatesPerSecondMin = 1000;
static unsigned int UpdatesPerSecondMax = 0;
  

static inline void fps(const unsigned int seconds = 1){
  static unsigned long lastMillis;
  static unsigned long frameCount;
    
  unsigned long now = millis();
  frameCount ++;
  if ((now - lastMillis) >= seconds * 1000) {          
      UpdatesPerSecond = frameCount / seconds;
      if (UpdatesPerSecondMin > UpdatesPerSecond) UpdatesPerSecondMin = UpdatesPerSecond;      
      if (UpdatesPerSecondMax < UpdatesPerSecond) UpdatesPerSecondMax = UpdatesPerSecond;
#if !defined(SERIAL_CLI_ENABLED)
    Serial.println(UpdatesPerSecond);
#endif
    frameCount = 0;
    lastMillis = now;
  }
}


#endif