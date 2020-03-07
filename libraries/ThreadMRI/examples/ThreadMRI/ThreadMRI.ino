/* This example demonstrates how to include the MRI library which
   allows to debug the Portenta H7 Cortex-M7 core with GDB via a
   serial interface.
*/

#include <ThreadMRI.h>

ThreadMRI g_debugger;

void setup() {
    // UNDONE: Using Serial1 for now to unblock my progress.
    Serial1.begin(115200);
    g_debugger.debugException();
}

void loop() {

}
