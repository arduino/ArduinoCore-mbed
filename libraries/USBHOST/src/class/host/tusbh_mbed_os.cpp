/*       
 *         _______                    _    _  _____ ____  
 *        |__   __|                  | |  | |/ ____|  _ \ 
 *           | | ___  ___ _ __  _   _| |  | | (___ | |_) |
 *           | |/ _ \/ _ \ '_ \| | | | |  | |\___ \|  _ < 
 *           | |  __/  __/ | | | |_| | |__| |____) | |_) |
 *           |_|\___|\___|_| |_|\__, |\____/|_____/|____/ 
 *                               __/ |                    
 *                              |___/                     
 *
 * TeenyUSB - light weight usb stack for STM32 micro controllers
 * 
 * Copyright (c) 2019 XToolBox  - admin@xtoolbox.org
 *                         www.tusb.org
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

extern "C" {
#include "tusbh.h"
}
#include <stdarg.h>
#include "string.h"
#include <stdlib.h>

#include "Arduino.h"
#include "mbed.h"

#ifdef TUSB_HAS_OS

#define  TUSBH_MSG_Q_LENGTH        16

static rtos::Mail<tusbh_message_t, TUSBH_MSG_Q_LENGTH> mail_box;

tusbh_msg_q_t* tusbh_mq_create()
{
    return NULL;
}

void tusbh_mq_free(tusbh_msg_q_t* mq)
{
}


int tusbh_mq_init(tusbh_msg_q_t* mq)
{
    return 0;
}

int tusbh_mq_post(tusbh_msg_q_t* mq, const tusbh_message_t* msg)
{
    tusbh_message_t *mail = mail_box.alloc();
    memcpy(mail, msg, sizeof(tusbh_message_t));
    mail_box.put(mail);
    return 1;
}

int tusbh_mq_get(tusbh_msg_q_t* mq, tusbh_message_t* msg)
{
    osEvent evt = mail_box.get();
    if (evt.status == osEventMail) {
        tusbh_message_t *mail = (tusbh_message_t *)evt.value.p;
        memcpy(msg, mail, sizeof(tusbh_message_t));
        mail_box.free(mail);
        return 1;
    }
    return 0;
}

#define MAX_DEVICE_COUNT   16
static tusbh_device_t* device_pool[MAX_DEVICE_COUNT] = { NULL };

tusbh_device_t* tusbh_new_device()
{
    for(int i=0;i<MAX_DEVICE_COUNT;i++){
        if(device_pool[i] == NULL){
            device_pool[i] = (tusbh_device_t*)calloc(sizeof(tusbh_device_t), 1);
            return device_pool[i];
        }
    }
    TUSB_OS_INFO("Error: no free device space\n");
    return 0;
}

void tusbh_free_device(tusbh_device_t* device)
{
    for(int i=0;i<MAX_DEVICE_COUNT;i++){
        if(device == device_pool[i] && device != NULL){
            delete device;
            device_pool[i] = NULL;
            return;
        }
    }
    TUSB_OS_INFO("Error: device memory out bound\n");
}

struct _tusbh_evt
{
    rtos::EventFlags* event;
};

#define MAX_EVENTS_COUNT    16
static tusbh_evt_t* event_pool[MAX_EVENTS_COUNT] = { NULL };

tusbh_evt_t* tusbh_evt_create()
{
    for(int i=0;i<MAX_EVENTS_COUNT;i++){
        if(event_pool[i] == NULL){
            event_pool[i] = (tusbh_evt_t*)calloc(sizeof(tusbh_evt_t), 1);
            event_pool[i]->event = new rtos::EventFlags();
            return event_pool[i];
        }
    }
    TUSB_OS_INFO("Error: no free event space\n");
    return 0;
}

void tusbh_evt_free(tusbh_evt_t* evt)
{
    for(int i=0;i<MAX_EVENTS_COUNT;i++){
        if(evt == event_pool[i] && evt != NULL){
            delete evt->event;
            delete evt;
            event_pool[i] = NULL;
            return;
        }
    }
    TUSB_OS_INFO("Error: Event memory out bound\n");
}

int tusbh_evt_init(tusbh_evt_t* evt)
{
    return 0;
}

int tusbh_evt_set(tusbh_evt_t* evt)
{
    evt->event->set(1);
    return 0;
}


int tusbh_evt_clear(tusbh_evt_t* evt)
{
    evt->event->set(0);
    return 0;
}

int tusbh_evt_wait(tusbh_evt_t* evt, uint32_t timeout_ms)
{
    evt->event->wait_any(0xFF, timeout_ms);
}

static int mem_used;
static int mem_max;
void* tusbh_malloc(uint32_t size)
{
    size = (size + 3) & (~3);
    mem_used+=size;
    if(mem_max < mem_used){
        mem_max = mem_used;
    }
    void* r = malloc(size+8);
    TUSB_ASSERT( (r != 0) && (((uint32_t)r) & 3) == 0 );
    uint32_t* p = (uint32_t*)r;
    *p = size;
    *(p + (size/4) + 1) = 0xdeadbeef;
    //TUSB_OS_INFO("Allocate %p %d\n", p, size);
    return (void*)(p+1);
}

void tusbh_free(void* ptr)
{
    TUSB_ASSERT(ptr != 0);
    uint32_t* p = (uint32_t*)ptr;
    p = p - 1;
    uint32_t size = *p;
    mem_used -= size;
    TUSB_ASSERT(*(p+(size/4)+1) == 0xdeadbeef);
    //TUSB_OS_INFO("Free %p %d\n", p, size);
    free(p);
}

void show_memory(void)
{
}

#endif //TUSB_HAS_OS