
/* Define mock symbols to nullify PinMode definitions */
#define PullNone        TempPullNone
#define PullDown        TempPullDown
#define PullUp          TempPullUp
#define PullDefault     TempPullDefault

#define INPUT           TempINPUT
#define OUTPUT          TempOUTPUT
#define INPUT_PULLUP    TempINPUT_PULLUP
#define INPUT_PULLDOWN  TempINPUT_PULLDOWN

/* Rename symbol PinMode into MbedPinMode for all the file PinNames.h
 * Functions using PinMode should be redeclared with the correct PinMode symbol */
#define PinMode MbedPinMode
#include "mbed_config.h"
#include "PinNames.h"
#undef PinMode

/* Rename symbol PinMode into ArduinoPinMode for all the file Common.h
 * Functions using PinMode should be redeclared with the correct PinMode symbol */
#define PinMode ArduinoPinMode
#include "api/Common.h"
#undef PinMode

#undef PullNone
#undef PullDown
#undef PullUp
#undef PullDefault

#undef INPUT
#undef OUTPUT
#undef INPUT_PULLUP
#undef INPUT_PULLDOWN

typedef enum {
    PullNone = TempPullNone,
    PullDown = TempPullDown,
    PullUp = TempPullUp,
    PullDefault = TempPullDefault,
    INPUT = TempINPUT,
    OUTPUT = TempOUTPUT,
    INPUT_PULLUP = TempINPUT_PULLUP,
    INPUT_PULLDOWN = TempINPUT_PULLDOWN
} PinMode;

#if defined(__cplusplus)

/* Redeclare Common.h functions with the updated PinMode */
void pinMode(pin_size_t pinNumber, PinMode pinMode);

#endif