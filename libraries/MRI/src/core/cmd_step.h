/* Copyright 2017 Adam Green (https://github.com/adamgreen/)

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
/* Handler for single step gdb command. */
#ifndef CMD_STEP_H_
#define CMD_STEP_H_

#include <stdint.h>

/* Real name of functions are in mri namespace. */
uint32_t mriCmd_HandleSingleStepCommand(void);
uint32_t mriCmd_HandleSingleStepWithSignalCommand(void);

/* Macroes which allow code to drop the mri namespace prefix. */
#define HandleSingleStepCommand           mriCmd_HandleSingleStepCommand
#define HandleSingleStepWithSignalCommand mriCmd_HandleSingleStepWithSignalCommand

#endif /* CMD_STEP_H_ */
