#include "mbed/drivers/InterruptIn.h"
#include "mbed/drivers/PwmOut.h"
#include "mbed/drivers/AnalogIn.h"
#include "mbed/drivers/DigitalInOut.h"

struct _PinDescription
{
  PinName name;
  mbed::InterruptIn* irq;
  mbed::PwmOut* pwm;
  mbed::DigitalInOut* gpio;
};

struct _AnalogPinDescription
{
  PinName name;
  mbed::AnalogIn* adc;
};

#define analogPinToPinName(P)       (P >= PINS_COUNT ? NC : P < A0 ? g_APinDescription[P+A0].name : g_APinDescription[P].name)
#define analogPinToAdcObj(P)		(P < A0 ? g_AAnalogPinDescription[P].adc : g_AAnalogPinDescription[P-A0].adc)
//#define digitalPinToPinName(P)      (P >= PINS_COUNT ? NC : g_APinDescription[P].name)
#define digitalPinToInterruptObj(P) (g_APinDescription[P].irq)
#define digitalPinToPwm(P)          (g_APinDescription[P].pwm)
#define digitalPinToGpio(P)         (g_APinDescription[P].gpio)

inline PinName digitalPinToPinName(int P) {
	return (P >= PINS_COUNT ? NC : g_APinDescription[P].name);
};

int PinNameToIndex(PinName P);
