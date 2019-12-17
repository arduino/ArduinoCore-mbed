/*
 * Copyright(c) 2016, Analogix Semiconductor. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */



#include "anx7625_driver.h"
#include "anx7625_private_interface.h"

#include  "MI2_REG.h"
#include  "Flash.h"

#define ReadReg Read_Reg
#define CMD_LINE_SIZE 44
#define  MAX_BYTE_COUNT_PER_RECORD_FLASH    16


/* record types; only I8HEX files are supported,*/
/*so only record types 00 and 01 are used */
#define  HEX_RECORD_TYPE_DATA   0
#define  HEX_RECORD_TYPE_EOF    1

#define HEX_LINE_SIZE 45

unsigned char const OCM_FW_HEX[][HEX_LINE_SIZE] = {
/*#include "MI2_main_ocm.h"*/
#include "MI2_main_ocm1300.h"
};
unsigned char const SECURE_OCM_HEX[][HEX_LINE_SIZE] = {
#include "MI2_secure_ocm.h"
};
static unsigned short int  BurnHexIndex;


/* for early tests before MI-2 chip is back */

#define  FLASH_TT_REF_SIZE     (2048)

#define  FLASH_PAGE_SIZE       (256)
#define  FLASH_SECTOR_SIZE     (4 * 1024)
#define  FLASH_BLOCK_SIZE_32K  (32 * 1024)
#define  FLASH_BLOCK_SIZE_64K  (64 * 1024)

u8 g_bFlashWrite;

struct tagFlashRWinfo g_FlashRWinfo;
unsigned char *g_CmdLineBuf;


/* wait until WIP (Write In Progress) bit cleared */
void flash_wait_until_WIP_cleared(void)
{
	unsigned char  tmp;

	do {
		read_status_enable();
		ReadReg(RX_P0, STATUS_REGISTER, &tmp);
	} while ((tmp & 1) != 0);
}


/* wait until MI-2 Flash controller hardware state machine*/
/*returns to idle state */
void flash_wait_until_flash_SM_done(void)
{
	unsigned char  tmp;

	do {
		ReadReg(RX_P0, R_RAM_CTRL, &tmp);
		printf("tmp: %x\n", tmp);
	} while (((tmp >> FLASH_DONE) & 1) == 0);
}


/* basic configurations of the Flash controller,*/
/* and some global variables initialization  */
/* This is not a debug command, but called by PROC_Main().*/
void flash_basic_config(void)
{
	WriteReg(RX_P0, XTAL_FRQ_SEL, XTAL_FRQ_27M);
	flash_wait_until_flash_SM_done();
	flash_write_disable();
#ifndef DRY_RUN
	flash_wait_until_WIP_cleared();
#endif
	flash_wait_until_flash_SM_done();
}


/* Flash write protection range */
#define  FLASH_PROTECTION_ALL \
((1 << BP4) | (1 << BP3) | (1 << BP2) | (1 << BP1) | (1 << BP0))

#define  FLASH_PROTECTION_PATTERN_MASK \
((1 << SRP0) | (1 << BP4) | (1 << BP3) | (1 << BP2) | (1 << BP1) | (1 << BP0))
#define  HW_FLASH_PROTECTION_PATTERN   ((1 << SRP0) | FLASH_PROTECTION_ALL)
#define  SW_FLASH_PROTECTION_PATTERN   ((0 << SRP0) | FLASH_PROTECTION_ALL)
/* enable Flash hardware write protection */
void flash_HW_write_protection_enable(void)
{
	unsigned char RegData;

	/* WP# pin of Flash die = high, not hardware write protected*/
	ReadReg(RX_P0, GPIO_CTRL_1, &RegData);
	RegData |= (WRITE_UNPROTECTED << FLASH_WP);
	WriteReg(RX_P0, GPIO_CTRL_1, RegData);

	RegData = HW_FLASH_PROTECTION_PATTERN;
	flash_wait_until_flash_SM_done();
	flash_write_status_register(RegData);
#ifndef DRY_RUN
	flash_wait_until_WIP_cleared();
#endif

	/* WP# pin of Flash die = low, hardware write protected*/
	ReadReg(RX_P0, GPIO_CTRL_1, &RegData);
	RegData &= ~(WRITE_UNPROTECTED << FLASH_WP);
	WriteReg(RX_P0, GPIO_CTRL_1, RegData);

	flash_wait_until_flash_SM_done();
	read_status_enable();
	ReadReg(RX_P0, STATUS_REGISTER, &RegData);
	flash_wait_until_flash_SM_done();

	if ((RegData & FLASH_PROTECTION_PATTERN_MASK) ==
		HW_FLASH_PROTECTION_PATTERN) {
		TRACE("Flash hardware write protection enabled.\n");
	} else {
		TRACE("Enabling Flash hardware write protection FAILED!\n");
	}
}


