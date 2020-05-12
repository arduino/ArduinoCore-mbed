/* This example demonstrates how to include the ThreadMRI library which allows debugging of the Portenta H7
   with GDB via a serial interface.
*/

#include <ThreadDebug.h>

//UartDebugCommInterface debugComm(SERIAL1_TX, SERIAL1_RX, 230400);
//ThreadDebug            threadDebug(&debugComm, DEBUG_BREAK_IN_SETUP);

UsbDebugCommInterface  debugComm(&SerialUSB);
ThreadDebug            threadDebug(&debugComm, DEBUG_BREAK_IN_SETUP);

void setup() {

}

void loop() {

}
