/*
 * Copyright (c) 2022 Syntiant Corp.  All rights reserved.
 * Contact at http://www.syntiant.com
 *
 * This software is available to you under a choice of one of two licenses.
 * You may choose to be licensed under the terms of the GNU General Public
 * License (GPL) Version 2, available from the file LICENSE in the main
 * directory of this source tree, or the OpenIB.org BSD license below.  Any
 * code involving Linux software will require selection of the GNU General
 * Public License (GPL) Version 2.
 *
 * OPENIB.ORG BSD LICENSE
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <stdint.h>

#include <syntiant_ndp120_tiny.h>
#include <syntiant_spictl_regs.h>
#include <syntiant_tiny_cspi.h>

#define SYNTIANT_NDP120_SPI_OP 0
#define SYNTIANT_NDP120_MCU_OP 1

#define MSPI_CLK_DIV 6
#define MSSB_OE_USED 7

static int _cspi_read(struct syntiant_ndp120_tiny_device_s *ndp, int ssb, int num_bytes, uint8_t *data, int end_packet);
static int _cspi_write(struct syntiant_ndp120_tiny_device_s *ndp, int ssb, int num_bytes, uint8_t *data, int end_packet);

/* Wait for SPI operation to complete */
static int cspi_poll_done(struct syntiant_ndp120_tiny_device_s *ndp)
{
    uint8_t done;
    int s;
    uint32_t spictl;

    s = syntiant_ndp120_tiny_read_block(
        ndp, SYNTIANT_NDP120_MCU_OP, NDP120_CHIP_CONFIG_SPICTL, &spictl, 4);
    if (s) goto error;

    done = NDP120_CHIP_CONFIG_SPICTL_DONE_EXTRACT(spictl);
    while (done == 0x0) {
        s = syntiant_ndp120_tiny_read_block(
            ndp, SYNTIANT_NDP120_MCU_OP, NDP120_CHIP_CONFIG_SPICTL, &spictl, 4);
        if (s) goto error;
        done = NDP120_CHIP_CONFIG_SPICTL_DONE_EXTRACT(spictl);
    }
error:
    return 0;
}

/* end a SPI packet */
static int cspi_end(struct syntiant_ndp120_tiny_device_s *ndp)
{
    uint32_t tmp_data;
    int s;

    s = syntiant_ndp120_tiny_read_block(
        ndp, SYNTIANT_NDP120_MCU_OP, NDP120_CHIP_CONFIG_SPICTL, &tmp_data, 4);
    if (s) goto error;

    tmp_data = NDP120_CHIP_CONFIG_SPICTL_MODE_MASK_INSERT(tmp_data, NDP120_CHIP_CONFIG_SPICTL_MODE_STANDBY);
    s = syntiant_ndp120_tiny_write_block(
        ndp, SYNTIANT_NDP120_MCU_OP, NDP120_CHIP_CONFIG_SPICTL, &tmp_data, 4);
    if (s) goto error;

error:
    return s;
}

int syntiant_cspi_write(struct syntiant_ndp120_tiny_device_s *ndp, int ssb, int num_bytes, uint8_t *data, int end_packet)
{
    int s;
    int l;

    /* split up write in chunks of max 16 bytes */
    while (num_bytes > 0) {
        l = (num_bytes > 16) ? 16 : num_bytes;
        num_bytes -= l;
        s = _cspi_write(ndp, ssb, l, data, num_bytes?0:end_packet);
        if (s) goto error;
        data += l;
    }
error:
    return s;
}


/*
 * Write up to 16 bytes of data
 */
