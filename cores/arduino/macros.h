#pragma once
#ifdef USE_ARDUINO_PINOUT

#define analogPinToPinName(P)			(P < A0 ? g_APinDescription[P+A0].name : g_APinDescription[P].name)
#define digitalPinToPinName(P)			(g_APinDescription[P].name)
#define digitalPinToInterrupt(P)		(P)
#define digitalPinToInterruptObj(P)		(g_APinDescription[P].irq)
#define digitalPinToPwmObj(P)		    (g_APinDescription[P].pwm)

#else

#define analogPinToPinName(P)			((PinName)P)
#define digitalPinToPinName(P)			((PinName)P)
#define digitalPinToInterrupt(P) 		((PinName)P)

#endif