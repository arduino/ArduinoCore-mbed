
/* Define mock symbols to nullify PinMode definitions */
#define PullNone            TempPullNone 
#define PullUp              TempPullUp 
#define PullDown            TempPullDown 
#define OpenDrainPullUp     TempOpenDrainPullUp 
#define OpenDrainNoPull     TempOpenDrainNoPull 
#define OpenDrainPullDown   TempOpenDrainPullDown 
#define PushPullNoPull      TempPushPullNoPull     
#define PushPullPullUp      TempPushPullPullUp    
#define PushPullPullDown    TempPushPullPullDown  
#define OpenDrain           TempOpenDrain         
#define PullDefault         TempPullDefault       

#define INPUT           TempINPUT
#define OUTPUT          TempOUTPUT
#define INPUT_PULLUP    TempINPUT_PULLUP
#define INPUT_PULLDOWN  TempINPUT_PULLDOWN

/* Rename symbol PinMode into MbedPinMode for all the file PinNamesTypes.h
 * Functions using PinMode should be redeclared with the correct PinMode symbol */
#define PinMode MbedPinMode
#include "mbed_config.h"
#include "PinNamesTypes.h"
#undef PinMode

/* Rename symbol PinMode into ArduinoPinMode for all the file Common.h
 * Functions using PinMode should be redeclared with the correct PinMode symbol */
#define PinMode ArduinoPinMode
#include "api/Common.h"
#undef PinMode

#undef PullNone         
#undef PullUp           
#undef PullDown         
#undef OpenDrainPullUp  
#undef OpenDrainNoPull  
#undef OpenDrainPullDown 
#undef PushPullNoPull   
#undef PushPullPullUp    
#undef PushPullPullDown  
#undef OpenDrain         
#undef PullDefault       

#undef INPUT
#undef OUTPUT
#undef INPUT_PULLUP
#undef INPUT_PULLDOWN

/* Define the PinName symbol to be used in all the contexts */
typedef enum {
    PullNone          = TempPullNone,
    PullUp            = TempPullUp,
    PullDown          = TempPullDown,
    OpenDrainPullUp   = TempOpenDrainPullUp,
    OpenDrainNoPull   = TempOpenDrainNoPull,
    OpenDrainPullDown = TempOpenDrainPullDown,
    PushPullNoPull    = TempPushPullNoPull,
    PushPullPullUp    = TempPushPullPullUp,
    PushPullPullDown  = TempPushPullPullDown,
    OpenDrain         = TempOpenDrain,
    PullDefault       = TempPullDefault,
    INPUT = TempINPUT,
    OUTPUT = TempOUTPUT,
    INPUT_PULLUP = TempINPUT_PULLUP,
    INPUT_PULLDOWN = TempINPUT_PULLDOWN
} PinMode;

#if defined(__cplusplus)

/* Redeclare Common.h functions with the updated PinMode */
void pinMode(pin_size_t pinNumber, PinMode pinMode);

#endif