#include "camera.h"

CameraClass cam;
bool motionDetected = false;

void onMotion() {
  motionDetected = true;
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
  cam.motionDetection(true, onMotion);
}

void loop() {

  if (motionDetected) {
    Serial.printf("Motion Detected!\n");
    digitalWrite(LEDB, LOW);
    delay(500);

    // Clear the detection flag
    cam.motionDetected();
    motionDetected = false;
    digitalWrite(LEDB, HIGH);
  }
  delay(10);
}
