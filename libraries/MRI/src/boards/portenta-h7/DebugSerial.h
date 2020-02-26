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
    #include "core/platforms.h"
}


namespace arduino {

class DebugSerial {
public:
    DebugSerial(HardwareSerial& serial, IRQn_Type IRQn);

    // You should never let your DebugSerial object go out of scope. Make it global or static. To warn you if you do
    // let it go out of scope by mistake, this destructor will break into GDB and then enter an infinite loop.
    ~DebugSerial();

protected:
    // These protected methods are called from global Platform* routines via singleton.
    void        setSerialPriority(uint32_t priority);
    void        initSerial();
    uint32_t    hasReceiveData(void);
    int         receiveChar(void);
    void        sendChar(int character);

    static void commInterruptHook(void);
    void        pendDebugMonAndRunCommIsr();
    void        handleAnyPendingCommInterrupts();

    typedef void(*IsrFunctionPtr)(void);

    IsrFunctionPtr  _commIsr;
    HardwareSerial& _serial;
    IRQn_Type       _irq;

    friend void     ::Platform_Init(Token* pParameterTokens);
    friend uint32_t ::Platform_CommHasReceiveData(void);
    friend int      ::Platform_CommReceiveChar(void);
    friend void     ::Platform_CommSendChar(int character);
};

} // namespace arduino
