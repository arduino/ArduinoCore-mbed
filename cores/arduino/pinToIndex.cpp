#include "Arduino.h"
#include "pinDefinitions.h"

int PinNameToIndex(PinName P) {
  for (pin_size_t i=0; i < PINS_COUNT; i++) {
    if (g_APinDescription[i].name == P) {
      return i;
    }
  }
  return NOT_A_PIN;
}

#ifdef __cplusplus__
extern "C" {
#endif
PinName digitalPinToPinName(pin_size_t P) {
  return (P >= PINS_COUNT ? NC : g_APinDescription[P].name);
};
#ifdef __cplusplus__
}
#endif