int _cspi_write(struct syntiant_ndp120_tiny_device_s *ndp, int ssb, int num_bytes, uint8_t *data, int end_packet)
{
    uint32_t tmp_data;
    int s;
    int len = (num_bytes + 3) & ~0x3;

    if (len > 16 || len <= 0) {
        s = SYNTIANT_NDP120_ERROR_ARG;
        goto error;
    }

    /* Turn on SSB and get ready */
    s = syntiant_ndp120_tiny_read_block(
        ndp, SYNTIANT_NDP120_MCU_OP, NDP120_CHIP_CONFIG_SPICTL, &tmp_data, 4);
    if (s) goto error;

    tmp_data = NDP120_CHIP_CONFIG_SPICTL_MODE_MASK_INSERT(tmp_data, NDP120_CHIP_CONFIG_SPICTL_MODE_SS);
    tmp_data = NDP120_CHIP_CONFIG_SPICTL_MSPI_READ_MASK_INSERT(tmp_data, 0x0);
    tmp_data = NDP120_CHIP_CONFIG_SPICTL_MSSB_OE_MASK_INSERT(tmp_data, MSSB_OE_USED);
    tmp_data = NDP120_CHIP_CONFIG_SPICTL_MSSB_MASK_INSERT(tmp_data, (uint32_t)ssb);
    s = syntiant_ndp120_tiny_write_block(
        ndp, SYNTIANT_NDP120_MCU_OP, NDP120_CHIP_CONFIG_SPICTL, &tmp_data, 4);
    if (s) goto error;

    /* copy data */
    s = syntiant_ndp120_tiny_write_block(
        ndp, SYNTIANT_NDP120_MCU_OP, NDP120_CHIP_CONFIG_SPITX(0), data, (unsigned int)len);
    if (s) goto error;

    s = syntiant_ndp120_tiny_read_block(
        ndp, SYNTIANT_NDP120_MCU_OP, NDP120_CHIP_CONFIG_SPICTL, &tmp_data, 4);
    if (s) goto error;
    tmp_data = NDP120_CHIP_CONFIG_SPICTL_MODE_MASK_INSERT(tmp_data, NDP120_CHIP_CONFIG_SPICTL_MODE_TRANSFER);
    tmp_data = NDP120_CHIP_CONFIG_SPICTL_NUMBYTES_MASK_INSERT(tmp_data, (uint32_t)num_bytes-1);
    s = syntiant_ndp120_tiny_write_block(
        ndp, SYNTIANT_NDP120_MCU_OP, NDP120_CHIP_CONFIG_SPICTL, &tmp_data, 4);
    if (s) goto error;

    /* Wait for MSPI Done */
    s = cspi_poll_done(ndp);
    if (s) goto error;

    /* Wait for next packet or end sequence */
    s = syntiant_ndp120_tiny_read_block(
        ndp, SYNTIANT_NDP120_MCU_OP, NDP120_CHIP_CONFIG_SPICTL, &tmp_data, 4);
    if (s) goto error;
    if (end_packet == 0) {
        tmp_data = NDP120_CHIP_CONFIG_SPICTL_MODE_MASK_INSERT(tmp_data, NDP120_CHIP_CONFIG_SPICTL_MODE_UPDATE);
    } else {
        tmp_data = NDP120_CHIP_CONFIG_SPICTL_MODE_MASK_INSERT(tmp_data, NDP120_CHIP_CONFIG_SPICTL_MODE_SS);
    }
    s = syntiant_ndp120_tiny_write_block(
        ndp, SYNTIANT_NDP120_MCU_OP, NDP120_CHIP_CONFIG_SPICTL, &tmp_data, 4);
    if (s) goto error;

    if (end_packet != 0) {
        s = cspi_end(ndp);
        if (s) goto error;
    }

error:
    return s;
}

int syntiant_cspi_read(struct syntiant_ndp120_tiny_device_s *ndp, int ssb, int num_bytes, uint8_t* data, int end_packet)
{
    int s;
    int l;

    /* split up read in chunks of 16 bytes */
    while (num_bytes > 0) {
        l = (num_bytes > 16) ? 16 : num_bytes;
        num_bytes -= l;
        s = _cspi_read(ndp, ssb, l, data, num_bytes?0:end_packet);
        if (s) goto error;
        data += l;
    }

error:
    return s;
}

/*
 * Read up to 16 bytes of data
 * Generally, the function assumes that write_cspi is already called with end_packet=False
 */
