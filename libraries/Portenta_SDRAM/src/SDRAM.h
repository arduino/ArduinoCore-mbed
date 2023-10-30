#include "ea_malloc.h"

#ifdef __cplusplus

#include "Arduino.h"

#if !defined(CORE_CM4)
#define SDRAM_END_ADDRESS			(0x60800000)
#define SDRAM_START_ADDRESS			(0x60000000)
#else
#define SDRAM_END_ADDRESS			(0xC0800000)
#define SDRAM_START_ADDRESS			(0xC0000000)
#endif

class SDRAMClass {
public:
	SDRAMClass() {}
	int begin(uint32_t start_address = SDRAM_START_ADDRESS);
	void* malloc(size_t size);
	void free(void* ptr);
	bool test(bool fast = false);
private:
	void mpu_config_start(void) {
		__disable_irq();
	}

	void mpu_config_region(uint32_t region, uint32_t base_addr, uint32_t attr_size) {
		MPU->RNR = region;
		MPU->RBAR = base_addr;
		MPU->RASR = attr_size;
	}

	void mpu_config_end(void) {
		__ISB();
		__DSB();
		__DMB();
		__enable_irq();
	}
};

extern SDRAMClass SDRAM;

#endif