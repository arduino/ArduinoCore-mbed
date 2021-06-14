#include "SFU.h"
#include "FlashIAPBlockDevice.h"
#include "FATFileSystem.h"
#include "PluggableUSBMSD.h"

const unsigned char SFU[0x10000] __attribute__ ((section(".second_stage_ota"), used)) = {
	#include "rp2040.h"
};

FlashIAPBlockDevice bd(XIP_BASE + 0xF00000, 0x100000);

void USBMSD::begin()
{
    int err = getFileSystem().mount(&bd);
    if (err) {
        err = getFileSystem().reformat(&bd);
    }
}

mbed::FATFileSystem& USBMSD::getFileSystem()
{
	static mbed::FATFileSystem fs("ota");
	return fs;
}

USBMSD MassStorage(&bd);

int SFU::begin() {
	MassStorage.begin();
}

int SFU::download(const char* url) {
	// No download at the moment, allow the user to upload a file via mass storage
}

int SFU::apply() {
	// No autoreboot
}