static int _cspi_read(struct syntiant_ndp120_tiny_device_s *ndp, int ssb, int num_bytes, uint8_t *data, int end_packet)
{
    uint32_t tmp_data;
    int bytes_to_transfer;
    int len;
    int s;

    if (num_bytes < 0) {
        s = SYNTIANT_NDP120_ERROR_ARG;
        goto error;
    }

    if (num_bytes < 16) {
        bytes_to_transfer = num_bytes;
    } else{
        bytes_to_transfer = 16;
    }
    len = (num_bytes + 3) & ~0x3;

    s = syntiant_ndp120_tiny_read_block(
            ndp, SYNTIANT_NDP120_MCU_OP, NDP120_CHIP_CONFIG_SPICTL, &tmp_data, 4);
    if (s) goto error;
    tmp_data = NDP120_CHIP_CONFIG_SPICTL_MODE_MASK_INSERT(tmp_data, NDP120_CHIP_CONFIG_SPICTL_MODE_TRANSFER);
    tmp_data = NDP120_CHIP_CONFIG_SPICTL_MSPI_READ_MASK_INSERT(tmp_data, 0x1);
    tmp_data = NDP120_CHIP_CONFIG_SPICTL_NUMBYTES_MASK_INSERT(tmp_data, (uint32_t)bytes_to_transfer-1);
    tmp_data = NDP120_CHIP_CONFIG_SPICTL_MSSB_OE_MASK_INSERT(tmp_data, MSSB_OE_USED);
    tmp_data = NDP120_CHIP_CONFIG_SPICTL_MSSB_MASK_INSERT(tmp_data, (uint32_t)ssb);
    s = syntiant_ndp120_tiny_write_block(
        ndp, SYNTIANT_NDP120_MCU_OP, NDP120_CHIP_CONFIG_SPICTL, &tmp_data, 4);
    if (s) goto error;

    /* wait for mspi done */
    s = cspi_poll_done(ndp);
    if (s) goto error;

    s = syntiant_ndp120_tiny_read_block(
            ndp, SYNTIANT_NDP120_MCU_OP, NDP120_CHIP_CONFIG_SPIRX(0), data, (uint32_t)len);
    if (s) goto error;

    /* Wait for next packet or end sequence */
    s = syntiant_ndp120_tiny_read_block(
            ndp, SYNTIANT_NDP120_MCU_OP, NDP120_CHIP_CONFIG_SPICTL, &tmp_data, 4);
    if (s) goto error;
    if (end_packet == 0) {
        tmp_data = NDP120_CHIP_CONFIG_SPICTL_MODE_MASK_INSERT(tmp_data, NDP120_CHIP_CONFIG_SPICTL_MODE_UPDATE);
    } else {
        tmp_data = NDP120_CHIP_CONFIG_SPICTL_MODE_MASK_INSERT(tmp_data, NDP120_CHIP_CONFIG_SPICTL_MODE_SS);
    }
    s = syntiant_ndp120_tiny_write_block(
        ndp, SYNTIANT_NDP120_MCU_OP, NDP120_CHIP_CONFIG_SPICTL, &tmp_data, 4);
    if (s) goto error;

    if (end_packet != 0) {
        s = cspi_end(ndp);
        if (s) goto error;
    }

error:
    return s;
}

int syntiant_cspi_init(struct syntiant_ndp120_tiny_device_s *ndp)
{
    uint32_t tmp_data;
    int s;

    s = syntiant_ndp120_tiny_read_block(
            ndp, SYNTIANT_NDP120_MCU_OP, NDP120_CHIP_CONFIG_SPICTL, &tmp_data, 4);
    if (s) goto error;
    tmp_data = NDP120_CHIP_CONFIG_SPICTL_MSPI_MODE_MASK_INSERT(tmp_data, NDP120_CHIP_CONFIG_SPICTL_MSPI_MODE_FOUR_WIRE);
    tmp_data = NDP120_CHIP_CONFIG_SPICTL_MSSB_OE_MASK_INSERT(tmp_data, MSSB_OE_USED);
    tmp_data = NDP120_CHIP_CONFIG_SPICTL_MODE_MASK_INSERT(tmp_data, NDP120_CHIP_CONFIG_SPICTL_MODE_STANDBY);
    tmp_data = NDP120_CHIP_CONFIG_SPICTL_MSPI_CLKDIV_MASK_INSERT(tmp_data, MSPI_CLK_DIV);
    s = syntiant_ndp120_tiny_write_block(
        ndp, SYNTIANT_NDP120_MCU_OP, NDP120_CHIP_CONFIG_SPICTL, &tmp_data, 4);
    if (s) goto error;

error:
    return s;
}
