#include "camera.h"

// uncomment the correct camera in use
#include "hm0360.h"
HM0360 himax;

// #include "himax.h"
// HM01B0 himax;

Camera cam(himax);

#ifdef ARDUINO_NICLA_VISION
  #error "GalaxyCore camera module does not support Motion Detection"
#endif

bool motion_detected = false;

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

void on_motion()
{
  motion_detected = true;
}

void setup() {
  Serial.begin(115200);

  pinMode(LEDB, OUTPUT);
  digitalWrite(LEDB, HIGH);

  // Init the cam QVGA, 30FPS
  if (!cam.begin(CAMERA_R320x240, CAMERA_GRAYSCALE, 30)) {
    blinkLED();
  }

  // Set motion detection threshold (0 -> 255).
  // The lower the threshold the higher the sensitivity.
  cam.setMotionDetectionThreshold(0);

  // Set motion detection window/ROI.
  cam.setMotionDetectionWindow(0, 0, 320, 240);

  // The detection can also be enabled without any callback
  cam.enableMotionDetection(on_motion);
}

void loop() {

  if (motion_detected) {
    Serial.println("Motion Detected!");
    digitalWrite(LEDB, LOW);
    delay(500);

    // Clear the detection flag
    cam.motionDetected();
    motion_detected = false;
    digitalWrite(LEDB, HIGH);
  }
  delay(10);
}
