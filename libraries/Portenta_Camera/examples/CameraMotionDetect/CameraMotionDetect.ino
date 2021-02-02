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

  cam.motionDetectionThreshold(100, 200);
  cam.motionDetectionWindow(0, 0, 320, 240);
  // The detection can also be enabled without any callback
  cam.motionDetection(true, on_motion);
}

void loop() {

  if (motion_detected) {
    Serial.printf("Motion Detected!\n");
    digitalWrite(LEDB, LOW);
    delay(500);

    // Clear the detection flag
    cam.motionDetected();
    motion_detected = false;
    digitalWrite(LEDB, HIGH);
  }
  delay(10);
}
