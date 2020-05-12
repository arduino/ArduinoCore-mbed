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
/* Common functionality shared between gdb command handlers in mri. */
#ifndef CMD_COMMON_H_
#define CMD_COMMON_H_

#include <stdint.h>
#include <core/buffer.h>
#include <core/try_catch.h>

/* The bits that can be set in the return value from a command handler to indicate if the caller should return
   immediately or send the prepared response back to gdb.  It also indicates whether program execution should be
   resumed for commands like continue and single step. */
#define HANDLER_RETURN_RESUME_PROGRAM       (1 << 0)
#define HANDLER_RETURN_RETURN_IMMEDIATELY   (1 << 1)
#define HANDLER_RETURN_SKIPPED_OVER_BREAK   (1 << 2)
#define HANDLER_RETURN_HANDLED              (1 << 31)

typedef struct
{
    uint32_t address;
    uint32_t length;
} AddressLength;

/* Real name of functions are in mri namespace. */
__throws void     mriCmd_ReadAddressAndLengthArguments(Buffer* pBuffer, AddressLength* pArguments);
__throws void     mriCmd_ReadAddressAndLengthArgumentsWithColon(Buffer* pBuffer, AddressLength* pArguments);
__throws uint32_t mriCmd_ReadUIntegerArgument(Buffer* pBuffer);
__throws void     mriCmd_ThrowIfNextCharIsNotEqualTo(Buffer* pBuffer, char thisChar);

/* Macroes which allow code to drop the mri namespace prefix. */
#define ReadAddressAndLengthArguments           mriCmd_ReadAddressAndLengthArguments
#define ReadAddressAndLengthArgumentsWithColon  mriCmd_ReadAddressAndLengthArgumentsWithColon
#define ReadUIntegerArgument                    mriCmd_ReadUIntegerArgument
#define ThrowIfNextCharIsNotEqualTo             mriCmd_ThrowIfNextCharIsNotEqualTo

#endif /* CMD_COMMON_H_ */
