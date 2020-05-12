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
/* Handler for continue gdb command. */
#ifndef CMD_CONTINUE_H_
#define CMD_CONTINUE_H_

#include <stdint.h>

/* Real name of functions are in mri namespace. */
uint32_t mriCmd_ContinueExecution(int setPC, uint32_t newPC);
uint32_t mriCmd_SkipHardcodedBreakpoint(void);
uint32_t mriCmd_HandleContinueCommand(void);
uint32_t mriCmd_HandleContinueWithSignalCommand(void);
uint32_t mriCmd_HandleDetachCommand(void);

/* Macroes which allow code to drop the mri namespace prefix. */
#define ContinueExecution               mriCmd_ContinueExecution
#define SkipHardcodedBreakpoint         mriCmd_SkipHardcodedBreakpoint
#define HandleContinueCommand           mriCmd_HandleContinueCommand
#define HandleContinueWithSignalCommand mriCmd_HandleContinueWithSignalCommand
#define HandleDetachCommand             mriCmd_HandleDetachCommand

#endif /* CMD_CONTINUE_H_ */
