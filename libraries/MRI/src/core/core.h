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
/* Core mri functionality exposed to other modules within the debug monitor.  These are the private routines exposed
   from within mri.c.  The public functionality is exposed via mri.h. */
#ifndef CORE_H_
#define CORE_H_

#include <stdint.h>
#include <core/buffer.h>
#include <core/context.h>


typedef struct
{
    uint32_t start;
    uint32_t end;
} AddressRange;


/* Real name of functions are in mri namespace. */
void    mriDebugException(MriContext* pContext);

void    mriCore_InitBuffer(void);
Buffer* mriCore_GetBuffer(void);
Buffer* mriCore_GetInitializedBuffer(void);
void    mriCore_PrepareStringResponse(const char* pErrorString);
#define PrepareEmptyResponseForUnknownCommand() mriCore_PrepareStringResponse("")

int     mriCore_WasControlCFlagSentFromGdb(void);
void    mriCore_RecordControlCFlagSentFromGdb(int controlCFlag);
int     mriCore_WasSemihostCallCancelledByGdb(void);
void    mriCore_FlagSemihostCallAsHandled(void);
int     mriCore_IsFirstException(void);
int     mriCore_WasSuccessfullyInit(void);
void    mriCore_RequestResetOnNextContinue(void);
void    mriCore_SetSingleSteppingRange(const AddressRange* pRange);

MriContext* mriCore_GetContext(void);
void        mriCore_SetContext(MriContext* pContext);

void    mriCore_SetSignalValue(uint8_t signalValue);
uint8_t mriCore_GetSignalValue(void);
void    mriCore_SetSemihostReturnValues(int semihostReturnCode, int semihostErrNo);
int     mriCore_GetSemihostReturnCode(void);
int     mriCore_GetSemihostErrno(void);

void    mriCore_SendPacketToGdb(void);
void    mriCore_GdbCommandHandlingLoop(void);

typedef int (*TempBreakpointCallbackPtr)(void*);
int     mriCore_SetTempBreakpoint(uint32_t breakpointAddress, TempBreakpointCallbackPtr pCallback, void* pvContext);


/* Macroes which allow code to drop the mri namespace prefix. */
#define InitBuffer                      mriCore_InitBuffer
#define GetBuffer                       mriCore_GetBuffer
#define GetInitializedBuffer            mriCore_GetInitializedBuffer
#define PrepareStringResponse           mriCore_PrepareStringResponse
#define WasControlCFlagSentFromGdb      mriCore_WasControlCFlagSentFromGdb
#define RecordControlCFlagSentFromGdb   mriCore_RecordControlCFlagSentFromGdb
#define WasSemihostCallCancelledByGdb   mriCore_WasSemihostCallCancelledByGdb
#define FlagSemihostCallAsHandled       mriCore_FlagSemihostCallAsHandled
#define IsFirstException                mriCore_IsFirstException
#define WasSuccessfullyInit             mriCore_WasSuccessfullyInit
#define RequestResetOnNextContinue      mriCore_RequestResetOnNextContinue
#define SetSingleSteppingRange          mriCore_SetSingleSteppingRange
#define GetContext                      mriCore_GetContext
#define SetContext                      mriCore_SetContext
#define SetSignalValue                  mriCore_SetSignalValue
#define GetSignalValue                  mriCore_GetSignalValue
#define SetSemihostReturnValues         mriCore_SetSemihostReturnValues
#define GetSemihostReturnCode           mriCore_GetSemihostReturnCode
#define GetSemihostErrno                mriCore_GetSemihostErrno
#define SendPacketToGdb                 mriCore_SendPacketToGdb
#define GdbCommandHandlingLoop          mriCore_GdbCommandHandlingLoop
#define SetTempBreakpoint               mriCore_SetTempBreakpoint

/* Macro to convert 32-bit addresses sent from GDB to pointer. */
#if _LP64
    /* When unit testing on 64-bit, address will be from stack so grab upper 32-bit from stack address. */
    /* NOTE: This is for unit testing only.  It would never work on real 64-bit systems. */
    #define ADDR32_TO_POINTER(X) (void*)((size_t)(X) | ((size_t)(&pBuffer) & 0xFFFFFFFF00000000ULL))
#else
    #define ADDR32_TO_POINTER(X) (void*)(X)
#endif /* _LP64 */

#endif /* CORE_H_ */
