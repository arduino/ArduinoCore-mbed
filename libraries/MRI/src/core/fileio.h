/* Copyright 2012 Adam Green (https://github.com/adamgreen/)

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
/* Constants used by gdb for remote file APIs. */
#ifndef FILEIO_H_
#define FILEIO_H_

#include <stdint.h>

#define GDB_O_RDONLY    0x0
#define GDB_O_WRONLY    0x1
#define GDB_O_RDWR      0x2
#define GDB_O_APPEND    0x8
#define GDB_O_CREAT     0x200
#define GDB_O_TRUNC     0x400
#define GDB_O_EXCL      0x800

#define GDB_S_IFREG     0100000
#define GDB_S_IFDIR     040000
#define GDB_S_IRUSR     0400
#define GDB_S_IWUSR     0200
#define GDB_S_IXUSR     0100
#define GDB_S_IRGRP     040
#define GDB_S_IWGRP     020
#define GDB_S_IXGRP     010
#define GDB_S_IROTH     04
#define GDB_S_IWOTH     02
#define GDB_S_IXOTH     01

#define GDB_SEEK_SET    0
#define GDB_SEEK_CUR    1
#define GDB_SEEK_END    2

typedef struct
{
    uint32_t device;
    uint32_t inode;
    uint32_t mode;
    uint32_t numberOfLinks;
    uint32_t userId;
    uint32_t groupId;
    uint32_t deviceType;
    uint32_t totalSizeUpperWord;
    uint32_t totalSizeLowerWord;
    uint32_t blockSizeUpperWord;
    uint32_t blockSizeLowerWord;
    uint32_t blockCountUpperWord;
    uint32_t blockCountLowerWord;
    uint32_t lastAccessTime;
    uint32_t lastModifiedTime;
    uint32_t lastChangeTime;
} GdbStats;

#endif /* FILEIO_H_ */
