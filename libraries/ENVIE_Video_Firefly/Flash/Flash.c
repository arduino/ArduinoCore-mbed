 /* file: Flash.c
 * description: Mississippi-2 (ANX7625) stack die Flash
 *              access routine implementations
*/

#include  <stdlib.h>
#include  "config.h"
#include  "MI2_REG.h"
#include  "REG_DRV.h"
#include  "HexFile.h"
#include  "Flash.h"
#include  "debug.h"
#include  "EFM8UB2_helper.h"
#include  "CmdHandler.h"

/* for early tests before MI-2 chip is back */
//#define  DRY_RUN


extern bit g_bFlashWrite;

extern tagFlashRWinfo g_FlashRWinfo;
extern unsigned char xdata g_CmdLineBuf[CMD_LINE_SIZE];


/* wait until WIP (Write In Progress) bit cleared */
void flash_wait_until_WIP_cleared(void)
{
    unsigned char  tmp;
    do
    {
        read_status_enable();
        ReadReg(RX_P0, STATUS_REGISTER, &tmp);
    }while( (tmp & 1) != 0);
}


/* wait until MI-2 Flash controller hardware state machine returns to idle state */
void flash_wait_until_flash_SM_done(void)
{
    unsigned char  tmp;
    do
    {
        ReadReg(RX_P0, R_RAM_CTRL, &tmp);
    }while( ((tmp >> FLASH_DONE) & 1) == 0);
}


/* basic configurations of the Flash controller, and some global variables initialization  */
/* This is not a debug command, but called by PROC_Main(). */
void flash_basic_config(void)
{
    WriteReg(RX_P0, XTAL_FRQ_SEL, (XTAL_FRQ == 27000000UL) ? XTAL_FRQ_27M : ((XTAL_FRQ == 26000000UL) ? XTAL_FRQ_26M : XTAL_FRQ_19M2));
    flash_wait_until_flash_SM_done();
    flash_write_disable();
#ifndef  DRY_RUN
    flash_wait_until_WIP_cleared();
#endif
    flash_wait_until_flash_SM_done();
}


/* Flash write protection range */
#define  FLASH_PROTECTION_ALL  ( (1 << BP4) | (1 << BP3) | (1 << BP2) | (1 << BP1) | (1 << BP0) )

#define  FLASH_PROTECTION_PATTERN_MASK ( (1 << SRP0) | (1 << BP4) | (1 << BP3) | (1 << BP2) | (1 << BP1) | (1 << BP0) )
#define  HW_FLASH_PROTECTION_PATTERN   ( (1 << SRP0) | FLASH_PROTECTION_ALL )
#define  SW_FLASH_PROTECTION_PATTERN   ( (0 << SRP0) | FLASH_PROTECTION_ALL )
/* enable Flash hardware write protection */
void flash_HW_write_protection_enable(void)
{
#ifndef  FLASH_WP_TEST
    unsigned char RegData;

    // WP# pin of Flash die = high, not hardware write protected
    ReadReg(RX_P0, GPIO_CTRL_1, &RegData);
    RegData |= (WRITE_UNPROTECTED << FLASH_WP);
    WriteReg(RX_P0, GPIO_CTRL_1, RegData);

    RegData = HW_FLASH_PROTECTION_PATTERN;
    flash_wait_until_flash_SM_done();
    flash_write_status_register(RegData);
#ifndef  DRY_RUN
    flash_wait_until_WIP_cleared();
#endif

    // WP# pin of Flash die = low, hardware write protected
    ReadReg(RX_P0, GPIO_CTRL_1, &RegData);
    RegData &= ~(WRITE_UNPROTECTED << FLASH_WP);
    WriteReg(RX_P0, GPIO_CTRL_1, RegData);

    flash_wait_until_flash_SM_done();
    read_status_enable();
    ReadReg(RX_P0, STATUS_REGISTER, &RegData);
    flash_wait_until_flash_SM_done();

    if ( (RegData & FLASH_PROTECTION_PATTERN_MASK) == HW_FLASH_PROTECTION_PATTERN )
    {
        TRACE("Flash hardware write protection enabled\n");
    }
    else
    {
        TRACE("Enabling Flash hardware write protection FAILED\n");
    }
#endif
}


