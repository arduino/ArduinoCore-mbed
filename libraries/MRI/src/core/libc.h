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
/*  Implementation of Standard C Library functions that MRI makes use of.

    Having its own implementation of these means that users won't hit difficulties if they try to single step
    through the real Standard C Library.

    NOTE: The debuggee shouldn't call these versions as that would defeat their purpose.
*/
#ifndef LIBC_H_
#define LIBC_H_

#include <stddef.h>

void* mri_memcpy(void* pDest, const void* pSrc, size_t len);
void* mri_memset(void* pDest, int val, size_t len);
int mri_strcmp(const char* p1, const char* p2);
int mri_strncmp(const char* p1, const char* p2, size_t len);
size_t mri_strlen(const char* p);
char* mri_strstr(const char* pHaystack, const char* pNeedle);
void* mri_memmove(void* pvDest, const void* pvSrc, size_t len);

#endif /* LIBC_H_ */
