/* SD/MMC Block device Library for MBED-OS
 * Copyright 2017 Roy Krikke
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
 * limitations under the License.
 *
 */

#include "SDMMCBlockDevice.h"
#include "mbed_debug.h"

/* Required version: 5.5.0 and above */
#if defined(MBED_MAJOR_VERSION) && MBED_MAJOR_VERSION >= 5
#if (MBED_VERSION < MBED_ENCODE_VERSION(5,5,0))
#error "Incompatible mbed-os version detected! Required 5.5.0 and above"
#endif
#else
#warning "mbed-os version 5.5.0 or above required"
#endif

#define SD_DBG 0 /*!< 1 - Enable debugging */

using namespace mbed;

/** Enum of standard error codes
 *
 *  @enum bd_sd_error
 */
enum bd_sd_error {
    SD_BLOCK_DEVICE_OK = 0,             /*!< no error */
    SD_BLOCK_DEVICE_ERROR = -5000,      /*!< device specific error */
    //SD_BLOCK_DEVICE_ERROR_WOULD_BLOCK = -5001,  /*!< operation would block */
    //SD_BLOCK_DEVICE_ERROR_UNSUPPORTED = -5002,  /*!< unsupported operation */
    SD_BLOCK_DEVICE_ERROR_PARAMETER = -5003,    /*!< invalid parameter */
    SD_BLOCK_DEVICE_ERROR_NO_INIT = -5004,      /*!< uninitialized */
    SD_BLOCK_DEVICE_ERROR_NO_DEVICE = -5005,    /*!< device is missing or not connected */
    //SD_BLOCK_DEVICE_ERROR_WRITE_PROTECTED = -5006,    /*!< write protected */
    //SD_BLOCK_DEVICE_ERROR_UNUSABLE = -5007,           /*!< unusable card */
    //SD_BLOCK_DEVICE_ERROR_NO_RESPONSE = -5008,        /*!< No response from device */
    //SD_BLOCK_DEVICE_ERROR_CRC = -5009,                /*!< CRC error */
    SD_BLOCK_DEVICE_ERROR_ERASE = -5010,        /*!< Erase error */
    SD_BLOCK_DEVICE_ERROR_READ = -5011,         /*!< Read error */
    SD_BLOCK_DEVICE_ERROR_PROGRAM = -5012,      /*!< Program error */
};

#define BLOCK_SIZE_HC 512 /*!< Block size supported for SD card is 512 bytes  */

SDMMCBlockDevice::SDMMCBlockDevice() :
    _read_size (BLOCK_SIZE_HC), _program_size (BLOCK_SIZE_HC),
    _erase_size(BLOCK_SIZE_HC), _block_size (BLOCK_SIZE_HC),
    _capacity_in_blocks (0)
{
    _timeout = 1000;
}

SDMMCBlockDevice::~SDMMCBlockDevice()
{
    if(_is_initialized) {
        deinit ();
    }
}

int SDMMCBlockDevice::init()
{
    lock();
    _sd_state = BSP_SD_Init();

    if(_sd_state != MSD_OK) {
        if(_sd_state == MSD_ERROR_SD_NOT_PRESENT) {
            debug_if(SD_DBG, "SD card is missing or not connected\n");
            unlock ();
            return SD_BLOCK_DEVICE_ERROR_NO_DEVICE;
        } else {
            debug_if(SD_DBG, "SD card initialization failed\n");
            unlock();
            return SD_BLOCK_DEVICE_ERROR_NO_INIT;
        }
    }
    BSP_SD_GetCardInfo(&_current_card_info);

    _card_type = _current_card_info.CardType;
    _read_size = _current_card_info.BlockSize;
    _program_size = _current_card_info.BlockSize;
    _erase_size = _current_card_info.BlockSize;
    _block_size = _current_card_info.BlockSize;
    _capacity_in_blocks = _current_card_info.BlockNbr;

    debug_if(SD_DBG, "Card Type: %i\n", _current_card_info.CardType);
    debug_if(SD_DBG, "Card Version: %i\n", _current_card_info.CardVersion);
    debug_if(SD_DBG, "Class: %i\n", _current_card_info.Class);
    debug_if(SD_DBG, "Relative Card Address: %x\n", _current_card_info.RelCardAdd);
    debug_if(SD_DBG, "Card Capacity in blocks: %i\n", _current_card_info.BlockNbr);
    debug_if(SD_DBG, "One block size in bytes: %i\n", _current_card_info.BlockSize);
    debug_if(SD_DBG, "Card logical Capacity in blocks: %i\n", _current_card_info.LogBlockNbr);
    debug_if(SD_DBG, "Logical block size in bytes: %i\n", _current_card_info.LogBlockSize);
    debug_if(SD_DBG, "Timeout: %i\n", _timeout);

    _is_initialized = true;
    unlock();
    return SD_BLOCK_DEVICE_OK;
}

