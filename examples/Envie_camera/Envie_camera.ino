#include "camera.h"

CameraClass cam;

mbed::DigitalOut pin(PD_4);

void setup() {

  Serial.begin(115200);
  while (!Serial);
  pin = 1;

  // put your setup code here, to run once:
  cam.begin();
  cam.snapshot();

}

void loop() {
  // put your main code here, to run repeatedly:

}
