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

        if (SDRAM_START_ADDRESS != 0xC0000000) {
		    printf("remap ram to 0x60000000\n");
            HAL_SetFMCMemorySwappingConfig(FMC_SWAPBMAP_SDRAM_SRAM);
        }

#if 0

		printf("setup mpu\n");
        #define MPU_SDRAM_EXEC_REGION_NUMBER  MPU_REGION_SDRAM1
        #define MPU_SDRAM_REGION_TEX          (0x4 << MPU_RASR_TEX_Pos) /* Cached memory */
        #define MPU_SDRAM_EXEC_REGION_SIZE    (22 << MPU_RASR_SIZE_Pos)  /* 2^(22+1) = 8Mo */
        #define MPU_SDRAM_ACCESS_PERMSSION    (0x03UL << MPU_RASR_AP_Pos)
        #define MPU_SDRAM_REGION_CACHABLE     (0x01UL << MPU_RASR_C_Pos)
        #define MPU_SDRAM_REGION_BUFFERABLE   (0x01UL << MPU_RASR_B_Pos)

        MPU->CTRL &= ~MPU_CTRL_ENABLE_Msk;
	    /* Configure SDARM region as first region */
	    MPU->RNR  = MPU_SDRAM_EXEC_REGION_NUMBER;
	    /* Set MPU SDARM base address (0xD0000000) */
        MPU->RBAR = SDRAM_START_ADDRESS;
        /*
            - Execute region: RASR[size] = 22  -> 2^(22+1) -> size 8MB
            - Access permission:  Full access: RASR[AP] = 0b011
            - Cached memory:  RASR[TEX] = 0b0100
            - Disable the Execute Never option: to allow the code execution on SDRAM: RASR[XN] = 0
                - Enable the region MPU: RASR[EN] = 1
        */
        MPU->RASR = (MPU_SDRAM_EXEC_REGION_SIZE | MPU_SDRAM_ACCESS_PERMSSION | MPU_SDRAM_REGION_TEX | \
	             MPU_RASR_ENABLE_Msk | MPU_SDRAM_REGION_BUFFERABLE) & ~MPU_RASR_XN_Msk  ;

        /* Enable MPU and leave the predefined regions to default configuration */
        MPU->CTRL |= MPU_CTRL_PRIVDEFENA_Msk |  MPU_CTRL_ENABLE_Msk;
#endif

#if 0
		mpu_config_start();
		mpu_config_region(MPU_REGION_SDRAM1, SDRAM_START_ADDRESS, MPU_CONFIG_DISABLE(0x00, MPU_REGION_SIZE_512MB));
		mpu_config_region(MPU_REGION_SDRAM2, SDRAM_START_ADDRESS, MPU_CONFIG_SDRAM(SDRAM_MPU_REGION_SIZE));
		mpu_config_end();
#endif

	}

	if (start_address) {
		printf("malloc_addblock: allocate %d bytes\n", SDRAM_END_ADDRESS - start_address);
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

bool __attribute__((optimize("O0"))) SDRAMClass::test(bool fast) {
    uint8_t const pattern = 0xaa;
    uint8_t const antipattern = 0x55;
    uint8_t *const mem_base = (uint8_t*)SDRAM_START_ADDRESS;

    /* test data bus */
    for (uint8_t i = 1; i; i <<= 1) {
        *mem_base = i;
        if (*mem_base != i) {
            printf("data bus lines test failed! data (%d)\n", i);
            __asm__ volatile ("BKPT");
        }
    }

    /* test address bus */
    /* Check individual address lines */
    for (uint32_t i = 1; i < HW_SDRAM_SIZE; i <<= 1) {
        mem_base[i] = pattern;
        if (mem_base[i] != pattern) {
            printf("address bus lines test failed! address (%p)\n", &mem_base[i]);
            __asm__ volatile ("BKPT");
        }
    }

    /* Check for aliasing (overlaping addresses) */
    mem_base[0] = antipattern;
    for (uint32_t i = 1; i < HW_SDRAM_SIZE; i <<= 1) {
        if (mem_base[i] != pattern) {
            printf("address bus overlap %p\n", &mem_base[i]);
            __asm__ volatile ("BKPT");
        }
    }

    /* test all ram cells */
    if (!fast) {
        for (uint32_t i = 0; i < HW_SDRAM_SIZE; ++i) {
            mem_base[i] = pattern;
            if (mem_base[i] != pattern) {
                printf("address bus test failed! address (%p)\n", &mem_base[i]);
                __asm__ volatile ("BKPT");
            }
        }
    } else {
        memset(mem_base, pattern, HW_SDRAM_SIZE);
    }

    return true;
}

SDRAMClass SDRAM;