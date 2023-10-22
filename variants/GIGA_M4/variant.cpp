#define initVariant   initVariantUnsused

#if defined(PORTENTA_H7_PINS)
#include "../PORTENTA_H7_M7/variant.cpp"
#elif defined(OPTA_PINS)
#include "../OPTA/variant.cpp"
#elif defined(NVISION_PINS)
#include "../NICLA_VISION/variant.cpp"
#elif defined(GIGA_PINS)
#include "../GIGA/variant.cpp"
#endif

#undef initVariant

void initVariant() {}