/* disable Flash write protection */
void flash_write_protection_disable(void)
{
#ifndef  FLASH_WP_TEST
    unsigned char RegData;

    // WP# pin of Flash die = high, not hardware write protected
    ReadReg(RX_P0, GPIO_CTRL_1, &RegData);
    RegData |= (WRITE_UNPROTECTED << FLASH_WP);
    WriteReg(RX_P0, GPIO_CTRL_1, RegData);

    RegData = 0;
    flash_wait_until_flash_SM_done();
    flash_write_status_register(RegData);
#ifndef  DRY_RUN
    flash_wait_until_WIP_cleared();
#endif
    flash_wait_until_flash_SM_done();
    read_status_enable();
    ReadReg(RX_P0, STATUS_REGISTER, &RegData);
    flash_wait_until_flash_SM_done();

    if ( (RegData & FLASH_PROTECTION_PATTERN_MASK) == 0 )
    {
        TRACE("Flash write protection disabled\n");
    }
    else
    {
        TRACE("Disabling Flash write protection FAILED\n");
    }
#endif
}


/* Flash Chip Erase */
/* Usage: fl_CE */
void command_flash_CE(void)
{
    flash_write_protection_disable();
    flash_chip_erase();
#ifndef  DRY_RUN
    flash_wait_until_WIP_cleared();
#endif
    flash_wait_until_flash_SM_done();
    TRACE("Flash chip erased\n");

    flash_HW_write_protection_enable();
}


/* reads Flash, and checks whether all the bytes are 0xFF */
/* Usage: fl_BC */
void command_flash_BC(void)
{
    unsigned char  ReadDataBuf[MAX_BYTE_COUNT_PER_RECORD_FLASH];
    unsigned char  i;  /* counter */
    unsigned int   Address = 0xFFFF;
    unsigned long  size_to_be_read = 65536;
    unsigned long  error_bytes = 0;
    unsigned char  RegBak1, RegBak2;  // register values back up
    unsigned char  RegVal;  // register value

    ReadReg(TX_P0, SYSTEM, &RegVal);
    RegBak1 = RegVal;
    RegVal &= ~(1 << HDCP2_FW_EN);
    WriteReg(TX_P0, SYSTEM, RegVal);  // stop secure OCM to avoid buffer access conflict

    ReadReg(RX_P0, OCM_DEBUG_REG_8, &RegVal);
    RegBak2 = RegVal;
    RegVal |= (1 << STOP_MAIN_OCM);
    WriteReg(RX_P0, OCM_DEBUG_REG_8, RegVal);  // stop main OCM to avoid buffer access conflict

    flash_wait_until_flash_SM_done();

    WriteReg(RX_P0, FLASH_LEN_H, (FLASH_READ_MAX_LENGTH - 1) >> 8);
    WriteReg(RX_P0, FLASH_LEN_L, (FLASH_READ_MAX_LENGTH - 1) & 0xFF);  // Reads 32 bytes

    while (1)
    {
        WriteReg(RX_P0, FLASH_ADDR_H, Address >> 8);
        WriteReg(RX_P0, FLASH_ADDR_L, Address);

        read_enable();
        flash_wait_until_flash_SM_done();

        /* =============== Reads 32 bytes, but only keeps 16 bytes (offset: 1~16) =============== */
        ReadBlockReg(RX_P0, FLASH_READ_BUF_BASE_ADDR + 1, MAX_BYTE_COUNT_PER_RECORD_FLASH, ReadDataBuf);

        for (i=0; i<MAX_BYTE_COUNT_PER_RECORD_FLASH; i++)
        {
            if (ReadDataBuf[i] != 0xFF)
            {
                error_bytes++;
            }
        }

        Address += MAX_BYTE_COUNT_PER_RECORD_FLASH;
        size_to_be_read -= MAX_BYTE_COUNT_PER_RECORD_FLASH;

        if (size_to_be_read == (unsigned long)0)  // done
        {
            break;
        }
    }

    WriteReg(TX_P0, SYSTEM, RegBak1);  // restore register value
    WriteReg(RX_P0, OCM_DEBUG_REG_8, RegBak2);  // restore register value
    TRACE1("Blank check completed. %lu non-0xFF bytes\n", error_bytes);
}


