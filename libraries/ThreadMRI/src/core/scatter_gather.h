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
/*  'Class' which represents a scatter gather list of registers so that blocks of them can be pulled from various
    locations on the stack and they don't all need to be placed in one contiguous place in memory.
*/
#ifndef SCATTER_GATHER_H_
#define SCATTER_GATHER_H_

#include <stdint.h>

typedef struct
{
    uint32_t* pValues;
    uint32_t  count;
} ScatterGatherEntry;

typedef struct
{
    ScatterGatherEntry* pEntries;
    uint32_t            entryCount;
} ScatterGather;

/* Real name of functions are in mri namespace. */
void     mriScatterGather_Init(ScatterGather* pThis, ScatterGatherEntry* pEntries, uint32_t entryCount);
uint32_t mriScatterGather_Count(ScatterGather* pThis);
uint32_t mriScatterGather_Get(const ScatterGather* pThis, uint32_t index);
void     mriScatterGather_Set(ScatterGather* pThis, uint32_t index, uint32_t newValue);

/* Macroes which allow code to drop the mri namespace prefix. */
#define ScatterGather_Init      mriScatterGather_Init
#define ScatterGather_Count     mriScatterGather_Count
#define ScatterGather_Get       mriScatterGather_Get
#define ScatterGather_Set       mriScatterGather_Set

#endif /* SCATTER_GATHER_H_ */
