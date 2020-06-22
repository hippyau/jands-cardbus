#ifndef _DEBUG_H
#define _DEBUG_H



// count loops per second, print to serial

static inline void fps(const unsigned int seconds = 1){
  static unsigned long lastMillis;
  static unsigned long frameCount;
  static unsigned int framesPerSecond;
    
  unsigned long now = millis();
  frameCount ++;
  if ((now - lastMillis) >= seconds * 1000) {
    framesPerSecond = frameCount / seconds;
    Serial.println(framesPerSecond);
    frameCount = 0;
    lastMillis = now;
  }
}


#endif