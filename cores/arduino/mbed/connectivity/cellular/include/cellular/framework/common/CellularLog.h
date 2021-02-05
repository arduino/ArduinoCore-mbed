/*
 * Copyright (c) 2017, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CELLULAR_LOG_H_
#define CELLULAR_LOG_H_

#if defined(HAVE_DEBUG) && !defined(FEA_TRACE_SUPPORT)
#define FEA_TRACE_SUPPORT
#endif

#include "mbed-trace/mbed_trace.h"
#ifndef TRACE_GROUP
#define TRACE_GROUP  "CELL"
#endif // TRACE_GROUP

/**
 * Set mutex wait/release functions for 'tr_' macros,
 * implementation here is modified from that found from mbed_trace.
 */
namespace mbed_cellular_trace {
void mutex_wait_function_set(void (*mutex_wait_f)(void));
void mutex_release_function_set(void (*mutex_release_f)(void));
void mutex_wait();
void mutex_release();
}

#endif // CELLULAR_LOG_H_
