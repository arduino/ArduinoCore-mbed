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
#include <core/scatter_gather.h>
#include <core/try_catch.h>


void ScatterGather_Init(ScatterGather* pThis, ScatterGatherEntry* pEntries, uint32_t entryCount)
{
    pThis->pEntries = pEntries;
    pThis->entryCount = entryCount;
}

uint32_t ScatterGather_Count(ScatterGather* pThis)
{
    uint32_t i;
    uint32_t count = 0;
    for (i = 0 ; i < pThis->entryCount ; i++)
    {
        count += pThis->pEntries[i].count;
    }
    return count;
}

uint32_t mriScatterGather_Get(const ScatterGather* pThis, uint32_t index)
{
    uint32_t i;
    uint32_t count = 0;
    uint32_t base = 0;
    for (i = 0 ; i < pThis->entryCount ; i++)
    {
        base = count;
        count += pThis->pEntries[i].count;
        if (index < count)
        {
            return pThis->pEntries[i].pValues[index - base];
        }
    }
    __throw_and_return(bufferOverrunException, 0);
}

void mriScatterGather_Set(ScatterGather* pThis, uint32_t index, uint32_t newValue)
{
    uint32_t i;
    uint32_t count = 0;
    uint32_t base = 0;
    for (i = 0 ; i < pThis->entryCount ; i++)
    {
        base = count;
        count += pThis->pEntries[i].count;
        if (index < count)
        {
            pThis->pEntries[i].pValues[index - base] = newValue;
            return;
        }
    }
    __throw(bufferOverrunException);
}