/* disable Flash write protection */
void flash_write_protection_disable(void)
{
	unsigned char RegData;

	/* WP# pin of Flash die = high, not hardware write protected*/
	ReadReg(RX_P0, GPIO_CTRL_1, &RegData);
	RegData |= (WRITE_UNPROTECTED << FLASH_WP);
	WriteReg(RX_P0, GPIO_CTRL_1, RegData);

	RegData = 0;
	flash_wait_until_flash_SM_done();
	flash_write_status_register(RegData);
#ifndef DRY_RUN
	flash_wait_until_WIP_cleared();
#endif
	flash_wait_until_flash_SM_done();
	read_status_enable();
	ReadReg(RX_P0, STATUS_REGISTER, &RegData);
	flash_wait_until_flash_SM_done();

	if ((RegData & FLASH_PROTECTION_PATTERN_MASK) == 0)
		TRACE("Flash write protection disabled.\n");
	else
		TRACE("Disabling Flash write protection FAILED!\n");
}


/* Flash Sector Erase */
/* Usage: fl_SE <address> */
void command_flash_SE(void)
{
/* Flash address, any address inside the sector is a valid address*/
/* for the Sector Erase (SE) command*/
	unsigned int Flash_Addr;

	if (sscanf((const char*)g_CmdLineBuf, "\\%*s %x", &Flash_Addr) == 1) {
		flash_write_protection_disable();
		flash_sector_erase(Flash_Addr);
#ifndef DRY_RUN
		flash_wait_until_WIP_cleared();
#endif
		flash_wait_until_flash_SM_done();
		TRACE2("Sector erase done: 0x%04X ~ 0x%04X\n",
			(Flash_Addr >> 12) * FLASH_SECTOR_SIZE,
			((Flash_Addr + FLASH_SECTOR_SIZE) >> 12) *
			FLASH_SECTOR_SIZE - 1);
		flash_HW_write_protection_enable();
	} else {
		TRACE("Bad parameter! Usage:\n");
		TRACE("fl_SE <address>\n");
	}
}


/* Erase OCM firmware */
void command_erase_FW(unsigned char FW_ID)
{
/* Flash address, any address inside the sector is a valid address*/
/* for the Sector Erase (SE) command*/
	unsigned int Flash_Addr;

	flash_write_protection_disable();

	if (FW_ID == MAIN_OCM) {
		for (Flash_Addr = MAIN_OCM_FW_ADDR_BASE;
			Flash_Addr <= MAIN_OCM_FW_ADDR_END;
			Flash_Addr += FLASH_SECTOR_SIZE) {
			flash_sector_erase(Flash_Addr);
#ifndef DRY_RUN
			flash_wait_until_WIP_cleared();
#endif
			flash_wait_until_flash_SM_done();
		}
		TRACE("Main OCM firmware erased.\n");
	} else if (FW_ID == SECURE_OCM) {
		for (Flash_Addr = SECURE_OCM_FW_ADDR_BASE;
			Flash_Addr <= SECURE_OCM_FW_ADDR_END;
			Flash_Addr += FLASH_SECTOR_SIZE) {
			flash_sector_erase(Flash_Addr);
#ifndef DRY_RUN
			flash_wait_until_WIP_cleared();
#endif
			flash_wait_until_flash_SM_done();
		}
		TRACE("Secure OCM firmware erased.\n");
	}

	flash_HW_write_protection_enable();
}


/* Flash Chip Erase */
/* Usage: fl_CE */
void command_flash_CE(void)
{
	flash_write_protection_disable();
	flash_chip_erase();
#ifndef DRY_RUN
	flash_wait_until_WIP_cleared();
#endif
	flash_wait_until_flash_SM_done();
	TRACE("Whole Flash chip erased.\n");
	flash_HW_write_protection_enable();
}

