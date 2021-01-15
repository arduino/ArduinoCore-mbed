
#include "camera.h"

CameraClass cam;
uint8_t fb[320*240] __attribute__((aligned(32)));

void setup() {

  Serial.begin(921600);

  // Init the cam
  cam.begin(320, 240);

  // Skip 60 frames
  cam.skip_frames(fb, 60);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial) {
    // Grab frame and write to serial
    if (cam.grab(fb) == 0) {
      Serial.write(fb, 320*240);
    }
  }
}
