/* This example demonstrates how to include the ThreadMRI library which allows debugging of the Portenta H7
   with GDB via a serial (HardwareSerial or USBSerial) interface.
*/

#include <ThreadMRI.h>

//UartDebugCommInterface g_comm(SERIAL1_TX, SERIAL1_RX, 230400);
//ThreadMRI              g_debugSerial(&g_comm, THREADMRI_BREAK_ON_SETUP);

UsbDebugCommInterface  g_comm(&SerialUSB);
ThreadMRI              g_debugSerial(&g_comm, THREADMRI_BREAK_ON_SETUP);

extern "C" void testContext(void);

void setup() {
}

void loop() {

}