void command_erase_sector(int start_index, int i_count)
{
	unsigned int i;

	WriteReg(RX_P0, 0x88,  0x40); /*reset main ocm*/
	WriteReg(TX_P0, 0x80,  0x00); /*reset secure ocm*/

	flash_write_protection_disable();

	for (i = start_index; i < start_index + i_count; i++) {
		flash_sector_erase(0x1000 * i);
#ifndef DRY_RUN
		flash_wait_until_WIP_cleared();
#endif
		flash_wait_until_flash_SM_done();
	}
	flash_HW_write_protection_enable();

	TRACE("MI-2 firewarm erase done.\n");
}
void command_erase_mainfw(void)
{

	WriteReg(RX_P0, 0x88,  0x40); /*reset main ocm*/
	WriteReg(TX_P0, 0x80,  0x00); /*reset secure ocm*/


	command_erase_FW(MAIN_OCM);

	TRACE("MI-2 Main firmware erase done.\n");
}


void command_erase_securefw(void)
{

	WriteReg(RX_P0, 0x88,  0x40); /*reset main ocm*/
	WriteReg(TX_P0, 0x80,  0x00); /*reset secure ocm*/


	command_erase_FW(SECURE_OCM);

	TRACE("MI-2 SECURE firmware erase done.\n");
}


/* reads Flash (32 bytes at a time), and prints the data on the UART console, */
/* so that the data can be saved in a HEX file */
/* Usage: fl_readA <base_address> <size_to_be_read> */
void command_flash_read(unsigned int addr, unsigned long read_size)
{
	unsigned char  ReadDataBuf[MAX_BYTE_COUNT_PER_RECORD_FLASH];
	unsigned char  i;  /* counter */
	unsigned int   Address;
	unsigned long  size_to_be_read;
	unsigned long  total_bytes_read = 0;
	unsigned char  checksum;

	unsigned char  pLine[CMD_LINE_SIZE + 4];


	Address = addr;
	size_to_be_read = read_size;


	WriteReg(RX_P0, 0x88,  0x40); /*reset main ocm*/
	WriteReg(TX_P0, 0x80,  0x00); /*reset secure ocm*/



	if (Address % MAX_BYTE_COUNT_PER_RECORD_FLASH != 0) {
		TRACE2("ERROR! Address = 0x%04X, not %u bytes aligned.\n",
			Address, MAX_BYTE_COUNT_PER_RECORD_FLASH);
		return;
	}

	flash_wait_until_flash_SM_done();
	Address -= 1;

	if (size_to_be_read < MAX_BYTE_COUNT_PER_RECORD_FLASH)
		goto  THE_LAST_READ;

	while (1) {
		WriteReg(RX_P0, FLASH_ADDR_H, Address >> 8);
		WriteReg(RX_P0, FLASH_ADDR_L, Address & 0xFF);

		WriteReg(RX_P0, FLASH_LEN_H, 0);
		/* Reads 32 bytes*/
		WriteReg(RX_P0, FLASH_LEN_L, FLASH_READ_MAX_LENGTH - 1);

		read_enable();
		flash_wait_until_flash_SM_done();

	/* == Reads 32 bytes, but only keeps 16 bytes (offset: 1~16) == */
		for (i = 0; i < MAX_BYTE_COUNT_PER_RECORD_FLASH; i++) {
			ReadReg(RX_P0, FLASH_READ_BUF_BASE_ADDR + 1 + i,
				&ReadDataBuf[i]);
		}

		snprintf((char*)&(pLine[0]), 10, ":%02hhX%04hX%02hhX",
			MAX_BYTE_COUNT_PER_RECORD_FLASH,
			Address + 1, HEX_RECORD_TYPE_DATA);

		checksum = MAX_BYTE_COUNT_PER_RECORD_FLASH +
			((Address + 1) >> 8)
			+ ((Address + 1) & 0xFF) + HEX_RECORD_TYPE_DATA;
		for (i = 0; i < MAX_BYTE_COUNT_PER_RECORD_FLASH; i++) {
			snprintf((char*)&(pLine[i * 2 + 9]), 3, "%02hhX",
				ReadDataBuf[i]);
			checksum += ReadDataBuf[i];
		}

		snprintf((char*)&(pLine[32 + 9]), 3, "%02hhX", -checksum);
		pLine[32 + 9 + 2] = '\0';
		pr_info("%s\n", pLine);



		Address += MAX_BYTE_COUNT_PER_RECORD_FLASH;
		total_bytes_read += MAX_BYTE_COUNT_PER_RECORD_FLASH;
		size_to_be_read  -= MAX_BYTE_COUNT_PER_RECORD_FLASH;

/* the last read*/
		if (size_to_be_read < MAX_BYTE_COUNT_PER_RECORD_FLASH) {
THE_LAST_READ:
			if (size_to_be_read > 0) {
				WriteReg(RX_P0, FLASH_ADDR_H, Address >> 8);
				WriteReg(RX_P0, FLASH_ADDR_L, Address & 0xFF);

				WriteReg(RX_P0, FLASH_LEN_H, 0);
				WriteReg(RX_P0, FLASH_LEN_L,
					FLASH_READ_MAX_LENGTH - 1);

				read_enable();
				flash_wait_until_flash_SM_done();

				for (i = 0; i < size_to_be_read; i++) {
					ReadReg(RX_P0,
					FLASH_READ_BUF_BASE_ADDR + 1 + i,
					&ReadDataBuf[i]);
				}
				snprintf((char*)&(pLine[0]), 10, ":%02hhX%04hX%02hhX",
				(unsigned char)size_to_be_read, Address + 1,
				HEX_RECORD_TYPE_DATA);

			checksum = size_to_be_read + ((Address + 1) >> 8) +
				((Address + 1) & 0xFF) + HEX_RECORD_TYPE_DATA;

			for (i = 0; i < size_to_be_read; i++) {
				snprintf((char*)&(pLine[i * 2 + 9]), 3, "%02hhX",
					ReadDataBuf[i]);
				checksum += ReadDataBuf[i];
			}
				snprintf((char*)&(pLine[i * 2 + 9]), 3, "%02hhX",
					-checksum);
				pLine[i * 2 + 9 + 2] = '\0';
				pr_info("%s\n", pLine);

				total_bytes_read += size_to_be_read;
				/* no need to update size_to_be_read anymore*/
			}

			/* write EOF record*/
			snprintf((char*)&(pLine[0]), 10, ":%02hhX%04hX%02hhX",
				(unsigned char)0, (unsigned int)0,
				HEX_RECORD_TYPE_EOF);
			checksum = 0 + (0 >> 8) + (0 & 0xFF) +
				HEX_RECORD_TYPE_EOF;
			snprintf((char*)&(pLine[9]), 3, "%02hhX", -checksum);
			pLine[9 + 2] = '\0';
			pr_info("%s\n", pLine);

			TRACE1("\n\nFlash read done. %lu bytes read.\n",
				total_bytes_read);
			break;
		}
	}

}

