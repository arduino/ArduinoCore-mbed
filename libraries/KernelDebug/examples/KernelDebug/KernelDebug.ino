/* This example demonstrates how to utilize the KernelDebug library which
   allows kernel debugging of the Portenta H7 with GDB over a UART serial
   connection.

   To connect to the target, launch gdb with the following parameters
 
   arm-none-eabi-gdb -ex "set pagination off" --baud {230400} -ex "set target-charset ASCII" -ex "target remote {debug.port}" {project_name}.elf

   The baud rate needs to match the one provided in KernelDebug constructor, while {debug.port} depends on the operating system (eg. /dev/ttyUSB0 or COM15)
*/

#include <KernelDebug.h>

KernelDebug kernelDebug(SERIAL1_TX, SERIAL1_RX, USART1_IRQn, 230400, DEBUG_BREAK_IN_SETUP);

void setup() {

}

void loop() {

}
