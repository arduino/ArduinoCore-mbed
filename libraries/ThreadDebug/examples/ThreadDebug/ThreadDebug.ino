/* This example demonstrates how to include the ThreadMRI library which allows debugging of the Portenta H7 and Nano 33 BLE [Sense]
   with GDB via a serial interface.

   To connect to the target, launch gdb with the following parameters

   arm-none-eabi-gdb -ex "set pagination off" --baud {230400} -ex "set target-charset ASCII" -ex "target remote {debug.port}" {project_name}.elf

   The baud rate needs to match the one provided in UartDebugCommInterface constructor, while {debug.port} depends on the operating system (eg. /dev/ttyUSB0 or COM15).

   If UsbDebugCommInterface is being used you can specify any baudrate.
*/

#include <ThreadDebug.h>

//UartDebugCommInterface debugComm(SERIAL1_TX, SERIAL1_RX, 230400);
//ThreadDebug            threadDebug(&debugComm, DEBUG_BREAK_IN_SETUP);

UsbDebugCommInterface  debugComm(&SerialUSB);
ThreadDebug            threadDebug(&debugComm, DEBUG_BREAK_IN_SETUP);

// Redirect Serial.print*() output to GDB instead of SerialUSB where it would conflict with ThreadDebug.
// NOTE: Calls to Serial.print*() will block waiting for GDB to be connected so only useful to use this redefine
//       when actively debugging the program.
#undef Serial
#define Serial DebugSerial

void setup() {

}

void loop() {
    static uint32_t count;

    Serial.print("Iteration: "); Serial.println(count++);
}
