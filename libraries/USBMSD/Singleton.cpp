#include "Arduino.h"
#include "PluggableUSBMSD.h"
#include "FlashIAPBlockDevice.h"
#include "FATFileSystem.h"

using namespace arduino;

static FlashIAPBlockDevice bd(0x80000, 0x80000);

void USBMSD::begin()
{
    int err = getFileSystem().mount(&bd);
    if (err) {
        err = getFileSystem().reformat(&bd);
    }
}

mbed::FATFileSystem& USBMSD::getFileSystem()
{
	static mbed::FATFileSystem fs("fs");
	return fs;
}

USBMSD MassStorage(&bd);