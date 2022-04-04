#if defined(MBEDTLS_ENTROPY_NV_SEED)

#include "mbed.h"

// Provide stubs for mbedtls entropy source
// The functions may be implemented using a crypto chip like ECCx08
extern "C" {
	WEAK int mbedtls_platform_std_nv_seed_read( unsigned char *buf, size_t buf_len ) {
		return -1;
	}

	WEAK int mbedtls_platform_std_nv_seed_write( unsigned char *buf, size_t buf_len ) {
		return -1;
	}
}

#endif