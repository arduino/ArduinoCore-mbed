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
/* Breakpoint constants shared between assembly language stubs and MRI's semihost hooks. */

#ifndef MRI_NEWLIB_STUBS_H_
#define MRI_NEWLIB_STUBS_H_

#define MRI_NEWLIB_SEMIHOST_MIN         0xf5

#define MRI_NEWLIB_SEMIHOST_SET_HOOKS   0xf5
#define MRI_NEWLIB_SEMIHOST_GET_ERRNO   0xf6
#define MRI_NEWLIB_SEMIHOST_WRITE       0xf7
#define MRI_NEWLIB_SEMIHOST_READ        0xf8
#define MRI_NEWLIB_SEMIHOST_OPEN        0xf9
#define MRI_NEWLIB_SEMIHOST_RENAME      0xfa
#define MRI_NEWLIB_SEMIHOST_UNLINK      0xfb
#define MRI_NEWLIB_SEMIHOST_STAT        0xfc
#define MRI_NEWLIB_SEMIHOST_LSEEK       0xfd
#define MRI_NEWLIB_SEMIHOST_CLOSE       0xfe
#define MRI_NEWLIB_SEMIHOST_FSTAT       0xff

#define MRI_NEWLIB_SEMIHOST_MAX         0xff

#endif /* MRI_NEWLIB_STUBS_H_ */
