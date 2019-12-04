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


#ifndef __FLASH_H__
#define __FLASH_H__

#include "arduino_hal.h"

struct tagFlashRWinfo {
	unsigned long  total_bytes_written;
	unsigned int   previous_addr;
	unsigned char  prog_is_Ping; /* Ping-Pong flag*/
	unsigned char  bytes_accumulated_in_Ping;
};


/* default value is 0 */
/* Flash SPI controller debug register, use default value is recommended. */
#define READ_DELAY_SELECT_VALUE  0

#define  FLASH_SECTOR_SIZE     (4 * 1024)

/* FW_ID (firmware ID)*/
#define  MAIN_OCM     0
#define  SECURE_OCM   1

/* Firmware address*/
#define  MAIN_OCM_FW_ADDR_BASE     0x1000
#define  MAIN_OCM_FW_ADDR_END      0x8FFF

#define  SECURE_OCM_FW_ADDR_BASE     0xA000
#define  SECURE_OCM_FW_ADDR_END      0xCFFF


/* inline function is not supported, thus define macros */
#define write_general_instruction(instruction_type) \
	WriteReg(RX_P0, GENERAL_INSTRUCTION_TYPE, instruction_type)

#define write_status_register(value) \
	WriteReg(RX_P0, STATUS_REGISTER_IN, value)

#define general_instruction_enable() \
	WriteReg(RX_P0, R_FLASH_RW_CTRL,\
	(READ_DELAY_SELECT_VALUE << READ_DELAY_SELECT) |\
	(1 << GENERAL_INSTRUCTION_EN))

#define erase_enable() \
	WriteReg(RX_P0, R_FLASH_RW_CTRL,\
	(READ_DELAY_SELECT_VALUE << READ_DELAY_SELECT) |\
	(1 << FLASH_ERASE_EN))

#define write_status_enable() \
	WriteReg(RX_P0, R_FLASH_RW_CTRL,\
	(READ_DELAY_SELECT_VALUE << READ_DELAY_SELECT) |\
	(1 << WRITE_STATUS_EN))

#define read_enable() \
	WriteReg(RX_P0, R_FLASH_RW_CTRL,\
	(READ_DELAY_SELECT_VALUE << READ_DELAY_SELECT) |\
	(1 << FLASH_READ))

#define write_enable() \
	WriteReg(RX_P0, R_FLASH_RW_CTRL,\
	(READ_DELAY_SELECT_VALUE << READ_DELAY_SELECT) |\
	(1 << FLASH_WRITE))

#define erase_type(type) \
	WriteReg(RX_P0, FLASH_ERASE_TYPE, type)

#define flash_address(addr) \
do { \
	WriteReg(RX_P0, FLASH_ADDR_L, addr); \
	WriteReg(RX_P0, FLASH_ADDR_H, addr>>8); \
} while (0)

#define read_status_enable() \
do { \
	unsigned char tmp; \
	ReadReg(RX_P0, R_DSC_CTRL_0, &tmp); \
	tmp |= (1<<READ_STATUS_EN); \
	WriteReg(RX_P0, R_DSC_CTRL_0, tmp);\
} while (0)

#define flash_write_enable()   \
do {\
	write_general_instruction(WRITE_ENABLE); \
	general_instruction_enable();\
} while (0)

#define flash_write_disable()  \
do {\
	write_general_instruction(WRITE_DISABLE); \
	general_instruction_enable();\
} while (0)

#define flash_write_status_register(value)  \
do {\
	flash_write_enable(); \
	write_status_register(value); \
	write_status_enable();\
} while (0)

#define flash_chip_erase()  \
do {\
	flash_write_enable(); \
	write_general_instruction(CHIP_ERASE_A); \
	general_instruction_enable();\
} while (0)

#define flash_sector_erase(addr)  \
do {\
	flash_write_enable(); \
	flash_address(addr); \
	erase_type(SECTOR_ERASE); \
	erase_enable();\
} while (0)

char GetLineData(unsigned char *pLine, unsigned char *pByteCount,
	       unsigned int *pAddress, unsigned char *pRecordType,
	       unsigned char *pData);
void SetLineData(unsigned char *pLine, unsigned char ByteCount,
	       unsigned short int Address, unsigned char RecordType,
	       unsigned char *pData);
extern unsigned char debug_on;

/* basic configurations of the Flash controller*/
void flash_basic_config(void);
/* wait until WIP (Write In Progress) bit cleared*/
void flash_wait_until_WIP_cleared(void);
/* wait until MI-2 Flash controller hardware state machine */
/* returns to idle state*/
void flash_wait_until_flash_SM_done(void);
/* enable Flash hardware write protection*/
void flash_HW_write_protection_enable(void);
/* disable Flash write protection*/
void flash_write_protection_disable(void);
/* Sector Erase*/
void command_flash_SE(void);
/* Erase OCM firmware*/
void command_erase_FW(unsigned char FW_ID);
/* Chip Erase*/
void command_flash_CE(void);
void command_flash_BC(void);        /* Blank Check*/
void command_flash_read(unsigned int addr, unsigned long read_size);
/* program Flash with data in a HEX file*/
void flash_program(int hex_index);
void burnhex(int hex_index);
void command_erase_sector(int start_index, int i_count);
void command_erase_mainfw(void);
void command_erase_securefw(void);



void flash_section_loader(unsigned char type,
	unsigned int address, unsigned int length);
void burnhexauto(void);



#endif  /* __FLASH_H__ */

