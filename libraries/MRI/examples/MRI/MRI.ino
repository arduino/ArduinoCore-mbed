/* This example demonstrates how to include the MRI library which
   allows to debug the Portenta H7 Cortex-M7 core with GDB via a
   serial interface.
*/

#include <MRI.h>

void setup() {
  Serial1.begin(115200);
  static DebugSerial debug(Serial1, USART1_IRQn);
  __debugbreak();
}

void loop() {

}
