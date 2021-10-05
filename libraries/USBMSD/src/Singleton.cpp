#include "Arduino.h"
#include "PluggableUSBMSD.h"
#include "FlashIAPBlockDevice.h"
#include "FATFileSystem.h"

using namespace arduino;

/*
struct disk_info {
    mbed::bd_addr_t addr;
    mbed::bd_size_t size;
    uint8_t data[4096];
    bool erase;
};

static rtos::Thread _t(osPriorityHigh, 32 * 1024, NULL, "msd");
static rtos::Mail<struct disk_info, 16> mail_box;

class FlashIAPBlockDeviceMutexFriendly: public FlashIAPBlockDevice {

public:
	FlashIAPBlockDeviceMutexFriendly(uint32_t address, uint32_t size) : FlashIAPBlockDevice(address, size)
	{
		_base = address;
	}

	void begin() {
		_t.start(mbed::callback(this, &FlashIAPBlockDeviceMutexFriendly::deferred_action));
	}

	int read(void *buffer, mbed::bd_addr_t virtual_address, mbed::bd_size_t size) {
		memcpy(buffer, (void*)(virtual_address + _base), size);
		return 0;
	}

	int erase(mbed::bd_addr_t virtual_address, mbed::bd_size_t size) {
		struct disk_info* info = mail_box.alloc();
		if (info == NULL) {
			return -1;
		}
		info->addr = virtual_address;
		info->size = size;
		info->erase = true;
		mail_box.put(info);
	}

	int program(const void *buffer, mbed::bd_addr_t virtual_address, mbed::bd_size_t size) {
		struct disk_info* info = mail_box.alloc();
		if (info == NULL) {
			return -1;
		}
		info->addr = virtual_address;
		info->size = size;
		info->erase = false;
		memcpy(info->data, buffer, get_erase_size());
		mail_box.put(info);
	}

	void deferred_action() {
		while (true) {
			osEvent evt = mail_box.get();
			if (evt.status == osEventMail) {
				struct disk_info *info = (struct disk_info*)evt.value.p;

				if (info->erase == true) {
					FlashIAPBlockDevice::erase(info->addr, info->size);
				} else {
					FlashIAPBlockDevice::program(info->data, info->addr, info->size);
				}
				mail_box.free(info);
			}
		}
	}

private:
	uint32_t _base;
};
*/

//static FlashIAPBlockDeviceMutexFriendly bd(0x80000, 0x80000);
#ifdef NRF52840_XXAA
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

#endif