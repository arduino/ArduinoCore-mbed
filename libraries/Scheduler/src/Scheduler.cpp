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

#include "Scheduler.h"

SchedulerClass::SchedulerClass() {

}

static void loophelper(SchedulerTask task) {
	while (1) {
		task();
	}
}

void SchedulerClass::startLoop(SchedulerTask task, uint32_t stackSize) {
	int i = 0;
	while (threads[i] != NULL && i < MAX_THREADS_NUMBER) {
		i++;
	}
	threads[i] = new rtos::Thread(osPriorityNormal, stackSize);
	threads[i]->start(mbed::callback(loophelper, task));
}

void SchedulerClass::start(SchedulerTask task, uint32_t stackSize) {
	int i = 0;
	while (threads[i] != NULL && i < MAX_THREADS_NUMBER) {
		i++;
	}
	threads[i] = new rtos::Thread(osPriorityNormal, stackSize);
	threads[i]->start(task);
}

void SchedulerClass::start(SchedulerParametricTask task, void *taskData, uint32_t stackSize) {
	int i = 0;
	while (threads[i] != NULL && i < MAX_THREADS_NUMBER) {
		i++;
	}
	threads[i] = new rtos::Thread(osPriorityNormal, stackSize);
	threads[i]->start(mbed::callback(task, taskData));
}

SchedulerClass Scheduler;

