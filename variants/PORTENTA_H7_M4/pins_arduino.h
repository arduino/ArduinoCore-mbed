#if defined(PORTENTA_H7_PINS)
#include "../PORTENTA_H7_M7/pins_arduino.h"
#elif defined(OPTA_PINS)
#include "../OPTA/pins_arduino.h"
#elif defined(NVISION_PINS)
#include "../NICLA_VISION/pins_arduino.h"
#elif defined(GIGA_PINS)
#include "../GIGA/pins_arduino.h"
#endif

#undef SERIAL_CDC