void HEX_file_validity_check(unsigned int Address,
	unsigned char ByteCount, char return_code)
{
	if (return_code != 0) {
		TRACE2("HEX file error! Address = 0x%04X, error code = %d\n",
			Address, return_code);
		TRACE("Please power cycle the EVB and check the HEX file!\n");
		/* hangs deliberately so that the user can see it */
		/*while (1);*/
		return;
	}

	if (ByteCount > MAX_BYTE_COUNT_PER_RECORD_FLASH) {
		TRACE2("ERROR! ByteCount = %u > %u\n",
			ByteCount, MAX_BYTE_COUNT_PER_RECORD_FLASH);
		TRACE("Please power cycle the EVB and check the HEX file!\n");
		/* hangs deliberately so that the user can see it */
		/*while (1);*/
		return;
	}

	if ((Address % MAX_BYTE_COUNT_PER_RECORD_FLASH) != 0) {
		TRACE2("ERROR! Address = 0x%04X, not %u bytes aligned.\n",
			Address, MAX_BYTE_COUNT_PER_RECORD_FLASH);
		TRACE("Please power cycle the EVB and check the HEX file!\n");
		/* hangs deliberately so that the user can see it */
		/*while (1);*/
		return;
	}
}


void flash_write_prepare(unsigned int Address, unsigned char offset,
	unsigned char ByteCount, unsigned char *WriteDataBuf)
{
	unsigned char i;  /* counter */

	flash_write_enable();

	WriteReg(RX_P0, FLASH_ADDR_H, Address >> 8);
	WriteReg(RX_P0, FLASH_ADDR_L, Address & 0xFF);

	for (i = 0; i < ByteCount; i++)
		WriteReg(RX_P0, FLASH_WRITE_BUF_BASE_ADDR + offset + i,
			WriteDataBuf[i]);
}


