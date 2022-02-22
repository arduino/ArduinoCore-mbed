#include "camera.h"

#ifdef ARDUINO_NICLA_VISION
  #include "gc2145.h"
  GC2145 galaxyCore;
  Camera cam(galaxyCore);
  #define IMAGE_MODE CAMERA_RGB565
#else
  #include "himax.h"
  HM01B0 himax;
  Camera cam(himax);
  #define IMAGE_MODE CAMERA_GRAYSCALE
#endif

/*
Other buffer instantiation options:
  FrameBuffer fb(0x30000000);
  FrameBuffer fb(320,240,2);
*/
FrameBuffer fb;

unsigned long lastUpdate = 0;


void blinkLED(uint32_t count = 0xFFFFFFFF)
{
  pinMode(LED_BUILTIN, OUTPUT);
  while (count--) {
    digitalWrite(LED_BUILTIN, LOW);  // turn the LED on (HIGH is the voltage level)
    delay(50);                       // wait for a second
    digitalWrite(LED_BUILTIN, HIGH); // turn the LED off by making the voltage LOW
    delay(50);                       // wait for a second
  }
}

void setup() {
  // Init the cam QVGA, 30FPS
  if (!cam.begin(CAMERA_R320x240, IMAGE_MODE, 30)) {
    blinkLED();
  }

  blinkLED(5);
}

void loop() {
  if(!Serial) {    
    Serial.begin(921600);
    while(!Serial);
  }

  // Time out after 2 seconds and send new data
  bool timeoutDetected = millis() - lastUpdate > 2000;
  
  // Wait for sync byte.
  if(!timeoutDetected && Serial.read() != 1) return;  

  lastUpdate = millis();
  
  // Grab frame and write to serial
  if (cam.grabFrame(fb, 3000) == 0) {
    Serial.write(fb.getBuffer(), cam.frameSize());
  } else {
    blinkLED(20);
  }
}
