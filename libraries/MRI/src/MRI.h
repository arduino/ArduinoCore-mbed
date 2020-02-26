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
// Library to enable GDB debugging of the Arduino Portenta-H7 over a serial connection.
#pragma once

#include "boards/portenta-h7/DebugSerial.h"

// Use to insert a hardcoded breakpoint into your code.
#ifndef __debugbreak
    #define __debugbreak()  { __asm volatile ("bkpt #0"); }
#endif