void flash_actual_write(void)
{
/* This is waiting for previous flash_write_enable() is done, */
/* i.e. writing status register is done */
#ifndef DRY_RUN
	flash_wait_until_WIP_cleared();
	write_enable();
	flash_wait_until_WIP_cleared();
#endif

	flash_wait_until_flash_SM_done();
}


/* program Flash with data in a HEX file, 32 bytes at a time */
/* When address is not 32-byte aligned, simply notify the user and exit */
/* Since address is always 32-byte aligned, and 32 bytes are written */
/* into Flash at a time, */
/* crossing 256-byte (FLASH_PAGE_SIZE) boundary will NEVER happen, */
/* thus no need to handle this. */
void flash_program(int hex_index)
{
	unsigned char WriteDataBuf[MAX_BYTE_COUNT_PER_RECORD_FLASH];
	unsigned char ByteCount;
	unsigned int  Address;
	unsigned char RecordType;
	unsigned char i;  /* counter */
	char  return_code = 0;

	if (hex_index == 1)
		g_CmdLineBuf = (unsigned char *)SECURE_OCM_HEX[BurnHexIndex++];
	else
		g_CmdLineBuf = (unsigned char *)OCM_FW_HEX[BurnHexIndex++];

	WriteReg(RX_P0, 0x88,  0x40); /*reset main ocm*/
	WriteReg(TX_P0, 0x80,  0x00); /*reset secure ocm*/

	flash_wait_until_flash_SM_done();

	WriteReg(RX_P0, FLASH_LEN_H, (FLASH_WRITE_MAX_LENGTH - 1) >> 8);
	WriteReg(RX_P0, FLASH_LEN_L, (FLASH_WRITE_MAX_LENGTH - 1) & 0xFF);

	/* ==================== Ping: accumulates data ================ */
	if (g_FlashRWinfo.prog_is_Ping) {
		return_code = GetLineData(g_CmdLineBuf, &ByteCount,
			&Address, &RecordType, WriteDataBuf);
		HEX_file_validity_check(Address, ByteCount, return_code);

		if (RecordType == HEX_RECORD_TYPE_EOF) { /* end of HEX file */
			g_bFlashWrite = 0;
			TRACE1("\nFlash program done. %lu bytes written....\n",
				g_FlashRWinfo.total_bytes_written);
			return;
		}

write_prepare_in_ping:
		flash_write_prepare(Address, (unsigned char)0,
			ByteCount, &WriteDataBuf[0]);
		g_FlashRWinfo.previous_addr = Address;

		g_FlashRWinfo.bytes_accumulated_in_Ping = ByteCount;
		g_FlashRWinfo.prog_is_Ping = 0;

/*We're now in ping, but we have to do something */
/*that is normally done in pong (the Address dictates this),*/
		if ((Address % FLASH_WRITE_MAX_LENGTH) != 0) {
			/* so that we can recover the ping-pong cadence.*/
		for (i = 0; i < MAX_BYTE_COUNT_PER_RECORD_FLASH; i++)
			WriteReg(RX_P0, FLASH_WRITE_BUF_BASE_ADDR + i, 0xFF);

		flash_write_prepare(Address - MAX_BYTE_COUNT_PER_RECORD_FLASH,
			(unsigned char)MAX_BYTE_COUNT_PER_RECORD_FLASH,
			ByteCount, &WriteDataBuf[0]);
		flash_actual_write();
		g_FlashRWinfo.total_bytes_written += ByteCount;
		g_FlashRWinfo.bytes_accumulated_in_Ping = 0;
		g_FlashRWinfo.prog_is_Ping = 1;
		g_FlashRWinfo.previous_addr = Address;
		}
		return;
	}

/* ============= Pong: program Flash =============== */
/* note: GetLineData() can ONLY be called ONCE per flash_program() invoke*/
/* otherwise serial port buffer has no chance to be updated, */
/* and the same HEX record is used twice, */
	/* which is incorrect. */
	if (!g_FlashRWinfo.prog_is_Ping) {
		return_code = GetLineData(g_CmdLineBuf, &ByteCount,
			&Address, &RecordType, WriteDataBuf);
		HEX_file_validity_check(Address, ByteCount, return_code);

		if (RecordType == HEX_RECORD_TYPE_EOF) {/* end of HEX file */
		for (i = 0; i < MAX_BYTE_COUNT_PER_RECORD_FLASH; i++)
			WriteReg(RX_P0,
		FLASH_WRITE_BUF_BASE_ADDR + MAX_BYTE_COUNT_PER_RECORD_FLASH + i,
		0xFF);


			flash_actual_write();
			g_FlashRWinfo.total_bytes_written +=
				g_FlashRWinfo.bytes_accumulated_in_Ping;
			g_bFlashWrite = 0;
			TRACE1("\n\nFlash program done. %lu bytes written.\n\n",
				g_FlashRWinfo.total_bytes_written);
			return;
		}

		if (((Address % FLASH_WRITE_MAX_LENGTH) != 0) &&
			(Address == g_FlashRWinfo.previous_addr +
			MAX_BYTE_COUNT_PER_RECORD_FLASH)) {
			/* contiguous address*/
			for (i = 0; i < ByteCount; i++) {
				WriteReg(RX_P0,
				FLASH_WRITE_BUF_BASE_ADDR +
				g_FlashRWinfo.bytes_accumulated_in_Ping + i,
				WriteDataBuf[i]);
			}
			flash_actual_write();
			g_FlashRWinfo.total_bytes_written +=
			(g_FlashRWinfo.bytes_accumulated_in_Ping + ByteCount);
			g_FlashRWinfo.bytes_accumulated_in_Ping = 0;
			g_FlashRWinfo.previous_addr = Address;
			g_FlashRWinfo.prog_is_Ping = 1;
		} else if (((Address % FLASH_WRITE_MAX_LENGTH) != 0) &&
			(Address != g_FlashRWinfo.previous_addr +
			MAX_BYTE_COUNT_PER_RECORD_FLASH)) {
			/* address is not contiguous*/
			for (i = 0; i < MAX_BYTE_COUNT_PER_RECORD_FLASH; i++) {
				WriteReg(RX_P0,
					FLASH_WRITE_BUF_BASE_ADDR +
					MAX_BYTE_COUNT_PER_RECORD_FLASH + i,
					0xFF);
			}
			flash_write_enable();
			WriteReg(RX_P0, FLASH_ADDR_H,
				g_FlashRWinfo.previous_addr >> 8);
			WriteReg(RX_P0, FLASH_ADDR_L,
				g_FlashRWinfo.previous_addr & 0xFF);
			/* write what was received in ping*/
			flash_actual_write();
			g_FlashRWinfo.total_bytes_written +=
				g_FlashRWinfo.bytes_accumulated_in_Ping;
			g_FlashRWinfo.bytes_accumulated_in_Ping = 0;

			for (i = 0; i < MAX_BYTE_COUNT_PER_RECORD_FLASH; i++)
				WriteReg(RX_P0, FLASH_WRITE_BUF_BASE_ADDR + i,
					0xFF);

		flash_write_prepare(
			Address - MAX_BYTE_COUNT_PER_RECORD_FLASH,
			(unsigned char)MAX_BYTE_COUNT_PER_RECORD_FLASH,
			ByteCount, &WriteDataBuf[0]);

		/* write what is received in this pong*/
			flash_actual_write();
			g_FlashRWinfo.total_bytes_written += ByteCount;
			g_FlashRWinfo.previous_addr = Address;
			g_FlashRWinfo.prog_is_Ping = 1;
		} else if (((Address % FLASH_WRITE_MAX_LENGTH) == 0) &&
			(Address != g_FlashRWinfo.previous_addr +
			MAX_BYTE_COUNT_PER_RECORD_FLASH)){
			for (i = 0; i < MAX_BYTE_COUNT_PER_RECORD_FLASH; i++) {
				WriteReg(RX_P0,
					FLASH_WRITE_BUF_BASE_ADDR +
					MAX_BYTE_COUNT_PER_RECORD_FLASH + i,
					0xFF);
			}
			flash_write_enable();
			WriteReg(RX_P0, FLASH_ADDR_H,
				g_FlashRWinfo.previous_addr >> 8);
			WriteReg(RX_P0, FLASH_ADDR_L,
				g_FlashRWinfo.previous_addr & 0xFF);
			/* write what was received in ping*/
			flash_actual_write();
			g_FlashRWinfo.total_bytes_written +=
				g_FlashRWinfo.bytes_accumulated_in_Ping;
			g_FlashRWinfo.bytes_accumulated_in_Ping = 0;
			goto write_prepare_in_ping;
		} else {
			TRACE("ERROR in flash_program()\n");
			TRACE("Please check the HEX file!\n");
	/*while (1);  hangs deliberately so that the user can see it */
		}
	}
}


