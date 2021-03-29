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
// The GDB compatible debug monitor for debugging application threads.
// Utilizes MRI (Monitor for Remote Inspection) library for core debugging functionality.
/* Example of using SerialUSB as interface to GDB.
    #include <ThreadDebug.h>

    UsbDebugCommInterface  debugComm(&SerialUSB);
    ThreadDebug            threadDebug(&debugComm, DEBUG_BREAK_ON_SETUP);

    void setup() {
        // Your setup code here.
        // GDB will automatically stop on the first line of this function.
    }

    void loop() {
        // Your loop code here.
    }
*/
#pragma once

#include <Arduino.h>
#ifdef SERIAL_CDC
#include <USB/PluggableUSBSerial.h>
#endif
#include "mbed.h"

#if defined(STM32H747xx) && defined(CORE_CM4)
// include RPC out of arduino namespace
#include "SerialRPC.h"
#endif

namespace arduino {


// Pass as breakInSetup parameter of constructor to halt in setup().
#define DEBUG_BREAK_IN_SETUP    true
// Pass as breakInSetup parameter of constructors to NOT halt in setup().
#define DEBUG_NO_BREAK_IN_SETUP false


class DebugCommInterface;

class ThreadDebug {
public:
    // You must declare your ThreadDebug object as a global.
    // Examples:
    //      UartDebugCommInterface debugComm(SERIAL1_TX, SERIAL1_RX, 230400);
    //      ThreadDebug            threadDebug(&debugComm, DEBUG_BREAK_ON_SETUP);
    //              -- OR --
    //      UsbDebugCommInterface  debugComm(&SerialUSB);
    //      ThreadDebug            threadDebug(&debugComm, DEBUG_BREAK_ON_SETUP);
    ThreadDebug(DebugCommInterface* pCommInterface, bool breakInSetup=DEBUG_BREAK_IN_SETUP, uint32_t maxThreadCount=32);

    // Your ThreadDebug object should never go out of scope. To warn you if you do let it go out of scope by mistake,
    // this destructor will break into GDB and then enter an infinite loop.
    ~ThreadDebug();
};


// Interface used to communicate with GDB.
// The currently supported interfaces can be found below, after this inteface.
class DebugCommInterface {
public:
    virtual ~DebugCommInterface() = 0;

    virtual bool readable() = 0;
    virtual bool writeable() = 0;
	virtual uint8_t read() = 0;
	virtual void write(uint8_t c) = 0;
    virtual void attach(void (*pCallback)()) = 0;
};

#ifdef SERIAL_CDC

#undef SerialUSB
#define SerialUSB _SerialUSB

// Use the SerialUSB interface to communicate with GDB.
class UsbDebugCommInterface : public DebugCommInterface {
public:
    UsbDebugCommInterface(arduino::USBSerial*);
    virtual ~UsbDebugCommInterface();

    virtual bool readable();
    virtual bool writeable();
	virtual uint8_t read();
	virtual void write(uint8_t c);
    virtual void attach(void (*pCallback)());

protected:
    arduino::USBSerial*  _pSerial;
};
#endif

#if defined(STM32H747xx) && defined(CORE_CM4)
// Use the RPC interface to communicate with GDB from M4 core
class RPCDebugCommInterface : public DebugCommInterface {
public:
    RPCDebugCommInterface(arduino::SerialRPCClass*);
    virtual ~RPCDebugCommInterface();

    virtual bool readable();
    virtual bool writeable();
    virtual uint8_t read();
    virtual void write(uint8_t c);
    virtual void attach(void (*pCallback)());

protected:
    arduino::SerialRPCClass*  _pSerial;
};
#endif

// Use one of the device's hardware UARTs to communicate with GDB.
class UartDebugCommInterface : public DebugCommInterface {
public:
    UartDebugCommInterface(PinName txPin, PinName rxPin, uint32_t baudRate=230400);
    virtual ~UartDebugCommInterface();

    virtual bool readable();
    virtual bool writeable();
	virtual uint8_t read();
	virtual void write(uint8_t c);
    virtual void attach(void (*pCallback)());

protected:
    uint32_t wrappingIncrement(uint32_t val);
    void onReceivedData();

    void                    (* _pCallback)();
    mbed::UnbufferedSerial  _serial;
    uint32_t                _baudRate;
    volatile uint32_t       _read;
    volatile uint32_t       _write;
    uint8_t                 _queue[8];
};


// Use to insert a hardcoded breakpoint into your code.
#ifndef debugBreak
    #define debugBreak()  { __asm volatile ("bkpt #0"); }
#endif

} // namespace arduino