/* Flash Sector Erase */
/* Usage: fl_SE <address> */
void command_flash_SE(void)
{
    unsigned int Flash_Addr;  /* Flash address, any address inside the sector is a valid address for the Sector Erase (SE) command*/

    if(sscanf(g_CmdLineBuf, "\\%*s %x", &Flash_Addr) == 1)
    {
        flash_write_protection_disable();
        flash_sector_erase(Flash_Addr);
#ifndef  DRY_RUN
        flash_wait_until_WIP_cleared();
#endif
        flash_wait_until_flash_SM_done();
        TRACE2("Sector erase done: 0x%04X ~ 0x%04X\n", (Flash_Addr >> 12) * FLASH_SECTOR_SIZE,
               ( (Flash_Addr + FLASH_SECTOR_SIZE) >> 12) * FLASH_SECTOR_SIZE - 1);
        flash_HW_write_protection_enable();
    }
    else
    {
        TRACE("Bad parameter! Usage:\n");
        TRACE("fl_SE <address>\n");
    }
}


/* Erase OCM firmware */
void command_erase_FW(bit FW_ID)
{
    unsigned int Flash_Addr;  /* Flash address, any address inside the sector is a valid address for the Sector Erase (SE) command*/

    flash_write_protection_disable();

    if (FW_ID)
    {
        for (Flash_Addr = SECURE_OCM_FW_ADDR_BASE; Flash_Addr <= SECURE_OCM_FW_ADDR_END; Flash_Addr += FLASH_SECTOR_SIZE)
        {
            flash_sector_erase(Flash_Addr);
#ifndef  DRY_RUN
            flash_wait_until_WIP_cleared();
#endif
            flash_wait_until_flash_SM_done();
        }
        TRACE("Secure");
    }
    else
    {
        for (Flash_Addr = MAIN_OCM_FW_ADDR_BASE; Flash_Addr <= MAIN_OCM_FW_ADDR_END; Flash_Addr += FLASH_SECTOR_SIZE)
        {

            flash_sector_erase(Flash_Addr);
#ifndef  DRY_RUN
            flash_wait_until_WIP_cleared();
#endif
            flash_wait_until_flash_SM_done();
        }
        TRACE("Main");
    }

    TRACE(" OCM FW erased\n");


    flash_HW_write_protection_enable();
}


void HEX_file_validity_check(unsigned int Address, unsigned char ByteCount, char return_code)
{
    if (return_code != 0)
    {
        TRACE2("HEX file err! Addr=0x%04X, err=%bd\n", Address, return_code);
        TRACE("Power cycle EVB and check HEX file\n");
        while(1);  /* hangs deliberately so that the user can see it */
    }

    if (ByteCount > MAX_BYTE_COUNT_PER_RECORD_FLASH)
    {
        TRACE2("ERR! ByteCount=%bu>%bu\n", ByteCount, MAX_BYTE_COUNT_PER_RECORD_FLASH);
        TRACE("Power cycle EVB and check HEX file\n");
        while(1);  /* hangs deliberately so that the user can see it */
    }

    if ( (Address % MAX_BYTE_COUNT_PER_RECORD_FLASH) != 0 )
    {
        TRACE2("ERR! Addr=0x%04X, not %bu bytes aligned\n", Address, MAX_BYTE_COUNT_PER_RECORD_FLASH);
        TRACE("Power cycle EVB and check HEX file\n");
        while(1);  /* hangs deliberately so that the user can see it */
    }
}


void flash_write_prepare(unsigned int Address, unsigned char offset, unsigned char ByteCount, unsigned char *WriteDataBuf)
{
    flash_write_enable();

    WriteReg(RX_P0, FLASH_ADDR_H, Address >> 8);
    WriteReg(RX_P0, FLASH_ADDR_L, Address);

    WriteBlockReg(RX_P0, FLASH_WRITE_BUF_BASE_ADDR + offset, ByteCount, WriteDataBuf);
}


