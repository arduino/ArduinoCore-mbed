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

/** \addtogroup storage */
/** @{*/

#ifndef MBED_FILE_BLOCK_DEVICE_H
#define MBED_FILE_BLOCK_DEVICE_H

#include "mbed.h"
#include "BlockDevice.h"
#include "platform/mbed_assert.h"
#include <stdio.h>

namespace mbed {

/** BlockDevice for mapping a file into a Blockdevice
 *
 */
class FileBlockDevice : public BlockDevice {
public:
    /** Create FileBlockDevice - An SFDP based Flash Block Device over QSPI bus
     *
     */
    FileBlockDevice(const char *path, const char * flags, bd_size_t bd_size, bd_size_t r_size, bd_size_t w_size, bd_size_t e_size);

    /** Lifetime of a block device
     */
    ~FileBlockDevice() {};

    /** Initialize a block device
     *
     *  @return         QSPIF_BD_ERROR_OK(0) - success
     *                  QSPIF_BD_ERROR_DEVICE_ERROR - device driver transaction failed
     *                  QSPIF_BD_ERROR_READY_FAILED - Waiting for Memory ready failed or timedout
     *                  QSPIF_BD_ERROR_PARSING_FAILED - unexpected format or values in one of the SFDP tables
     */
    virtual int init();

    /** Deinitialize a block device
     *
     *  @return         QSPIF_BD_ERROR_OK(0) - success
     *                  QSPIF_BD_ERROR_DEVICE_ERROR - device driver transaction failed
     */
    virtual int deinit();

    /** Ensure data on storage is in sync with the driver
     *
     *  @return         0 on success or a negative error code on failure
     */
    virtual int sync();

    /** Read blocks from a block device
     *
     *  @param buffer   Buffer to write blocks to
     *  @param addr     Address of block to begin reading from
     *  @param size     Size to read in bytes, must be a multiple of read block size
     *  @return         QSPIF_BD_ERROR_OK(0) - success
     *                  QSPIF_BD_ERROR_DEVICE_ERROR - device driver transaction failed
     */
    virtual int read(void *buffer, bd_addr_t addr, bd_size_t size);

    /** Program blocks to a block device
     *
     *  The blocks must have been erased prior to being programmed
     *
     *  @param buffer   Buffer of data to write to blocks
     *  @param addr     Address of block to begin writing to
     *  @param size     Size to write in bytes, must be a multiple of program block size
     *  @return         QSPIF_BD_ERROR_OK(0) - success
     *                  QSPIF_BD_ERROR_DEVICE_ERROR - device driver transaction failed
     *                  QSPIF_BD_ERROR_READY_FAILED - Waiting for Memory ready failed or timed out
     *                  QSPIF_BD_ERROR_WREN_FAILED - Write Enable failed
     *                  QSPIF_BD_ERROR_PARSING_FAILED - unexpected format or values in one of the SFDP tables
     */
    virtual int program(const void *buffer, bd_addr_t addr, bd_size_t size);

    /** Erase blocks on a block device
     *
     *  The state of an erased block is undefined until it has been programmed
     *
     *  @param addr     Address of block to begin erasing
     *  @param size     Size to erase in bytes, must be a multiple of erase block size
     *  @return         QSPIF_BD_ERROR_OK(0) - success
     *                  QSPIF_BD_ERROR_DEVICE_ERROR - device driver transaction failed
     *                  QSPIF_BD_ERROR_READY_FAILED - Waiting for Memory ready failed or timed out
     *                  QSPIF_BD_ERROR_WREN_FAILED - Write Enable failed
     *                  QSPIF_BD_ERROR_PARSING_FAILED - unexpected format or values in one of the SFDP tables
     *                  QSPIF_BD_ERROR_INVALID_ERASE_PARAMS - Trying to erase unaligned address or size
     */
    virtual int erase(bd_addr_t addr, bd_size_t size);

    /** Get the size of a readable block
     *
     *  @return         Size of a readable block in bytes
     */
    virtual bd_size_t get_read_size() const;

    /** Get the size of a programable block
     *
     *  @return         Size of a program block size in bytes
     *  @note Must be a multiple of the read size
     */
    virtual bd_size_t get_program_size() const;

    /** Get the size of a eraseable block
     *
     *  @return         Size of a minimal erase block, common to all regions, in bytes
     *  @note Must be a multiple of the program size
     */
    virtual bd_size_t get_erase_size() const;

    /** Get the size of minimal eraseable sector size of given address
     *
     *  @param addr     Any address within block queried for erase sector size (can be any address within flash size offset)
     *  @return         Size of minimal erase sector size, in given address region, in bytes
     *  @note Must be a multiple of the program size
     */
    virtual bd_size_t get_erase_size(bd_addr_t addr) const;

    /** Get the value of storage byte after it was erased
     *
     *  If get_erase_value returns a non-negative byte value, the underlying
     *  storage is set to that value when erased, and storage containing
     *  that value can be programmed without another erase.
     *
     *  @return         The value of storage when erased, or -1 if you can't
     *                  rely on the value of erased storage
     */
    virtual int get_erase_value() const;

    /** Get the total size of the underlying device
     *
     *  @return         Size of the underlying device in bytes
     */
    virtual mbed::bd_size_t size() const;

    /** Get the BlockDevice class type.
     *
     *  @return         A string represent the BlockDevice class type.
     */
    virtual const char *get_type() const;

        /** Convenience function for checking block program validity
     *
     *  @param addr     Address of block to begin writing to
     *  @param size     Size to write in bytes
     *  @return         True if program is valid for underlying block device
     */
    virtual bool is_valid_program(bd_addr_t addr, bd_size_t size) const;

    /** Convenience function for checking block read validity
     *
     *  @param addr     Address of block to begin reading from
     *  @param size     Size to read in bytes
     *  @return         True if read is valid for underlying block device
     */
    virtual bool is_valid_read(bd_addr_t addr, bd_size_t size) const;

    /** Convenience function for checking block erase validity
     *
     *  @param addr     Address of block to begin erasing
     *  @param size     Size to erase in bytes
     *  @return         True if erase is valid for underlying block device
     */
    virtual bool is_valid_erase(bd_addr_t addr, bd_size_t size) const;

private:
    FILE *_file[1];
    const char *_path;
    const char *_oflag;
    bd_size_t _bd_size;
    bd_size_t _r_size;
    bd_size_t _w_size;
    bd_size_t _e_size;
    bool _is_initialized;
};

} // namespace mbed

// Added "using" for backwards compatibility
#ifndef MBED_NO_GLOBAL_USING_DIRECTIVE
using mbed::FileBlockDevice;
#endif

#endif

/** @}*/