unsigned short int strtoval(unsigned char *str, unsigned char n)
{
	unsigned char c;
	unsigned char i;
	unsigned short int rsult = 0;

	for (i = 0; i < n; i++) {
		c = str[i];

		rsult = rsult * 16;

		if (c >= '0' && c <= '9')
			rsult += (c - '0');
		else if (c >= 'A' && c <= 'F')
			rsult += (c - 'A' + 10);
		else if (c >= 'a' && c <= 'f')
			rsult += (c - 'a' + 10);

	}
	return rsult;
}

char GetLineData(unsigned char *pLine, unsigned char *pByteCount,
	unsigned int *pAddress, unsigned char *pRecordType,
	unsigned char *pData)
{
	unsigned int ValBuf[3];
	unsigned char checksum;
	unsigned char sum;
	unsigned char i;

	if (sscanf((const char*)pLine, ":%2x%4x%2x", ValBuf, ValBuf + 1, ValBuf + 2) == 3) {

		*pByteCount = strtoval(pLine + 1, 2);
		*pAddress = strtoval(pLine + 1 + 2, 4);
		*pRecordType = strtoval(pLine + 1 + 2 + 4, 2);

		sum = *pByteCount + *pAddress / 256 +
			*pAddress % 256 + *pRecordType;


		pLine += 1 + 2 + 4 + 2;
		for (i = *pByteCount; i != 0; i--) {
			if (sscanf((const char*)pLine, "%2x", ValBuf) == 1) {
				*pData = strtoval(pLine, 2);
				sum += *pData;
				pData++;
				pLine += 2;
			} else {
				return -EPERM;
			}
		}
		if (sscanf((const char*)pLine, "%2x", ValBuf) == 1) {

			checksum = strtoval(pLine, 2);

			if ((char)(sum + checksum) == 0) {
				/*TRACE1("BHBA sum%2hhx ",sum); // test*/
				return 0;
			} else {
				return -EPERM;
			}
		} else {
			return -EPERM;
		}
	} else {
		return -EPERM;
	}
}

