/* This example demonstrates how to include the ThreadMRI library which allows debugging of the Portenta H7
   with GDB via a serial (HardwareSerial or USBSerial) interface.
*/

#include <ThreadMRI.h>

//ThreadMRI g_debugger(Serial1, USART1_IRQn, 115200, THREADMRI_BREAK_ON_SETUP);
ThreadMRI g_debugger(Serial, OTG_HS_IRQn, 115200, THREADMRI_BREAK_ON_SETUP);
//ThreadMRI g_debugger(Serial, false);

extern "C" void testContext(void);

void setup() {
}

void loop() {

}
