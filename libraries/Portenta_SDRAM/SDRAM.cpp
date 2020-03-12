#include "SDRAM.h"
extern "C" {
	#include "ram_internal.h"
}

int SDRAMClass::begin(uint32_t start_address) {

	printf("FMC_SDRAM_DEVICE->SDCMR: %x\n", FMC_SDRAM_DEVICE->SDCMR);
	if (FMC_SDRAM_DEVICE->SDCMR == 0x00000000U) {
		printf("initializing external ram\n");
		bool ret = sdram_init();
		if (ret == false) {
			return 0;
		}
		/*  Enable MPU for the SDRAM Memory Region to allow non-aligned
			accesses (hard-fault otherwise)
			Initially disable all access for the entire SDRAM memory space,
			then enable access/caching for the size used
		*/

		mpu_config_start();
		mpu_config_region(MPU_REGION_SDRAM1, SDRAM_START_ADDRESS, MPU_CONFIG_DISABLE(0x00, MPU_REGION_SIZE_512MB));
		mpu_config_region(MPU_REGION_SDRAM2, SDRAM_START_ADDRESS, MPU_CONFIG_SDRAM(SDRAM_MPU_REGION_SIZE));
		mpu_config_end();
	}

	printf("malloc_addblock: allocate %d bytes\n", SDRAM_END_ADDRESS - start_address);
	malloc_addblock((void*)start_address, SDRAM_END_ADDRESS - start_address);

	return 1;
}

void* SDRAMClass::malloc(size_t size) {
	return ea_malloc(size);
}

void SDRAMClass::free(void* ptr) {
	ea_free(ptr);
}

SDRAMClass SDRAM;