void flash_actual_write(void)
{
#ifndef  DRY_RUN
    flash_wait_until_WIP_cleared();  /* This is waiting for previous flash_write_enable() is done, i.e. writing status register is done */
    write_enable();
    flash_wait_until_WIP_cleared();
#endif

    flash_wait_until_flash_SM_done();
}


/* program Flash with data in a HEX file, 32 bytes at a time (in some special cases, 16 bytes at a time) */
/* When the address is not 16-byte aligned, simply notify the user and hangs deliberately */
/* The address is always 16-byte aligned, and 16 or 32 bytes are written into Flash at a time, */
/* with some special handling, */
/* crossing 256-byte (Flash page size) boundary will NEVER happen, thus no need to handle this. */
/* Note: This function only programs the Flash; it does NOT automatically erase Flash or even backup the data. */
/* Erasing Flash properly is the user's responsiblity. */
void flash_program(void)
{
    unsigned char WriteDataBuf[MAX_BYTE_COUNT_PER_RECORD_FLASH];
    unsigned char ByteCount;
    unsigned int  Address;
    unsigned char RecordType;
    unsigned char i;  /* counter */
    unsigned char  RegVal;  // register value
    char  return_code = 0;

    ReadReg(TX_P0, SYSTEM, &RegVal);
    RegVal &= ~(1 << HDCP2_FW_EN);
    WriteReg(TX_P0, SYSTEM, RegVal);  // stop secure OCM to avoid buffer access conflict

    ReadReg(RX_P0, OCM_DEBUG_REG_8, &RegVal);
    RegVal |= (1 << STOP_MAIN_OCM);
    WriteReg(RX_P0, OCM_DEBUG_REG_8, RegVal);  // stop main OCM to avoid buffer access conflict

    flash_wait_until_flash_SM_done();

    WriteReg(RX_P0, FLASH_LEN_H, (FLASH_WRITE_MAX_LENGTH - 1) >> 8);
    WriteReg(RX_P0, FLASH_LEN_L, (FLASH_WRITE_MAX_LENGTH - 1) & 0xFF);

    /* ================================ Ping: accumulates data ================================ */
    if (g_FlashRWinfo.prog_is_Ping)
    {
        return_code = GetLineData(g_CmdLineBuf, &ByteCount, &Address, &RecordType, WriteDataBuf);
        HEX_file_validity_check(Address, ByteCount, return_code);

        if (RecordType == HEX_RECORD_TYPE_EOF)   /* end of HEX file */
        {
            g_bFlashWrite = 0;
            TRACE1("\n\n%lu bytes written\n", g_FlashRWinfo.total_bytes_written);
            TRACE("EVB power cycle is REQUIRED before Flash readback and verify.\n");

            flash_HW_write_protection_enable();
            reset_MI2();
            return;
        }

write_prepare_in_ping:
        flash_write_prepare(Address, (unsigned char)0, ByteCount, WriteDataBuf);
        g_FlashRWinfo.previous_addr = Address;

        g_FlashRWinfo.bytes_accumulated_in_Ping = ByteCount;
        g_FlashRWinfo.prog_is_Ping = 0;

        if ( (Address % FLASH_WRITE_MAX_LENGTH) != 0 ) // We're now in ping, but we have to do something that is normally done in pong (the Address dictates this),
        {                                              // so that we can recover the ping-pong cadence.
            for (i=0; i<MAX_BYTE_COUNT_PER_RECORD_FLASH; i++)
            {
                WriteReg(RX_P0, FLASH_WRITE_BUF_BASE_ADDR + i, 0xFF);
            }
            flash_write_prepare(Address - MAX_BYTE_COUNT_PER_RECORD_FLASH, (unsigned char)MAX_BYTE_COUNT_PER_RECORD_FLASH, ByteCount, WriteDataBuf);
            flash_actual_write();
            g_FlashRWinfo.total_bytes_written += ByteCount;
            g_FlashRWinfo.bytes_accumulated_in_Ping = 0;
            g_FlashRWinfo.prog_is_Ping = 1;
            g_FlashRWinfo.previous_addr = Address;
        }
        return;
    }

    /* ================================ Pong: program Flash ================================ */
    /* note: GetLineData() can ONLY be called ONCE per flash_program() invoke, */
    /* otherwise serial port buffer has no chance to be updated, and the same HEX record is used twice, */
    /* which is incorrect. */
    if (!g_FlashRWinfo.prog_is_Ping)
    {
        return_code = GetLineData(g_CmdLineBuf, &ByteCount, &Address, &RecordType, WriteDataBuf);
        HEX_file_validity_check(Address, ByteCount, return_code);

        if (RecordType == HEX_RECORD_TYPE_EOF)   /* end of HEX file */
        {
            for (i=0; i<MAX_BYTE_COUNT_PER_RECORD_FLASH; i++)
            {
                WriteReg(RX_P0, FLASH_WRITE_BUF_BASE_ADDR + MAX_BYTE_COUNT_PER_RECORD_FLASH + i, 0xFF);
            }
            flash_actual_write();
            g_FlashRWinfo.total_bytes_written += g_FlashRWinfo.bytes_accumulated_in_Ping;
            g_bFlashWrite = 0;
            TRACE1("\n\n%lu bytes written\n", g_FlashRWinfo.total_bytes_written);
            TRACE("EVB power cycle is REQUIRED before Flash readback and verify.\n");

            flash_HW_write_protection_enable();
            reset_MI2();
            return;
        }

        if ( ((Address % FLASH_WRITE_MAX_LENGTH) != 0) && (Address == g_FlashRWinfo.previous_addr + MAX_BYTE_COUNT_PER_RECORD_FLASH) )
        {         // contiguous address
            WriteBlockReg(RX_P0, FLASH_WRITE_BUF_BASE_ADDR + g_FlashRWinfo.bytes_accumulated_in_Ping, ByteCount, WriteDataBuf);
            flash_actual_write();
            g_FlashRWinfo.total_bytes_written += (g_FlashRWinfo.bytes_accumulated_in_Ping + ByteCount);
            g_FlashRWinfo.bytes_accumulated_in_Ping = 0;
            g_FlashRWinfo.previous_addr = Address;
            g_FlashRWinfo.prog_is_Ping = 1;
        }
        else if ( ((Address % FLASH_WRITE_MAX_LENGTH) != 0) && (Address != g_FlashRWinfo.previous_addr + MAX_BYTE_COUNT_PER_RECORD_FLASH) )
        {          // address is not contiguous
            for (i=0; i<MAX_BYTE_COUNT_PER_RECORD_FLASH; i++)
            {
                WriteReg(RX_P0, FLASH_WRITE_BUF_BASE_ADDR + MAX_BYTE_COUNT_PER_RECORD_FLASH + i, 0xFF);
            }
            flash_write_enable();
            WriteReg(RX_P0, FLASH_ADDR_H, g_FlashRWinfo.previous_addr >> 8);
            WriteReg(RX_P0, FLASH_ADDR_L, g_FlashRWinfo.previous_addr & 0xFF);
            flash_actual_write();  // write what was received in ping
            g_FlashRWinfo.total_bytes_written += g_FlashRWinfo.bytes_accumulated_in_Ping;
            g_FlashRWinfo.bytes_accumulated_in_Ping = 0;

            for (i=0; i<MAX_BYTE_COUNT_PER_RECORD_FLASH; i++)
            {
                WriteReg(RX_P0, FLASH_WRITE_BUF_BASE_ADDR + i, 0xFF);
            }
            flash_write_prepare(Address - MAX_BYTE_COUNT_PER_RECORD_FLASH, (unsigned char)MAX_BYTE_COUNT_PER_RECORD_FLASH, ByteCount, WriteDataBuf);
            flash_actual_write();  // write what is received in this pong
            g_FlashRWinfo.total_bytes_written += ByteCount;
            g_FlashRWinfo.previous_addr = Address;
            g_FlashRWinfo.prog_is_Ping = 1;
        }
        else if ( ((Address % FLASH_WRITE_MAX_LENGTH) == 0) && (Address != g_FlashRWinfo.previous_addr + MAX_BYTE_COUNT_PER_RECORD_FLASH) )
        {
            for (i=0; i<MAX_BYTE_COUNT_PER_RECORD_FLASH; i++)
            {
                WriteReg(RX_P0, FLASH_WRITE_BUF_BASE_ADDR + MAX_BYTE_COUNT_PER_RECORD_FLASH + i, 0xFF);
            }
            flash_write_enable();
            WriteReg(RX_P0, FLASH_ADDR_H, g_FlashRWinfo.previous_addr >> 8);
            WriteReg(RX_P0, FLASH_ADDR_L, g_FlashRWinfo.previous_addr & 0xFF);
            flash_actual_write();  // write what was received in ping
            g_FlashRWinfo.total_bytes_written += g_FlashRWinfo.bytes_accumulated_in_Ping;
            g_FlashRWinfo.bytes_accumulated_in_Ping = 0;
            goto write_prepare_in_ping;
        }
        else
        {
            TRACE("Internal ERR in flash_program\n");
            TRACE("Power cycle EVB and check HEX file\n");
            while(1);  /* hangs deliberately so that the user can see it */
        }
    }
}


