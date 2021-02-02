#include "camera.h"

CameraClass cam;
uint8_t fb[320*240];

void setup() {

  Serial.begin(921600);

  // Init the cam QVGA, 30FPS
  cam.begin(CAMERA_R320x240, 30);
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
