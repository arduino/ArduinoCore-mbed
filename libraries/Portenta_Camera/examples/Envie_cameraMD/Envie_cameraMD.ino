#include "camera.h"

CameraClass cam;
uint8_t fb[320*240];
bool motion_detected = false;

void on_motion()
{
  motion_detected = true;
}

void setup() {
  Serial.begin(115200);

  pinMode(LEDB, OUTPUT);
  digitalWrite(LEDB, HIGH);
  
  // Init the cam QVGA, 30FPS
  cam.begin(CAMERA_R320x240, 60);

  cam.setMDThreshold(100, 200);
  cam.setMDWindow(0, 0, 320, 240);
  cam.enableMD(true, on_motion);
}

void loop() {
  // put your main code here, to run repeatedly:  
  if (motion_detected) {
    Serial.printf("Motion Detected!\n");
    digitalWrite(LEDB, LOW);
    delay(500);
    cam.clearMDFlag();
    motion_detected =false;
    digitalWrite(LEDB, HIGH);
  }
  delay(10);
}