/* Flash programming preparation */
void burn_hex_prepare(void)
{
    g_bFlashWrite = 1;
    g_FlashRWinfo.total_bytes_written = 0;
    g_FlashRWinfo.prog_is_Ping = 1;
    flash_write_protection_disable();
    TRACE("You may send the HEX file now. SecureCRT -> Transfer -> Send ASCII ...\n");
    TRACE("Please make sure line send delay (SecureCRT -> Options -> Session Options -> Terminal\n");
    TRACE("-> Emulation -> Advanced -> Line Send Delay) is set to long enough (MI-2: at least 5ms for 1MHz I2C)\n");
    TRACE("before you select HEX file and transfer it.\n");
}


/* reads Flash, and prints the data on the UART console, so that the data can be saved in a HEX file */
/* Software workaround to deal with issue MIS2-87 (The first byte is wrong in every 32 bytes) has been applied. */
/* Usage: readhex <base_address> <size_to_be_read> */
void command_flash_read(void)
{
    unsigned char  ReadDataBuf[MAX_BYTE_COUNT_PER_RECORD_FLASH];
    unsigned char  i;  /* counter */
    unsigned int   Address;
    unsigned char  Addr_adjust;
    unsigned long  size_to_be_read;
    unsigned long  total_bytes_read = 0;
    unsigned char  checksum;
    unsigned char  RegBak1, RegBak2;  // register values back up
    unsigned char  RegVal;  // register value

    if (sscanf(g_CmdLineBuf, "\\%*s %x %lx", &Address, &size_to_be_read) == 2)
    {
        if (Address%MAX_BYTE_COUNT_PER_RECORD_FLASH != 0)
        {
            TRACE2("ERR! Addr=0x%04X, not %bu bytes aligned\n", Address, MAX_BYTE_COUNT_PER_RECORD_FLASH);
            return;
        }

        ReadReg(TX_P0, SYSTEM, &RegVal);
        RegBak1 = RegVal;
        RegVal &= ~(1 << HDCP2_FW_EN);
        WriteReg(TX_P0, SYSTEM, RegVal);  // stop secure OCM to avoid buffer access conflict

        ReadReg(RX_P0, OCM_DEBUG_REG_8, &RegVal);
        RegBak2 = RegVal;
        RegVal |= (1 << STOP_MAIN_OCM);
        WriteReg(RX_P0, OCM_DEBUG_REG_8, RegVal);  // stop main OCM to avoid buffer access conflict

        flash_wait_until_flash_SM_done();

        WriteReg(RX_P0, FLASH_LEN_H, (FLASH_READ_MAX_LENGTH - 1) >> 8);
        WriteReg(RX_P0, FLASH_LEN_L, (FLASH_READ_MAX_LENGTH - 1) & 0xFF);  // Reads 32 bytes

        if (size_to_be_read < MAX_BYTE_COUNT_PER_RECORD_FLASH)
        {
            goto  THE_LAST_READ;
        }

        while (1)
        {
            if (Address != 0)
            {
                Addr_adjust = 1;
            }
            else
            {
                Addr_adjust = 0;
            }

            WriteReg(RX_P0, FLASH_ADDR_H, (Address - Addr_adjust) >> 8);
            WriteReg(RX_P0, FLASH_ADDR_L, (Address - Addr_adjust));

            read_enable();
            flash_wait_until_flash_SM_done();

            /* =============== Reads 32 bytes, but only keeps 16 bytes (due to silicon bug MIS2-87) =============== */
            ReadBlockReg(RX_P0, FLASH_READ_BUF_BASE_ADDR + Addr_adjust, MAX_BYTE_COUNT_PER_RECORD_FLASH, ReadDataBuf);

            TRACE3(":%02BX%04X%02BX", MAX_BYTE_COUNT_PER_RECORD_FLASH, Address, HEX_RECORD_TYPE_DATA);

            checksum = MAX_BYTE_COUNT_PER_RECORD_FLASH + (Address >> 8) + (Address & 0xFF) + HEX_RECORD_TYPE_DATA;
            for (i=0; i<MAX_BYTE_COUNT_PER_RECORD_FLASH; i++)
            {
                TRACE1("%02BX", ReadDataBuf[i]);
                checksum += ReadDataBuf[i];
            }
            TRACE1("%02BX\n", -checksum);

            Address += MAX_BYTE_COUNT_PER_RECORD_FLASH;
            total_bytes_read += MAX_BYTE_COUNT_PER_RECORD_FLASH;
            size_to_be_read  -= MAX_BYTE_COUNT_PER_RECORD_FLASH;

            if (size_to_be_read < MAX_BYTE_COUNT_PER_RECORD_FLASH)  // the last read
            {
THE_LAST_READ:
                if (size_to_be_read > 0)
                {
                    if (Address != 0)
                    {
                        Addr_adjust = 1;
                    }
                    else
                    {
                        Addr_adjust = 0;
                    }

                    WriteReg(RX_P0, FLASH_ADDR_H, (Address - Addr_adjust) >> 8);
                    WriteReg(RX_P0, FLASH_ADDR_L, (Address - Addr_adjust));

                    read_enable();
                    flash_wait_until_flash_SM_done();

                    ReadBlockReg(RX_P0, FLASH_READ_BUF_BASE_ADDR + Addr_adjust, size_to_be_read, ReadDataBuf);
                    TRACE3(":%02BX%04X%02BX", (unsigned char)size_to_be_read, Address, HEX_RECORD_TYPE_DATA);

                    checksum = size_to_be_read + (Address >> 8) + (Address & 0xFF) + HEX_RECORD_TYPE_DATA;
                    for (i=0; i<size_to_be_read; i++)
                    {
                        TRACE1("%02BX", ReadDataBuf[i]);
                        checksum += ReadDataBuf[i];
                    }
                    TRACE1("%02BX\n", -checksum);

                    total_bytes_read += size_to_be_read;
                    // no need to update size_to_be_read anymore
                }

                // write EOF record
                TRACE3(":%02BX%04X%02BX", (unsigned char)0, (unsigned int)0, HEX_RECORD_TYPE_EOF);
                checksum = 0 + (0 >> 8) + (0 & 0xFF) + HEX_RECORD_TYPE_EOF;
                TRACE1("%02BX\n", -checksum);

                TRACE1("\n\n%lu bytes read\n", total_bytes_read);
                break;
            }
        }

        WriteReg(TX_P0, SYSTEM, RegBak1);  // restore register value
        WriteReg(RX_P0, OCM_DEBUG_REG_8, RegBak2);  // restore register value
    }
    else
    {
        TRACE("Bad parameter! Usage:\n");
        TRACE("readhex <base_address> <size_to_be_read>\n");
    }
}

