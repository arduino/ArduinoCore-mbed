#ifdef ARDUINO_AS_MBED_LIBRARY

#include "Arduino.h"
#include "pinDefinitions.h"

// generic variant

PinDescription g_APinDescription[PINS_COUNT];

void initVariant() {
	for (int i = 0; i<PINS_COUNT; i++) {
		g_APinDescription[i].name = (PinName)i;
	}
}

#endif