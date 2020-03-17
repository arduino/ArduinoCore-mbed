/* This example demonstrates how to include the MRI library which
   allows debugging the Portenta H7 Cortex-M7 core with GDB via a
   serial interface.
*/

#include <ThreadMRI.h>

//ThreadMRI g_debugger(Serial1, USART1_IRQn, 115200, true);
ThreadMRI g_debugger(Serial, OTG_HS_IRQn, 115200, true);

extern "C" void testContext(void);

void setup() {
    //testContext();
    //__debugbreak();
}

void loop() {

}
