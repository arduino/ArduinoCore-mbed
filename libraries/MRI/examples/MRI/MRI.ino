/* This example demonstrates how to include the MRI library which
   allows to debug the Portenta H7 Cortex-M7 core with GDB via a
   serial interface.
*/

#include <MRI.h>

DebugSerial debugSerial(SERIAL1_TX, SERIAL1_RX, USART1_IRQn, 230400);

void setup() {
}

void loop() {

}
