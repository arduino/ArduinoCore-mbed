/* Copyright 2014 Adam Green (https://github.com/adamgreen/)

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
/* Semihost functionality for redirecting operations such as file I/O to the GNU debugger. */
#ifndef SEMIHOST_H_
#define SEMIHOST_H_

#include <core/platforms.h>

/* Real name of functions are in mri namespace. */
int mriSemihost_IsDebuggeeMakingSemihostCall(void);
int mriSemihost_HandleSemihostRequest(void);
int mriSemihost_HandleNewlibSemihostRequest(PlatformSemihostParameters* pSemihostParameters);
int mriSemihost_HandleMbedSemihostRequest(PlatformSemihostParameters* pParameters);

/* Macroes which allow code to drop the mri namespace prefix. */
#define Semihost_IsDebuggeeMakingSemihostCall   mriSemihost_IsDebuggeeMakingSemihostCall
#define Semihost_HandleSemihostRequest          mriSemihost_HandleSemihostRequest
#define Semihost_HandleNewlibSemihostRequest    mriSemihost_HandleNewlibSemihostRequest
#define Semihost_HandleMbedSemihostRequest      mriSemihost_HandleMbedSemihostRequest

#endif /* SEMIHOST_H_ */
