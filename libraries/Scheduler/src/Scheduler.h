/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include <Arduino.h>
#include "mbed.h"

#define MAX_THREADS_NUMBER	10

extern "C" {
	typedef void (*SchedulerTask)(void);
	typedef void (*SchedulerParametricTask)(void *);
}

// This class exists for only backwards compatibility with arduino-libraries/Scheduler. 
// You are encouraged to use mbed::Thread directly rather than using this.
class SchedulerClass {
public:
	SchedulerClass();
	void startLoop(SchedulerTask task, uint32_t stackSize = OS_STACK_SIZE);
	void start(SchedulerTask task, uint32_t stackSize = OS_STACK_SIZE);
	void start(SchedulerParametricTask task, void *data, uint32_t stackSize = OS_STACK_SIZE);

	void yield() { ::yield(); };
private:
	rtos::Thread* threads[MAX_THREADS_NUMBER] = {NULL};
};

extern SchedulerClass Scheduler;

#endif