int SDMMCBlockDevice::deinit()
{
    lock();
    _sd_state = BSP_SD_DeInit ();
    if(_sd_state != MSD_OK) {
        debug_if (SD_DBG, "SD card deinitialization failed\n");
        return SD_BLOCK_DEVICE_ERROR;
    }
    _is_initialized = false;
    unlock();
    return BD_ERROR_OK;
}

int SDMMCBlockDevice::read(void *b, bd_addr_t addr, bd_size_t size)
{
    if(!is_valid_read (addr, size)) {
        return SD_BLOCK_DEVICE_ERROR_PARAMETER;
    }

    lock();
    if(!_is_initialized) {
        unlock();
        return SD_BLOCK_DEVICE_ERROR_NO_INIT;
    }

    uint32_t *buffer = static_cast<uint32_t *> (b);
    int status = SD_BLOCK_DEVICE_OK;

    // Get block address
    uint32_t block_addr = addr / _block_size;
    // Get block count
    uint32_t block_cnt = size / _block_size;

    debug_if(
        SD_DBG,
        "SDMMCBlockDevice::read addr: 0x%x, block_addr: %i size: %lu block count: %i\n",
        addr, block_addr, size, block_cnt);

    if(BSP_SD_ReadBlocks (buffer, block_addr, block_cnt, _timeout) != MSD_OK) {
        status = SD_BLOCK_DEVICE_ERROR_READ;
    }

    // Wait until SD card is ready to use for new operation
    while(BSP_SD_GetCardState() != SD_TRANSFER_OK) {
    }

    unlock ();
    return status;
}

int SDMMCBlockDevice::program(const void *b, bd_addr_t addr, bd_size_t size)
{
    if(!is_valid_program (addr, size)) {
        return SD_BLOCK_DEVICE_ERROR_PARAMETER;
    }

    lock();
    if(!_is_initialized) {
        unlock ();
        return SD_BLOCK_DEVICE_ERROR_NO_INIT;
    }

    uint32_t* buffer =
        const_cast<uint32_t*> (reinterpret_cast<const uint32_t*> (b));
    int status = SD_BLOCK_DEVICE_OK;

    // Get block address
    uint32_t block_addr = addr / _block_size;
    // Get block count
    uint32_t block_cnt = size / _block_size;

    debug_if (
        SD_DBG,
        "SDMMCBlockDevice::program addr: 0x%x, block_addr: %i size: %lu block count: %i\n",
        addr, block_addr, size, block_cnt);

    if(BSP_SD_WriteBlocks (buffer, block_addr, block_cnt, _timeout) != MSD_OK) {
        status = SD_BLOCK_DEVICE_ERROR_PROGRAM;
    }

    // Wait until SD card is ready to use for new operation
    while(BSP_SD_GetCardState() != SD_TRANSFER_OK) {
    }

    unlock();
    return status;
}

int SDMMCBlockDevice::erase(bd_addr_t addr, bd_size_t size)
{
    if (!is_valid_erase(addr, size)) {
        return SD_BLOCK_DEVICE_ERROR_PARAMETER;
    }

    lock();
    if(!_is_initialized) {
        unlock();
        return SD_BLOCK_DEVICE_ERROR_NO_INIT;
    }

    size -= _block_size;

    int status = SD_BLOCK_DEVICE_OK;
    uint32_t start_addr = addr;
    uint32_t end_addr = start_addr + size;

    // Get block count
    uint32_t block_start_addr = start_addr / _block_size;
    // Get block address
    uint32_t block_end_addr = end_addr / _block_size;

    debug_if(
        SD_DBG,
        "SDMMCBlockDevice::erase start_addr: 0x%x, block_start_addr: %i | end_addr: 0x%x, block_end_addr: %i size: %lu\n",
        start_addr, block_start_addr, end_addr, block_end_addr, size);

    if(BSP_SD_Erase (block_start_addr, block_end_addr) != MSD_OK) {
        status = SD_BLOCK_DEVICE_ERROR_ERASE;
    }

    /* Wait until SD card is ready to use for new operation */
    while(BSP_SD_GetCardState() != SD_TRANSFER_OK) {
    }

    unlock();
    return status;
}

bd_size_t SDMMCBlockDevice::get_read_size() const
{
    return _read_size;
}

bd_size_t SDMMCBlockDevice::get_program_size() const
{
    return _program_size;
}

bd_size_t SDMMCBlockDevice::get_erase_size() const
{
    return _erase_size;
}

bd_size_t SDMMCBlockDevice::size() const
{
    return (_block_size * _capacity_in_blocks);
}

const char *SDMMCBlockDevice::get_type() const
{
    return "SDCARD";
}