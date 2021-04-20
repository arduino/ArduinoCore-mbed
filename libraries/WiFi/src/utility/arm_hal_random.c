#include "mbed_config.h"

#if !defined(NS_USE_EXTERNAL_MBED_TLS)

#include "ns_types.h"
#include "arm_hal_random.h"

#include "mbedtls/entropy_poll.h"

#ifdef MBEDTLS_ENTROPY_HARDWARE_ALT
#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#endif
#endif

void arm_random_module_init(void)
{
}

uint32_t arm_random_seed_get(void)
{
    uint32_t result = 0;
#ifdef MBEDTLS_ENTROPY_HARDWARE_ALT
#if defined(MBEDTLS_PLATFORM_C)
    int ret = mbedtls_platform_setup(NULL);
    if (ret != 0) {
        return result;
    }
#endif /* MBEDTLS_PLATFORM_C */
    /* Grab a seed from a function we provide for mbedtls */
    size_t len;
    mbedtls_hardware_poll(NULL, (uint8_t *) &result, sizeof result, &len);
#if defined(MBEDTLS_PLATFORM_C)
    mbedtls_platform_teardown(NULL);
#endif /* MBEDTLS_PLATFORM_C */
#endif
    return result;
}

#endif