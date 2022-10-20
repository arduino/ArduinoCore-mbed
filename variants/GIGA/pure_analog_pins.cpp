#include "pure_analog_pins.h"
#include "AnalogIn.h"
#include "pinDefinitions.h"

PureAnalogPin  A8(8);
PureAnalogPin  A9(9);
PureAnalogPin  A10(10);
PureAnalogPin  A11(11);

int getAnalogReadResolution();

int analogRead(PureAnalogPin pin) {
  mbed::AnalogIn* adc = g_AAnalogPinDescription[pin.get()].adc;
  auto name = g_AAnalogPinDescription[pin.get()].name;
  if (adc == NULL) {
    adc = new mbed::AnalogIn(name);
    g_AAnalogPinDescription[pin.get()].adc = adc;
  }
  return (adc->read_u16() >> (16 - getAnalogReadResolution()));
}
