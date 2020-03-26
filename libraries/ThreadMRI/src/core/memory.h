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
/* Routines to read/write memory and detect any faults that might occur while attempting to do so. */
#ifndef MEMORY_H_
#define MEMORY_H_

#include <stdint.h>
#include <core/buffer.h>

/* Real name of functions are in mri namespace. */
uint32_t mriMem_ReadMemoryIntoHexBuffer(Buffer* pBuffer, const void* pvMemory, uint32_t readByteCount);
int      mriMem_WriteHexBufferToMemory(Buffer* pBuffer, void* pvMemory, uint32_t writeByteCount);
int      mriMem_WriteBinaryBufferToMemory(Buffer* pBuffer, void* pvMemory, uint32_t writeByteCount);

/* Macroes which allow code to drop the mri namespace prefix. */
#define ReadMemoryIntoHexBuffer     mriMem_ReadMemoryIntoHexBuffer
#define WriteHexBufferToMemory      mriMem_WriteHexBufferToMemory
#define WriteBinaryBufferToMemory   mriMem_WriteBinaryBufferToMemory

#endif /* MEMORY_H_ */
