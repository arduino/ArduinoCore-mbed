#ifdef USE_ARDUINO_PINOUT

#include "drivers/InterruptIn.h"
#include "drivers/PwmOut.h"
#include "drivers/AnalogIn.h"
#include "drivers/DigitalInOut.h"

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

#ifdef __cplusplus__
extern "C" {
#endif
inline PinName digitalPinToPinName(pin_size_t P) {
	return (P >= PINS_COUNT ? NC : g_APinDescription[P].name);
};
#ifdef __cplusplus__
}
#endif

int PinNameToIndex(PinName P);

#endif