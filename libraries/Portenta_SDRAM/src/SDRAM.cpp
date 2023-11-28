#include "SDRAM.h"
extern "C" {
	#include "ram_internal.h"
}

static void MPU_Config() {
    MPU_Region_InitTypeDef MPU_InitStruct;

    /* Disable the MPU */
    HAL_MPU_Disable();

    // Initialize SDRAM Start as shareable
    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress = SDRAM_START_ADDRESS;
    MPU_InitStruct.Size = ARM_MPU_REGION_SIZE_8MB;
    //MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.Number = MPU_REGION_NUMBER5;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    /* Enable the MPU */
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

int SDRAMClass::begin(uint32_t start_address) {

	if (FMC_SDRAM_DEVICE->SDCMR == 0x00000000U) {
		bool ret = sdram_init();
		if (ret == false) {
			return 0;
		}
		/*  Enable MPU for the SDRAM Memory Region to allow non-aligned
			accesses (hard-fault otherwise)
			Initially disable all access for the entire SDRAM memory space,
			then enable access/caching for the size used
		*/

        if (SDRAM_START_ADDRESS == 0x60000000) {
            HAL_SetFMCMemorySwappingConfig(FMC_SWAPBMAP_SDRAM_SRAM);
        }

        #ifdef CORE_CM4
        MPU_Config();
        #endif
	}

	if (start_address) {
		malloc_addblock((void*)start_address, SDRAM_END_ADDRESS - start_address);
	}

	return 1;
}

void* SDRAMClass::malloc(size_t size) {
	return ea_malloc(size);
}

void SDRAMClass::free(void* ptr) {
	ea_free(ptr);
}

bool __attribute__((optimize("O0"))) SDRAMClass::test(bool fast, Stream& _serial) {
    uint8_t const pattern = 0xaa;
    uint8_t const antipattern = 0x55;
    uint8_t *const mem_base = (uint8_t*)SDRAM_START_ADDRESS;

    /* test data bus */
    for (uint8_t i = 1; i; i <<= 1) {
        *mem_base = i;
        if (*mem_base != i) {
            _serial.println("data bus lines test failed! data (" + String(i) + ")");
            __asm__ volatile ("BKPT");
        }
    }

    /* test address bus */
    /* Check individual address lines */
    for (uint32_t i = 1; i < HW_SDRAM_SIZE; i <<= 1) {
        mem_base[i] = pattern;
        if (mem_base[i] != pattern) {
            _serial.println("address bus lines test failed! address ("+ String((uint32_t)&mem_base[i], HEX) + ")");
            __asm__ volatile ("BKPT");
        }
    }

    /* Check for aliasing (overlaping addresses) */
    mem_base[0] = antipattern;
    for (uint32_t i = 1; i < HW_SDRAM_SIZE; i <<= 1) {
        if (mem_base[i] != pattern) {
            _serial.println("address bus overlap! address ("+ String((uint32_t)&mem_base[i], HEX) + ")");
            __asm__ volatile ("BKPT");
        }
    }

    /* test all ram cells */
    if (!fast) {
        for (uint32_t i = 0; i < HW_SDRAM_SIZE; ++i) {
            mem_base[i] = pattern;
            if (mem_base[i] != pattern) {
                _serial.println("address bus test failed! address ("+ String((uint32_t)&mem_base[i], HEX) + ")");
                __asm__ volatile ("BKPT");
            }
        }
    } else {
        memset(mem_base, pattern, HW_SDRAM_SIZE);
    }

    return true;
}

SDRAMClass SDRAM;