void SetLineData(unsigned char *pLine, unsigned char ByteCount,
	unsigned short int Address, unsigned char RecordType,
	unsigned char *pData)
{
	unsigned char checksum;

	snprintf((char*)pLine, 10, ":%02hhX%04hX%02hhX",
		ByteCount, Address, RecordType);
	pLine += 1 + 2 + 4 + 2;
	checksum = ByteCount + Address / 256 + Address % 256 + RecordType;
	for (; ByteCount;  ByteCount--) {
		snprintf((char*)pLine, 3, "%02hhX", *pData);
		pLine += 2;
		checksum += *pData;
		pData++;
	}
	snprintf((char*)pLine, 3, "%02hhX", -checksum);
	pLine += 2;
	*pLine = '\0';
}

void TRACE_ARRAY(unsigned char array[], unsigned char len)
{
/*
*	unsigned char i;
*
*	i = 0;
*	while (1) {
*		printk("%02X", array[i]);
*		i++;
*		if (i != len) {
*			printk(" ");
*		} else {
*			printk("\n");
*			break;
*		}
*	}
*/
}


void burnhex(int file_index)
{

	debug_on = 1;

	if (atomic_read(&anx7625_power_status) == 0) {
		/* power on chip first*/
		MI2_power_on();
	} else {
		pr_info("Chip already power on!\n");
		anx7625_hardware_reset(0);
		usleep_range(10000, 11000);
		anx7625_hardware_reset(1);
		usleep_range(10000, 11000);
		pr_info("Chip reset and release!\n");
	}
	usleep_range(10000, 11000);

	g_bFlashWrite = 1;
	g_FlashRWinfo.total_bytes_written = 0;
	g_FlashRWinfo.prog_is_Ping = 1;
	TRACE("burn begin...\n");

	BurnHexIndex = 0;
	flash_write_protection_disable();

	usleep_range(50000, 51000);/* wait power ready*/

	while  (g_bFlashWrite)
		flash_program(file_index);

	flash_HW_write_protection_enable();

	if (atomic_read(&anx7625_power_status) == 0)
		anx7625_power_standby();

	debug_on = 0;
}

