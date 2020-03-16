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
// Library to enable GDB debugging of Thread Mode code running on the Adruino Portenta H7 over
// the USB serial connection.
#pragma once


class ThreadMRI {
public:
    // You must declare your ThreadMRI object as a global or function scoped static so that it doesn't get
    // destroyed. Which constructor you use, depends on where you declare it.

    // Use this constructor when declaring ThreadMRI object as a global outside of any functions.
    // You must specify the baudrate so that ThreadMRI can call begin() on the specified serial object.
    //  breakInSetup - should it break at beginning of setup().
    //
    // Global Example:
    //      ThreadMRI threadMRI(Serial1, USART1_IRQn, 115200);
    ThreadMRI(HardwareSerial& serial, IRQn_Type IRQn, uint32_t baudRate, bool breakInSetup=true);

    // Use this constructor when declaring ThreadMRI object as a function scoped static. You must call begin() on
    // the serial object before constructing the ThreadMRI object.
    //
    // Function Scoped Static Example:
    //      #include <MRI.h>
    //      void setup() {
    //          Serial1.begin(115200);
    //          static ThreadMRI threadMRI(Serial1, USART1_IRQn);
    //          __debugbreak();
    //      }
    ThreadMRI(HardwareSerial& serial, IRQn_Type IRQn);

    // You should never let your ThreadMRI object go out of scope. Make it global or static. To warn you if you do
    // let it go out of scope by mistake, this destructor will break into GDB and then enter an infinite loop.
    ~ThreadMRI();

protected:
    void init(HardwareSerial& serial, IRQn_Type IRQn, uint32_t baudRate, bool breakInSetup);
};

// Use to insert a hardcoded breakpoint into your code.
#ifndef __debugbreak
    #define __debugbreak()  { __asm volatile ("bkpt #0"); }
#endif

