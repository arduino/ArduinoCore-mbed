/*
 * Mbed-OS Microcontroller Library
 * Copyright (c) 2020 Embedded Planet
 * Copyright (c) 2020 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License
 */

#include "flash_map_backend/secondary_bd.h"
#include "mcuboot_config/mcuboot_logging.h"
#include "platform/mbed_toolchain.h"
#include "MBRBlockDevice.h"
#include "fileblockdevice/FileBlockDevice.h"
#include "FATFileSystem.h"
#include "bootutil/bootutil_priv.h"

mbed::FATFileSystem* get_filesystem(void) {
    mbed::BlockDevice* raw = mbed::BlockDevice::get_default_instance();
    static mbed::MBRBlockDevice mbr = mbed::MBRBlockDevice(raw, 2);
    static mbed::FATFileSystem fs("fs");
    
    int err = mbr.init();
    if(err) {
        MCUBOOT_LOG_ERR("Cannot initialize Block Device");
        return nullptr;
    }
    
    // Mount can fail if filerystem already mounted
    fs.mount(&mbr);
    return &fs;
}

mbed::BlockDevice* get_secondary_bd(void) {
    mbed::FATFileSystem* fs = get_filesystem();
    if(fs == nullptr) {
        MCUBOOT_LOG_ERR("Cannot initialize secondary fs");
    }
    
    // TODO: This is the default configuration, check RTC registers to allow custom settings
    static mbed::FileBlockDevice secondary_bd = mbed::FileBlockDevice("/fs/update.bin", "rb+", MCUBOOT_SLOT_SIZE, 0x01, 0x01, 0x1000);
    return &secondary_bd;
}

mbed::BlockDevice* get_scratch_bd(void) {
    mbed::FATFileSystem* fs = get_filesystem();
    if(fs == nullptr) {
        MCUBOOT_LOG_ERR("Cannot initialize scratch fs");
    }
    
    // TODO: This is the default configuration, check RTC registers to allow custom settings
    static mbed::FileBlockDevice scratch_bd = mbed::FileBlockDevice("/fs/scratch.bin", "rb+", MCUBOOT_SCRATCH_SIZE, 0x01, 0x01, 0x1000);
    return &scratch_bd;
}
