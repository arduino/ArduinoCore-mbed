/*
  Copyright (c) 2022 Arduino SA.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "FileBlockDevice.h"
#include "platform/mbed_assert.h"
#include "mcuboot_config/mcuboot_logging.h"
#include "stddef.h"
#include <stdio.h>

#ifndef MBED_CONF_MBED_TRACE_ENABLE
#define MBED_CONF_MBED_TRACE_ENABLE        0
#endif

#include "mbed_trace.h"
#define TRACE_GROUP "FILEBD"

namespace mbed {

FileBlockDevice::FileBlockDevice(const char *path, const char *flags, bd_size_t bd_size, bd_size_t r_size, bd_size_t w_size, bd_size_t e_size)
    :_path(path), _oflag(flags), _bd_size(bd_size), _r_size(r_size), _w_size(w_size), _e_size(e_size)
{

}

int FileBlockDevice::init()
{
    tr_debug("Opening file block device %s", _path);

    _file[0] = fopen(_path, _oflag);
    if (_file[0] == NULL) {
        MCUBOOT_LOG_ERR("Cannot open file block device %s %s", _path, _oflag);
        return BD_ERROR_DEVICE_ERROR;
    }

    int err = fseek (_file[0] , 0 , SEEK_END);
    if (err) {
        MCUBOOT_LOG_ERR("Init:fseek");
        return BD_ERROR_DEVICE_ERROR;
    }

    uint32_t f_size = ftell (_file[0]);
    if (f_size != _bd_size) {
        MCUBOOT_LOG_WRN("File %s size (0x%zx) should be the same of underlying block device (0x%zx)", _path, f_size, _bd_size);
    }

    tr_debug("Opened file block device handle %d file size 0x%zx", _file[0], f_size);

    _is_initialized = true;

    return BD_ERROR_OK;
}

int FileBlockDevice::deinit()
{
    tr_debug("Closing file block handle %d", _file[0]);
    fclose(_file[0]);
    return BD_ERROR_OK;
}

int FileBlockDevice::sync()
{
    return BD_ERROR_OK;
}

int FileBlockDevice::read(void *buffer, bd_addr_t addr, bd_size_t size)
{
    if (_file[0] == NULL) {
        MCUBOOT_LOG_ERR("Read:invalid handle");
        return BD_ERROR_DEVICE_ERROR;
    }

    tr_debug("reading addr 0x%x", (long)addr);
    tr_debug("reading size %d", (int)size);
    int err = fseek(_file[0], addr, SEEK_SET);
    if (err) {
        MCUBOOT_LOG_ERR("Read:fseek");
        return BD_ERROR_DEVICE_ERROR;
    }
    size_t count = fread(buffer, 1u,  size, _file[0]);
    tr_debug("reading count %d", (int)count);
    if (count != size) {
        MCUBOOT_LOG_ERR("Read:handle %d count 0x%zx", _file[0], count);
        return BD_ERROR_DEVICE_ERROR;
    }

    return BD_ERROR_OK;
}

int FileBlockDevice::program(const void *buffer, bd_addr_t addr, bd_size_t size)
{
    if (_file[0] == NULL) {
        MCUBOOT_LOG_ERR("Program:invalid handle");
        return BD_ERROR_DEVICE_ERROR;
    }

    tr_debug("program addr 0x%x", (long)addr);
    tr_debug("program size %d", (int)size);
    int err = fseek(_file[0], addr, SEEK_SET);
    if (err) {
        MCUBOOT_LOG_ERR("Program:fseek");
        return BD_ERROR_DEVICE_ERROR;
    }

    int count = fwrite(buffer, size, 1u, _file[0]);
    if (count != 1u) {
        MCUBOOT_LOG_ERR("Program:handle %d count 0x%zx", _file[0], count);
        return BD_ERROR_DEVICE_ERROR;
    }

    err = fflush(_file[0]);
    if (err != 0u) {
        MCUBOOT_LOG_ERR("Program:flush");
        return BD_ERROR_DEVICE_ERROR;
    }

    return BD_ERROR_OK;
}

int FileBlockDevice::erase(bd_addr_t addr, bd_size_t size)
{
    if (_file[0] == NULL) {
        MCUBOOT_LOG_ERR("Erase:invalid handle");
        return BD_ERROR_DEVICE_ERROR;
    }

    tr_debug("program addr 0x%x", (long)addr);
    tr_debug("program size %d", (int)size);
    bd_size_t len;
    for (len = 0; len < size; len++) {
        int err = fseek(_file[0], addr + len, SEEK_SET);
        if (err) {
            MCUBOOT_LOG_ERR("Erase:fseek");
            return BD_ERROR_DEVICE_ERROR;
        }
        uint8_t erase_val = 0xFF;
        int count = fwrite(&erase_val, 1u, 1u, _file[0]);
        if (count != 1u) {
            MCUBOOT_LOG_ERR("Erase:handle %d count 0x%zx", _file[0], count);
            return BD_ERROR_DEVICE_ERROR;
        }
    }

    int err = fflush(_file[0]);
    if (err != 0u) {
        MCUBOOT_LOG_ERR("Erase:flush");
        return BD_ERROR_DEVICE_ERROR;
    }

    return BD_ERROR_OK;
}

bool FileBlockDevice::is_valid_read(bd_addr_t addr, bd_size_t size) const
{
    return ((addr + size)  <= _bd_size);
}

bool FileBlockDevice::is_valid_program(bd_addr_t addr, bd_size_t size) const
{
    return ((addr + size)  <= _bd_size);
}

bool FileBlockDevice::is_valid_erase(bd_addr_t addr, bd_size_t size) const
{
    return ((addr + size)  <= _bd_size);
}

bd_size_t FileBlockDevice::get_read_size() const
{
    // Return minimum read size in bytes for the device
    return _r_size;
}

bd_size_t FileBlockDevice::get_program_size() const
{
    // Return minimum program/write size in bytes for the device
    return _w_size;
}

bd_size_t FileBlockDevice::get_erase_size() const
{
    // return minimal erase size supported by all regions (0 if none exists)
    return _e_size;
}

bd_size_t FileBlockDevice::get_erase_size(bd_addr_t addr) const
{
    return _e_size;
}

int FileBlockDevice::get_erase_value() const
{
    return 0xFF;
}

bd_size_t FileBlockDevice::size() const
{
    return _bd_size;
}

const char *FileBlockDevice::get_type() const
{
    return "FILEBD";
}

} // namespace mbed
