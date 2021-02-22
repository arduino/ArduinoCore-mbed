#include "camera.h"

CameraClass cam;
uint8_t frameBuffer[320*240];

void setup() {

  Serial.begin(921600);

  // Init the cam QVGA, 30FPS
  cam.begin(CAMERA_R320x240, 30);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial) {
    // Grab frame and write to serial
    if (cam.grab(frameBuffer)) {
       Serial.write(frameBuffer, 320*240);
    }
  }
}
