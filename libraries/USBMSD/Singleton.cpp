#include "Arduino.h"
#include "PluggableUSBMSD.h"
#include "FlashIAPBlockDevice.h"
#include "FATFileSystem.h"

using namespace arduino;

static void process_msd(USBMSD* obj) {
    while (1) {
        obj->process();
        rtos::ThisThread::sleep_for(1);
    }
}

static mbed::FATFileSystem fs("fs");
static FlashIAPBlockDevice bd(0x80000, 0x80000);
static rtos::Thread th;

void USBMSD::begin()
{
    int err = fs.mount(&bd);
    if (err) {
        err = fs.reformat(&bd);
    }

    th.start(mbed::callback(process_msd, this));
}

USBMSD MassStorage(&bd);