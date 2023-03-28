#include <Wire.h>
#include "Goodix.h"  // Arduino_GT911_Library
#include "pinDefinitions.h"

Goodix touch = Goodix();

REDIRECT_STDOUT_TO(Serial);

#ifdef ARDUINO_GIGA
#define Wire Wire1
#define INT_PIN PinNameToIndex(PI_1)
#define RST_PIN PinNameToIndex(PI_2)
#endif

#ifdef ARDUINO_PORTENTA_H7_M7
#define Wire Wire2
#define INT_PIN PinNameToIndex(PD_4)
#define RST_PIN PinNameToIndex(PD_5)
#endif

uint16_t touchpad_x;
uint16_t touchpad_y;
bool touchpad_pressed;

void handleTouch(int8_t contacts, GTPoint *points) {
  touchpad_pressed = true;
  //printf("Contacts: %d\n", contacts);
  for (uint8_t i = 0; i < contacts; i++) {
    if (i == 0) {
      touchpad_x = points[i].x;
      touchpad_y = points[i].y;
    }
    //printf("C%d: #%d %d,%d s:%d\n", i, points[i].trackId, points[i].x, points[i].y, points[i].area);
    yield();
  }
}

void touchStart() {
  if (touch.begin(INT_PIN, RST_PIN, 0xDD) != true) {
    Serial.println("! Module reset failed");
  } else {
    Serial.println("Module reset OK");
  }

  Serial.print("Check ACK on addr request on 0x");
  Serial.print(touch.i2cAddr, HEX);

  Wire.beginTransmission(touch.i2cAddr);
  Wire.write(0);
  int error = Wire.endTransmission();
  if (error == 0) {
    Serial.println(": SUCCESS");
  } else {
    Serial.print(": ERROR #");
    Serial.println(error);
  }
}

void giga_touch_setup() {
  Wire.setClock(400000);
  Wire.begin();
  delay(300);

  touch.setHandler(handleTouch);
  touchStart();
}

void giga_touch_handler() {
  touch.loop();
  delay(1);
}

uint16_t giga_get_touch_x() {
  return touchpad_x;
}

uint16_t giga_get_touch_y() {
  return touchpad_y;
}