void flash_section_loader(unsigned char type,
	unsigned int address, unsigned int length)
{
	/*enable hdcp 2.2 OCM load*/
	WriteReg(RX_P0, 0x00, type); /*select hdcp 2.2 tx firmware load*/
	WriteReg(RX_P0, 0x0f, address >> 8); /*select flash high byte*/
	WriteReg(RX_P0, 0x10, address & 0xff); /*select flash low byte*/
	WriteReg(RX_P0, 0x03, length >> 12); /*select flash length high byte*/
	/*select flash length low byte, real length/16*/
	WriteReg(RX_P0, 0x04, length >> 4);
	if ((type & 0xf0) == 0x60)
		WriteReg(RX_P0, 0x01, 0x06); /*select sram length high byte*/
	else
		WriteReg(RX_P0, 0x01, 0x00); /*select sram length high byte*/
	WriteReg(RX_P0, 0x02, 0x00); /*select sram length low byte*/
	WriteReg(RX_P0, R_RAM_CTRL, 0x03); /*enable load   0x3*/
}

#define VERSION_ADDR 0x2000

unsigned char readhexver(unsigned char *pData, unsigned char selectHex)
{
	unsigned char *LineBuf;
	unsigned short int Index = 0;
	unsigned short int Lines = 0;
	unsigned char WriteDataBuf[16];
	unsigned char ByteCount;
	unsigned int Address;
	unsigned char RecordType;

	if (selectHex == 1)
		/*Lines = sizeof(OCM_FW_HEX) / sizeof(OCM_FW_HEX[0]);*/
		Lines = ARRAY_SIZE(OCM_FW_HEX);
	else
		/*Lines = sizeof(OCM_FW_HEX) / sizeof(OCM_FW_HEX[0]);*/
		Lines = ARRAY_SIZE(OCM_FW_HEX);

	while (Index < Lines) {
		if (selectHex == 1)
			LineBuf = (unsigned char *)OCM_FW_HEX[Index++];
		else
			LineBuf = (unsigned char *)OCM_FW_HEX[Index++];

		if (GetLineData(LineBuf, &ByteCount,
			&Address, &RecordType, WriteDataBuf) == 0) {
			if (RecordType == 0) {
				if (Address == VERSION_ADDR) {
					/* main version*/
				pData[0] = WriteDataBuf[VERSION_ADDR - Address];
				/* minor version*/
				pData[1] =
				WriteDataBuf[VERSION_ADDR + 1 - Address];
				break;
				}
			} else if (RecordType == 1) {
				pr_info("Hex file end!\n");
				return 1;
			}
		} else {
			pr_info("Hex file error!\n");
			return 1;
		}
	}
	return 0;
}


void burnhexauto(void)
{

	unsigned char VerBuf[2];
	unsigned short int VersionHex;
	unsigned short int VersionChip;

	debug_on = 1;

	pr_info("burnhexauto.\n");


	if (atomic_read(&anx7625_power_status) == 0) {
		/* power on chip first*/
		MI2_power_on();
	} else {
		pr_info("Chip already power on!\n");
		anx7625_hardware_reset(0);
		usleep_range(10000, 11000);
		anx7625_hardware_reset(1);
		usleep_range(10000, 11000);
		pr_info("Chip reset and release!\n");
	}

	readhexver(VerBuf, 0);
	VersionHex = ((unsigned short int)VerBuf[0] << 8) + VerBuf[1];
	pr_info("Version from hex: %2hx%2hx\n",
		VersionHex >> 8, VersionHex & 0xFF);


	ReadReg(OCM_SLAVE_I2C_ADDR, OCM_FW_VERSION, &VerBuf[0]);
	ReadReg(OCM_SLAVE_I2C_ADDR, OCM_FW_REVERSION, &VerBuf[1]);

	VersionChip = ((unsigned short int)VerBuf[0] << 8) + VerBuf[1];
	pr_info("Version from chip: %2hx%2hx\n",
		VersionChip >> 8, VersionChip & 0xFF);

	/* Check load done*/
	if (VersionHex == VersionChip) {
		if (atomic_read(&anx7625_power_status) == 0)
			anx7625_power_standby();

		pr_info("Same version not need update.\n");
		debug_on = 0;
		return;
	}

	pr_info("Update version.\n");
	command_erase_mainfw();
	burnhex(0);

	debug_on = 0;

}






