/* This example demonstrates how to utilize the KernelDebug library which
   allows kernel debugging of the Portenta H7 with GDB over a UART serial
   connection.
*/

#include <KernelDebug.h>

KernelDebug kernelDebug(SERIAL1_TX, SERIAL1_RX, USART1_IRQn, 230400, DEBUG_BREAK_ON_SETUP);

void setup() {

}

void loop() {

}
