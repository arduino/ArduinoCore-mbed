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
/* Implementation of semihost assembly language routines to be used to redirect standard I/O (stdin/stdout/stderr)
   from the newlib library to the MRI debug monitor. */
#include "newlib_stubs.h"

    .text
    .code 16
    .syntax unified

    .global mriNewlib_SemihostWrite
    .section .text.mriNewlib_SemihostWrite
    .type mriNewlib_SemihostWrite, function
    /* extern "C" int mriNewlib_SemihostWrite(int file, char *ptr, int len);
       Issues semihost write call to PC via gdb.
    */
mriNewlib_SemihostWrite:
    bkpt    MRI_NEWLIB_SEMIHOST_WRITE
    bx      lr


    .global mriNewlib_SemihostRead
    .section .text.mriNewlib_SemihostRead
    .type mriNewlib_SemihostRead, function
    /* extern "C" int mriNewlib_SemihostRead(int file, char *ptr, int len);
       Issues semihost read call to PC via gdb.
    */
mriNewlib_SemihostRead:
    bkpt    MRI_NEWLIB_SEMIHOST_READ
    bx      lr


    .global mriNewLib_SemihostOpen
    .section .text.mriNewLib_SemihostOpen
    .type mriNewLib_SemihostOpen, function
    /* extern "C" int mriNewLib_SemihostOpen(const char *pFilename, int flags, int mode);
       Issues semihost open file call to PC via gdb.
    */
mriNewLib_SemihostOpen:
    bkpt    MRI_NEWLIB_SEMIHOST_OPEN
    bx      lr


    .global mriNewLib_SemihostRename
    .section .text.mriNewLib_SemihostRename
    .type mriNewLib_SemihostRename, function
    /* extern "C" int mriNewLib_SemihostRename(const char *pOldFilename, const char *pNewFilename);
       Issues file rename call to PC via GDB.
    */
mriNewLib_SemihostRename:
    bkpt    MRI_NEWLIB_SEMIHOST_RENAME
    bx      lr

    .global mriNewLib_SemihostUnlink
    .section .text.mriNewLib_SemihostUnlink
    .type mriNewLib_SemihostUnlink, function
    /* extern "C" int mriNewLib_SemihostUnlink(const char *pFilename);
       Issues file delete (unlink) call to PC via GDB.
    */
mriNewLib_SemihostUnlink:
    bkpt    MRI_NEWLIB_SEMIHOST_UNLINK
    bx      lr


    .global mriNewLib_SemihostStat
    .section .text.mriNewLib_SemihostStat
    .type mriNewLib_SemihostStat, function
    /* extern "C" int mriNewLib_SemihostStat(const char *pFilename, struct stat *pStat);
       Issues stat call to PC via GDB.
    */
mriNewLib_SemihostStat:
    bkpt    MRI_NEWLIB_SEMIHOST_STAT
    bx      lr


    .global mriNewlib_SemihostLSeek
    .section .text.mriNewlib_SemihostLSeek
    .type mriNewlib_SemihostLSeek, function
    /* extern "C" int mriNewlib_SemihostLSeek(int file, int offset, int whence);
       Issues seek call to PC via GDB.
    */
mriNewlib_SemihostLSeek:
    bkpt    MRI_NEWLIB_SEMIHOST_LSEEK
    bx      lr


    .global mriNewlib_SemihostClose
    .section .text.mriNewlib_SemihostClose
    .type mriNewlib_SemihostClose, function
    /* extern "C" int mriNewlib_SemihostClose(int file);
       Issues file close call to PC via GDB.
    */
mriNewlib_SemihostClose:
    bkpt    MRI_NEWLIB_SEMIHOST_CLOSE
    bx      lr


    .global mriNewlib_SemihostFStat
    .section .text.mriNewlib_SemihostFStat
    .type mriNewlib_SemihostFStat, function
    /* extern "C" int mriNewlib_SemihostFStat(int file, struct stat *pStat);
       Issues stat call to PC via GDB.
    */
mriNewlib_SemihostFStat:
    bkpt    MRI_NEWLIB_SEMIHOST_FSTAT
    bx      lr


    .global mriNewlib_SemihostGetErrNo
    .section .text.mriNewlib_SemihostGetErrNo
    .type mriNewlib_SemihostGetErrNo, function
    /* extern "C" int mriNewlib_SemihostGetErrNo();
       Retrieves the errno recorded for the last failed newlib semihost call.
    */
mriNewlib_SemihostGetErrNo:
    bkpt    MRI_NEWLIB_SEMIHOST_GET_ERRNO
    bx      lr


    .global mriSetDebuggerHooks
    .section .text.mriSetDebuggerHooks
    .type mriSetDebuggerHooks, function
    /* extern "C" void mriSetDebuggerHooks(MriDebuggerHookPtr pEnteringHook, MriDebuggerHookPtr pLeavingHook, void* pvContext);
       Tells MRI which hook functions to call when entering and leaving the debugger.
    */
mriSetDebuggerHooks:
    bkpt    MRI_NEWLIB_SEMIHOST_SET_HOOKS
    bx      lr
