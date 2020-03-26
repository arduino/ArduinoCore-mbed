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
/* Very rough exception handling like macros for C. */
#ifndef MRI_TRY_CATCH_H_
#define MRI_TRY_CATCH_H_

#define noException                         0
#define bufferOverrunException              1
#define invalidHexDigitException            2
#define invalidValueException               3
#define invalidArgumentException            4
#define timeoutException                    5
#define invalidIndexException               6
#define notFoundException                   7
#define exceededHardwareResourcesException  8
#define invalidDecDigitException            9
#define memFaultException                   10
#define mriMaxException                     15

extern int mriExceptionCode;


/* Allow an application including MRI to extend with their own exception codes and replace the below declarations. */
#ifndef MRI_SKIP_TRY_CATCH_MACRO_DEFINES

/* On Linux, it is possible that __try and __catch are already defined. */
#undef __try
#undef __catch

#define __throws

#define __try \
        do \
        { \
            clearExceptionCode();

#define __throwing_func(X) \
            X; \
            if (mriExceptionCode) \
                break;

#define __catch \
        } while (0); \
        if (mriExceptionCode)

#define __throw(EXCEPTION) return ((void)setExceptionCode(EXCEPTION))

#define __throw_and_return(EXCEPTION, RETURN) return (setExceptionCode(EXCEPTION), (RETURN))

#define __rethrow return

#define __rethrow_and_return(RETURN) return RETURN

static inline int getExceptionCode(void)
{
    return mriExceptionCode;
}

static inline void setExceptionCode(int exceptionCode)
{
    mriExceptionCode = exceptionCode > mriExceptionCode ? exceptionCode : mriExceptionCode;
}

static inline void clearExceptionCode(void)
{
    mriExceptionCode = noException;
}

#endif /* MRI_SKIP_TRY_CATCH_MACRO_DEFINES */
#endif /* MRI_TRY_CATCH_H_ */
