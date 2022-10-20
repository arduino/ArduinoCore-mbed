#ifndef _PURE_ANALOG_PINS_
#define _PURE_ANALOG_PINS_

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include "Arduino.h"

/******************************************************************************
 * PREPROCESSOR-MAGIC
 ******************************************************************************/

#define PURE_ANALOG_AS_DIGITAL_ATTRIBUTE __attribute__ ((error("Can't use pins A8-A11 as digital")))

/******************************************************************************
 * TYPEDEF
 ******************************************************************************/

class PureAnalogPin {
public:
	PureAnalogPin(int _pin) : pin(_pin) {};
	int get() {
		return pin;
	};
	bool operator== (PureAnalogPin const & other) const {
		return pin == other.pin;
	}
	//operator int() = delete;
	__attribute__ ((error("Change me to a #define"))) operator int();
private:
	int pin;
};

extern PureAnalogPin  A8;
extern PureAnalogPin  A9;
extern PureAnalogPin  A10;
extern PureAnalogPin  A11;

/******************************************************************************
 * FUNCTION DECLARATION
 ******************************************************************************/

void      PURE_ANALOG_AS_DIGITAL_ATTRIBUTE pinMode     (PureAnalogPin pin, PinMode mode);
PinStatus PURE_ANALOG_AS_DIGITAL_ATTRIBUTE digitalRead (PureAnalogPin pin);
void      PURE_ANALOG_AS_DIGITAL_ATTRIBUTE digitalWrite(PureAnalogPin pin, PinStatus value);
int       analogRead  (PureAnalogPin pin);
void      PURE_ANALOG_AS_DIGITAL_ATTRIBUTE analogWrite (PureAnalogPin pin, int value);

#undef PURE_ANALOG_AS_DIGITAL_ATTRIBUTE

#endif /* _PURE_ANALOG_PINS_ */
