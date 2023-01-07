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
#include <stdint.h>
#include <core/libc.h>

void* mri_memcpy(void* pvDest, const void* pvSrc, size_t len)
{
    uint8_t* pDest = (uint8_t*)pvDest;
    const uint8_t* pSrc = (const uint8_t*)pvSrc;

    while (len--)
    {
        *pDest++ = *pSrc++;
    }
    return pvDest;
}



void* mri_memset(void *pvDest, int val, size_t len)
{
    uint8_t* pDest = (uint8_t*)pvDest;
    while (len--)
    {
        *pDest++ = (uint8_t)val;
    }
    return pvDest;
}



int mri_strcmp(const char* pc1, const char* pc2)
{
    uint8_t* p1 = (uint8_t*)pc1;
    uint8_t* p2 = (uint8_t*)pc2;
    int      cmp = 0;

    do
    {
        cmp = *p1 - *p2++;
    } while (cmp == 0 && *p1++);
    return cmp;
}




int mri_strncmp(const char* pc1, const char* pc2, size_t len)
{
    uint8_t* p1 = (uint8_t*)pc1;
    uint8_t* p2 = (uint8_t*)pc2;
    int      cmp = 0;

    if (len == 0)
    {
        return 0;
    }

    do
    {
        cmp = *p1 - *p2++;
    } while (cmp == 0 && --len > 0 && *p1++);
    return cmp;
}



size_t mri_strlen(const char* p)
{
    const char* pStart = p;
    while (*p)
    {
        p++;
    }
    return p - pStart;
}



char* mri_strstr(const char* pHaystack, const char* pNeedle)
{
    size_t len = mri_strlen(pNeedle);

    while (*pHaystack)
    {
        if (mri_strncmp(pHaystack, pNeedle, len) == 0)
        {
            return (char*)pHaystack;
        }
        pHaystack++;
    }
    return NULL;
}



void* mri_memmove(void* pvDest, const void* pvSrc, size_t len)
{
    uint8_t* pDest = (uint8_t*)pvDest;
    uint8_t* pDestEnd = pDest + len;
    uint8_t* pSrc = (uint8_t*)pvSrc;
    uint8_t* pSrcEnd = pSrc + len;
    int dir = 1;
    uint8_t* pDestCurr = pDest;
    uint8_t* pSrcCurr = pSrc;

    if (pDest > pSrc && pDest < pSrcEnd)
    {
        dir = -1;
        pDestCurr = pDestEnd - 1;
        pSrcCurr = pSrcEnd - 1;
    }

    while (len--)
    {
        *pDestCurr = *pSrcCurr;
        pDestCurr += dir;
        pSrcCurr += dir;
    }

    return pvDest;
}
