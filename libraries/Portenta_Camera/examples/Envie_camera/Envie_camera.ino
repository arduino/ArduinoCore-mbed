#include "camera.h"

CameraClass cam;

void setup() {

  Serial.begin(115200);
  //while (!Serial);

  // put your setup code here, to run once:
  cam.begin(324, 244);
  cam.start();
  //cam.testPattern(true);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial) {
    Serial.write(cam.grab(), 324 * 244);
  }
}
