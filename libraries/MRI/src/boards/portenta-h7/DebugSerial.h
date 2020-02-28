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
// GDB debugging of the Arduino Portenta-H7 over a serial connection.
#pragma once

#include <Arduino.h>

extern "C" {
    typedef struct Token Token;
    void     __mriPlatform_Init(Token* pParameterTokens);
    uint32_t __mriPlatform_CommHasReceiveData(void);
    int      __mriPlatform_CommReceiveChar(void);
    void     __mriPlatform_CommSendChar(int character);
}


namespace arduino {

class DebugSerial {
public:
    // You must declare your DebugSerial object as a global or function scoped static so that it doesn't get
    // destroyed. Which constructor you use, depends on where you declare it.

    // Use this constructor when declaring DebugSerial object as a global outside of any functions.
    // You must specify the baudrate so that DebugSerial can call begin() on the specified serial object.
    //  breakInSetup - should it break at beginning of setup().
    //
    // Global Example:
    //      DebugSerial debugSerial(Serial1, USART1_IRQn, 115200);
    DebugSerial(HardwareSerial& serial, IRQn_Type IRQn, uint32_t baudRate, bool breakInSetup=true);

    // Use this constructor when declaring DebugSerial object as a function scoped static. You must call begin() on
    // the serial object before constructing the DebugSerial object.
    //
    // Function Scoped Static Example:
    //      #include <MRI.h>
    //      void setup() {
    //          Serial1.begin(115200);
    //          static DebugSerial debugSerial(Serial1, USART1_IRQn);
    //          __debugbreak();
    //      }
    DebugSerial(HardwareSerial& serial, IRQn_Type IRQn);

    // You should never let your DebugSerial object go out of scope. Make it global or static. To warn you if you do
    // let it go out of scope by mistake, this destructor will break into GDB and then enter an infinite loop.
    ~DebugSerial();

protected:
    void        construct();
    void        callSerialBeginFromSetup();

    // These protected methods are called from global Platform* routines via singleton.
    void        setSerialPriority(uint32_t priority);
    uint32_t    hasReceiveData(void);
    int         receiveChar(void);
    void        sendChar(int character);

    static void _initSerial();
    void        initSerial();

    static void commInterruptHook(void);
    void        pendDebugMonAndRunCommIsr();
    void        handleAnyPendingCommInterrupts();

    static int  justEnteredSetupCallback(void* pvContext);
    int         justEnteredSetup();
    static int  justReturnedFromInitSerialCallback(void* pvContext);
    int         justReturnedFromInitSerial();

    typedef void(*IsrFunctionPtr)(void);

    char*           _pContextBuffer;
    IsrFunctionPtr  _commIsr;
    HardwareSerial& _serial;
    uint32_t        _contextBufferSize;
    uint32_t        _baudRate;
    uint32_t        _lrOrig;
    uint32_t        _pcOrig;
    IRQn_Type       _irq;
    bool            _breakInSetup;

    friend void     ::__mriPlatform_Init(Token* pParameterTokens);
    friend uint32_t ::__mriPlatform_CommHasReceiveData(void);
    friend int      ::__mriPlatform_CommReceiveChar(void);
    friend void     ::__mriPlatform_CommSendChar(int character);
};

} // namespace arduino
