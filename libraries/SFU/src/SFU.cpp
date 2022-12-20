#include "SFU.h"
#include "FlashIAPBlockDevice.h"
#if __has_include("PluggableUSBMSD.h")
#include "PluggableUSBMSD.h"
#define ADD_USB_MSD
#endif

const unsigned char SFU[0x10000] __attribute__ ((section(".second_stage_ota"), used)) = {
	#include "rp2040.h"
};

FlashIAPBlockDevice bd(XIP_BASE + 0xF00000, 0x100000);

mbed::FATFileSystem& SFU::getFileSystem() {
	static mbed::FATFileSystem fs("ota");
	return fs;
}

int SFU::begin() {
	int err = getFileSystem().mount(&bd);
    if (err) {
        err = getFileSystem().reformat(&bd);
    }
}

int SFU::download(const char* url) {
	// No download at the moment, allow the user to upload a file via mass storage
}

int SFU::apply() {
	// No autoreboot
}

#ifdef ADD_USB_MSD

void USBMSD::begin()
{
	SFU::begin();
}

mbed::FATFileSystem& USBMSD::getFileSystem()
{
	return SFU::getFileSystem();
}

USBMSD MassStorage(&bd);

#endif