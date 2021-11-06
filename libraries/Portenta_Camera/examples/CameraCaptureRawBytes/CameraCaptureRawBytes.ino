#include "camera.h"
#include "SDRAM.h"

//REDIRECT_STDOUT_TO(Serial);

// Uncomment for HM01B0
#include "himax.h"
Camera cam(new HM01B0());

// Uncomment to use GC2145 instead
//#include "gc2145.h"
//Camera cam(new GC2145());

uint8_t *fb = (uint8_t*) SDRAM_START_ADDRESS;

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

  // Init DRAM and reserve 2MB for framebuffer.
  SDRAM.begin(SDRAM_START_ADDRESS + 2 * 1024 * 1024);  

  // Init the cam QVGA, 30FPS
  if (cam.begin(CAMERA_R320x240, CAMERA_GRAYSCALE, 30) != 0) {
    blink_error();
  }
  
  //Serial.print("Sensor ID:");
  //Serial.println(cam.GetID());
  blink_error(5);
}

void loop() {
  // Wait for sync byte.
  while (Serial.read() != 1) { };
  
  // Grab frame and write to serial
  if (cam.GrabFrame(fb, 3000) == 0) {
    Serial.write(fb, cam.FrameSize());
  } else {
    blink_error();
  } 
}
