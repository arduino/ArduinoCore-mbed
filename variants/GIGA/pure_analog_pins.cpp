#include "pure_analog_pins.h"
#include "AnalogIn.h"
#include "pinDefinitions.h"

PureAnalogPin  A8(0);
PureAnalogPin  A9(1);
PureAnalogPin  A10(2);
PureAnalogPin  A11(3);


int getAnalogReadResolution();

int analogRead(PureAnalogPin pin) {
  mbed::AnalogIn* adc = g_pureAAnalogPinDescription[pin.get()].adc;
  auto name = g_pureAAnalogPinDescription[pin.get()].name;
  if (adc == NULL) {
    adc = new mbed::AnalogIn(name);
    g_pureAAnalogPinDescription[pin.get()].adc = adc;
  }
  return (adc->read_u16() >> (16 - getAnalogReadResolution()));
}
