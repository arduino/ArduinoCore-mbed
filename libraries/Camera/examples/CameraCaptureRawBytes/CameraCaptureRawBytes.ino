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


void blink_error(uint32_t count = 0xFFFFFFFF)
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
  Serial.begin(921600);

  // Init the cam QVGA, 30FPS
  if (cam.begin(CAMERA_R320x240, IMAGE_MODE, 30) != 0) {
    blink_error();
  }

  blink_error(5);
}

void loop() {
  // Wait for sync byte.
  while (Serial.read() != 1) { };
  
  // Grab frame and write to serial
  if (cam.GrabFrame(fb, 3000) == 0) {
    Serial.write(fb.getBuffer(), cam.FrameSize());
  } else {
    blink_error();
  }
}
