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

#include <mbed.h>


class DebugCommInterface {
public:
    virtual ~DebugCommInterface() = 0;

    virtual bool available() = 0;
	virtual uint8_t read() = 0;
	virtual void write(uint8_t c) = 0;
    virtual void attach(void (*pCallback)()) = 0;
};

class UartDebugCommInterface : public DebugCommInterface {
public:
    UartDebugCommInterface(PinName txPin, PinName rxPin, uint32_t baudRate);
    virtual ~UartDebugCommInterface();

    virtual bool available();
	virtual uint8_t read();
	virtual void write(uint8_t c);
    virtual void attach(void (*pCallback)());

protected:
    uint32_t wrappingIncrement(uint32_t val);
    void onReceivedData();

    void                    (* _pCallback)();
    mbed::UnbufferedSerial  _serial;
    volatile uint32_t       _read;
    volatile uint32_t       _write;
    uint8_t                 _queue[8];
};

class UsbDebugCommInterface : public DebugCommInterface {
public:
    UsbDebugCommInterface(arduino::USBSerial*);
    virtual ~UsbDebugCommInterface();

    virtual bool available();
	virtual uint8_t read();
	virtual void write(uint8_t c);
    virtual void attach(void (*pCallback)());

protected:
    arduino::USBSerial*  _pSerial;
};


// Pass THREADMRI_BREAK_ON_SETUP as breakInSetup parameter of ThreadMRI constructors to halt in setup().
#define THREADMRI_BREAK_ON_SETUP    true
// Pass THREADMRI_NO_BREAK_ON_SETUP as breakInSetup parameter of ThreadMRI constructors to NOT halt in setup().
#define THREADMRI_NO_BREAK_ON_SETUP false


class ThreadMRI {
public:
    // You must declare your ThreadMRI object as a global.
    // Examples:
    //      UartDebugCommInterface g_comm(SERIAL1_TX, SERIAL1_RX, 230400);
    //      ThreadMRI              g_debugSerial(&g_comm, THREADMRI_BREAK_ON_SETUP);
    //              -- OR --
    //      UsbDebugCommInterface  g_comm(&SerialUSB);
    //      ThreadMRI              g_debugSerial(&g_comm, THREADMRI_BREAK_ON_SETUP);
    ThreadMRI(DebugCommInterface* pCommInterface, bool breakInSetup=THREADMRI_BREAK_ON_SETUP);

    // Your ThreadMRI object should never go out of scope. To warn you if you do let it go out of scope by mistake,
    // this destructor will break into GDB and then enter an infinite loop.
    ~ThreadMRI();
};


// Use to insert a hardcoded breakpoint into your code.
#ifndef __debugbreak
    #define __debugbreak()  { __asm volatile ("bkpt #0"); }
#endif

