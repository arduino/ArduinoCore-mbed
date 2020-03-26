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
/* Handling and issuing routines for gdb file commands. */
#ifndef CMD_FILE_H_
#define CMD_FILE_H_

#include <stdint.h>
#include <core/buffer.h>

typedef struct
{
    uint32_t        filenameAddress;
    uint32_t        filenameLength;
    uint32_t        flags;
    uint32_t        mode;
} OpenParameters;

typedef struct
{
    uint32_t        fileDescriptor;
    uint32_t        bufferAddress;
    int32_t         bufferSize;
} TransferParameters;

typedef struct
{
    uint32_t        fileDescriptor;
    int32_t         offset;
    int32_t         whence;
} SeekParameters;

typedef struct
{
    uint32_t        filenameAddress;
    uint32_t        filenameLength;
} RemoveParameters;

typedef struct
{
    uint32_t        filenameAddress;
    uint32_t        filenameLength;
    uint32_t        fileStatBuffer;
} StatParameters;

typedef struct
{
    uint32_t        origFilenameAddress;
    uint32_t        origFilenameLength;
    uint32_t        newFilenameAddress;
    uint32_t        newFilenameLength;
} RenameParameters;

/* Real name of functions are in mri namespace. */
int      mriIssueGdbFileOpenRequest(const OpenParameters* pParameters);
int      mriIssueGdbFileWriteRequest(const TransferParameters* pParameters);
int      mriIssueGdbFileReadRequest(const TransferParameters* pParameters);
int      mriIssueGdbFileCloseRequest(uint32_t fileDescriptor);
int      mriIssueGdbFileSeekRequest(const SeekParameters* pParameters);
int      mriIssueGdbFileFStatRequest(uint32_t fileDescriptor, uint32_t fileStatBuffer);
int      mriIssueGdbFileUnlinkRequest(const RemoveParameters* pParameters);
int      mriIssueGdbFileStatRequest(const StatParameters* pParameters);
int      mriIssueGdbFileRenameRequest(const RenameParameters* pParameters);
uint32_t mriHandleFileIOCommand(void);

/* Macroes which allow code to drop the mri namespace prefix. */
#define IssueGdbFileOpenRequest     mriIssueGdbFileOpenRequest
#define IssueGdbFileWriteRequest    mriIssueGdbFileWriteRequest
#define IssueGdbFileReadRequest     mriIssueGdbFileReadRequest
#define IssueGdbFileCloseRequest    mriIssueGdbFileCloseRequest
#define IssueGdbFileSeekRequest     mriIssueGdbFileSeekRequest
#define IssueGdbFileFStatRequest    mriIssueGdbFileFStatRequest
#define IssueGdbFileUnlinkRequest   mriIssueGdbFileUnlinkRequest
#define IssueGdbFileStatRequest     mriIssueGdbFileStatRequest
#define IssueGdbFileRenameRequest   mriIssueGdbFileRenameRequest
#define HandleFileIOCommand         mriHandleFileIOCommand

#endif /* CMD_FILE_H_ */
