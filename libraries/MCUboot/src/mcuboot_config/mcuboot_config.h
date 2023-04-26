/*
 * Copyright (c) 2018 Open Source Foundries Limited
 * Copyright (c) 2019-2020 Arm Limited
 * Copyright (c) 2019-2020 Linaro Limited
 * Copyright (c) 2020 Embedded Planet
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MCUBOOT_CONFIG_H__
#define __MCUBOOT_CONFIG_H__

/*
 * Primary slot start address (Internal flash)
 */
#define MCUBOOT_PRIMARY_SLOT_START_ADDR 0x8020000

/*
 * Primary/Secondary slot size
 */
#define MCUBOOT_SLOT_SIZE 0x1E0000

/*
 * Scratch slot start address (QSPI flash FS "virtual" address)
 */
#define MCUBOOT_SCRATCH_START_ADDR 0x9000000

/*
 * Scratch slot size
 */
#define MCUBOOT_SCRATCH_SIZE 0x20000

/*
 * Maximum number of flash sector per slot
 */
#define MCUBOOT_MAX_IMG_SECTORS 0x3C0

/*
 * STM32H7 flash read alignment
 */
#define MCUBOOT_BOOT_MAX_ALIGN 32

/*
 * MCUboot swaps slots using scratch partition
 */
#define MCUBOOT_SWAP_USING_SCRATCH 1

/*
 * LOG level: 0 OFF, 1 ERROR, 2 WARNING, 3 DEBUG, 4 INFO
 */
#define MCUBOOT_LOG_LEVEL 4

/*
 * Signature algorithm
 */
#define MCUBOOT_SIGN_EC256

/*
 * Crypto backend
 */
#define MCUBOOT_USE_MBED_TLS

/*
 * Only one image (two slots) supported for now
 */
#define MCUBOOT_IMAGE_NUMBER 1

/*
 * Currently there is no configuration option, for this platform,
 * that enables the system specific mcumgr commands in mcuboot
 */
#define MCUBOOT_PERUSER_MGMT_GROUP_ENABLED 0

/*
 * Encrypted Images
 */
#if defined(MCUBOOT_ENCRYPT_RSA) || defined(MCUBOOT_ENCRYPT_EC256) || defined(MCUBOOT_ENCRYPT_X25519)
#define MCUBOOT_ENC_IMAGES
#endif

/*
 * Enabling this option uses newer flash map APIs. This saves RAM and
 * avoids deprecated API usage.
 */
#define MCUBOOT_USE_FLASH_AREA_GET_SECTORS

/*
 * No watchdog integration for now
 */
#define MCUBOOT_WATCHDOG_FEED()                 \
    do {                                        \
    } while (0)

/*
 * No direct idle call implemented
 */
#define MCUBOOT_CPU_IDLE() \
    do {                   \
    } while (0)

/*
 * Enable MCUBoot logging
 */
#define MCUBOOT_HAVE_LOGGING

#endif /* __MCUBOOT_CONFIG_H__ */
