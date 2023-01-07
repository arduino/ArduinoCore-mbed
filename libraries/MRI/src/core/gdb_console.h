/* Copyright 2022 Adam Green (https://github.com/adamgreen/)

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
/* Routines to output text to stdout on the gdb console. */
#ifndef GDB_CONSOLE_H_
#define GDB_CONSOLE_H_

#include <stdint.h>

/* Real name of functions are in mri namespace. */
size_t mriGdbConsole_WriteString(const char* pString);
size_t mriGdbConsole_WriteSizedString(const char* pString, size_t length);
void mriGdbConsole_WriteHexValue(uint32_t value);

/* Macroes which allow code to drop the mri namespace prefix. */
#define WriteStringToGdbConsole      mriGdbConsole_WriteString
#define WriteSizedStringToGdbConsole mriGdbConsole_WriteSizedString
#define WriteHexValueToGdbConsole    mriGdbConsole_WriteHexValue

#endif /* GDB_CONSOLE_H_ */
