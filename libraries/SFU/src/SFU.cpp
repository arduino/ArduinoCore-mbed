#include "SFU.h"
#include "FlashIAPBlockDevice.h"
#include "FATFileSystem.h"
//#include "PluggableUSBMSD.h"

const unsigned char SFU[0x10000] __attribute__ ((section(".second_stage_ota"), used)) = {
	#include "rp2040.h"
};

FlashIAPBlockDevice bd(XIP_BASE + 0x100000, 0x100000);
static mbed::FATFileSystem fs("ota");

int SFU::begin() {
	int err = fs.mount(&bd);
	if (err) {
        err = fs.reformat(&bd);
    }
}

int SFU::download(const char* url) {
	// No download at the moment, allow the user to upload a file via mass storage
}

int SFU::apply() {
	// No autoreboot
}
