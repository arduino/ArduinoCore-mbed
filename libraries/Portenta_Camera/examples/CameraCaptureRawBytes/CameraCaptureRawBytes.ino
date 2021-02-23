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
  
  // Wait until the receiver acknowledges
  // that they are ready to receive new data
  while(Serial.read() != 1){};
  
  // Grab frame and write to serial
  if (cam.grab(fb) == 0) {
     Serial.write(fb, 320*240);       
  }
  
}
