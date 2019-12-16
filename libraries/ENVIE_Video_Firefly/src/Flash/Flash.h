/* file: Flash.h
 * description: Mississippi-2 (ANX7625) stack die Flash
 *              access routine prototypes
 * note: This file only contains Flash access routine prototypes
 *       which will be called from higher level software
*/

#ifndef __FLASH_H__
#define __FLASH_H__


typedef struct
{
    unsigned long  total_bytes_written;
    unsigned int   previous_addr;
    unsigned char  prog_is_Ping;  // Ping-Pong flag
    unsigned char  bytes_accumulated_in_Ping;
} tagFlashRWinfo;


//#define  FLASH_WP_TEST   /* Flash write protection test */

/* default value is 0 */
/* Flash SPI controller debug register, use default value is recommended. */
#define READ_DELAY_SELECT_VALUE  0

#define  FLASH_SECTOR_SIZE     (4 * 1024)

// FW_ID (firmware ID)
#define  MAIN_OCM     0
#define  SECURE_OCM   1

// volatile_ID
#define  NON_VOLATILE  0
#define  VOLATILE      1

// Firmware address
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
          WriteReg(RX_P0, R_FLASH_RW_CTRL, (READ_DELAY_SELECT_VALUE << READ_DELAY_SELECT) | (1 << GENERAL_INSTRUCTION_EN))

#define erase_enable() \
          WriteReg(RX_P0, R_FLASH_RW_CTRL, (READ_DELAY_SELECT_VALUE << READ_DELAY_SELECT) | (1 << FLASH_ERASE_EN))

#define write_status_enable() \
          WriteReg(RX_P0, R_FLASH_RW_CTRL, (READ_DELAY_SELECT_VALUE << READ_DELAY_SELECT) | (1 << WRITE_STATUS_EN))

#define read_enable() \
          WriteReg(RX_P0, R_FLASH_RW_CTRL, (READ_DELAY_SELECT_VALUE << READ_DELAY_SELECT) | (1 << FLASH_READ))

#define write_enable() \
          WriteReg(RX_P0, R_FLASH_RW_CTRL, (READ_DELAY_SELECT_VALUE << READ_DELAY_SELECT) | (1 << FLASH_WRITE))

#define erase_type(type) \
          WriteReg(RX_P0, FLASH_ERASE_TYPE, type)

#define flash_address(addr) \
          do{WriteReg(RX_P0, FLASH_ADDR_L, addr); WriteReg(RX_P0, FLASH_ADDR_H, addr>>8);}while(0)

#define read_status_enable() \
          do{unsigned char tmp; ReadReg(RX_P0, R_DSC_CTRL_0, &tmp); tmp |= (1<<READ_STATUS_EN); \
             WriteReg(RX_P0, R_DSC_CTRL_0, tmp);}while(0)

#define flash_write_enable()   do{write_general_instruction(WRITE_ENABLE); general_instruction_enable();}while(0)

#define flash_write_disable()  do{write_general_instruction(WRITE_DISABLE); general_instruction_enable();}while(0)

#define flash_write_enable_volatile_SR()   do{write_general_instruction(WRITE_ENABLE_VOLATILE_SR); general_instruction_enable();}while(0)

#define flash_write_status_register(value)  do{flash_write_enable(); write_status_register(value); write_status_enable();}while(0)

#define flash_write_status_register_volatile(value)  do{flash_write_enable_volatile_SR(); write_status_register(value); write_status_enable();}while(0)

#define flash_chip_erase()  do{flash_write_enable(); write_general_instruction(CHIP_ERASE_A); general_instruction_enable();}while(0)

#define flash_sector_erase(addr)     do{flash_write_enable(); flash_address(addr); erase_type(SECTOR_ERASE);    erase_enable();}while(0)


void flash_basic_config(void);      // basic configurations of the Flash controller
void flash_wait_until_WIP_cleared(void);     // wait until WIP (Write In Progress) bit cleared
void flash_wait_until_flash_SM_done(void);  // wait until MI-2 Flash controller hardware state machine returns to idle state
void flash_HW_write_protection_enable(void);  // enable Flash hardware write protection
void flash_write_protection_disable(void);       // disable Flash write protection
void command_flash_SE(void);        // Sector Erase
void command_erase_FW(bit FW_ID);   // Erase OCM firmware
void command_flash_CE(void);        // Chip Erase
void command_flash_BC(void);        // Blank Check
void command_flash_read(void);      // reads Flash, and saves the data into a HEX file - user interface
void flash_program(void);        // program Flash with data in a HEX file
void burn_hex_prepare(void);   // Flash programming preparation

#ifdef  FLASH_WP_TEST
void flash_WP_tests(void);   // Flash write protection tests
#endif


#endif  /* __FLASH_H__ */

