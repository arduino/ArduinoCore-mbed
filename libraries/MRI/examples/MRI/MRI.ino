/* This example demonstrates how to include the MRI library which
   allows to debug the Portenta H7 Cortex-M7 core with GDB via a
   serial interface.
*/

#include <MRI.h>

DebugSerial debugSerial(Serial1, USART1_IRQn, 115200);

void setup() {
}

void loop() {

}
