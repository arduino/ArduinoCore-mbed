/* Copyright 2020 Adam Green (https://github.com/adamgreen/)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
// The GDB compatible debug monitor for kernel debugging of thread and handler mode code.
// Utilizes MRI (Monitor for Remote Inspection) library for core debugging functionality.
/* Example of using UART1 as interface to GDB.
    #include <KernelDebug.h>

    KernelDebug kernelDebug(SERIAL1_TX, SERIAL1_RX, USART1_IRQn, 230400, DEBUG_BREAK_ON_SETUP);

    void setup() {
        // Your setup code here.
        // GDB will automatically stop on the first line of this function.
    }

    void loop() {
        // Your loop code here.
    }
*/
#pragma once

#include <pinDefinitions.h>
#include <mbed.h>

namespace arduino {

// Pass as breakInSetup parameter of constructor to halt in setup().
#define DEBUG_BREAK_IN_SETUP    true
// Pass as breakInSetup parameter of constructors to NOT halt in setup().
#define DEBUG_NO_BREAK_IN_SETUP false


class KernelDebug {
public:
    // You must declare your KernelDebug object as a global.
    // Example:
    //      KernelDebug debug(SERIAL1_TX, SERIAL1_RX, USART1_IRQn, 230400, DEBUG_BREAK_ON_SETUP);
    KernelDebug(PinName txPin, PinName rxPin, IRQn_Type irq, uint32_t baudRate=230400, bool breakInSetup=DEBUG_BREAK_IN_SETUP);

    // You should never let your DebugSerial object go out of scope. Make it global or static. To warn you if you do
    // let it go out of scope by mistake, this destructor will break into GDB and then enter an infinite loop.
    ~KernelDebug();

protected:
    mbed::UnbufferedSerial  _serial;
};


// Use to insert a hardcoded breakpoint into your code.
#ifndef debugBreak
    #define debugBreak()  { __asm volatile ("bkpt #0"); }
#endif


// This class can be used instead of Serial for sending output to the PC via GDB.
class DebugSerial : public Print
{
public:
    DebugSerial();

    // Leaving out input Stream related methods so that compiler throws errors if user tries to use them.

    // Methods that must be implemented for Print subclasses.
    virtual size_t write(uint8_t);
    virtual size_t write(const uint8_t *buffer, size_t size);
    virtual void   flush();

    // Additional methods defined by HardwareSerial that user might call.
    void begin(unsigned long baud) { begin(baud, SERIAL_8N1); }
    void begin(unsigned long baudrate, uint16_t config);
    void end();
    operator bool() { return true; }

protected:
} extern DebugSerial;

} // namespace arduino
