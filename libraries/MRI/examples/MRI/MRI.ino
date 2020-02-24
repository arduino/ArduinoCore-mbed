/* This example demonstrates how to include the MRI library which
   allows to debug the Portenta H7 Cortex-M7 core with GDB via a
   serial interface.
*/

#include <MRI.h>

void setup() {
  Serial1.begin(115200); /* Initialize Serial1 via mbed ... should actually be done within MRI library to have the configuration we need */
  __mriInit("");
}

void loop() {

}
