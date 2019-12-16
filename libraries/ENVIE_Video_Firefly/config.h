/******************************************************************************

Copyright (c) 2016, Analogix Semiconductor, Inc.

PKG Ver  : V0.1

Filename :

Project  : ANX7625

Created  : 20 Sept. 2016

Devices  : ANX7625

Toolchain: Keil

Description:

Revision History:

******************************************************************************/

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "defs.h"
#include "SI_EFM8UB2_Defs.h"

/* ========================================================================== */
// EFM8UB20F64G MCU pin configuration, which is board dependent
sbit RESET_N = P1^1;
sbit POWER_EN = P1^2;

sbit PWR_CTRL_V10 = P0^0;  // controls AVDD10, DVDD10L, and DVDD10G
sbit PWR_CTRL_V18 = P0^1;  // controls AVDD18 and DVDD18
sbit PWR_CTRL_V30 = P2^5;  // controls AVDD30 (ANX7625) or AVDD3V3 (ANX7688)

sbit ALERT_N   = P0^6;
sbit CABLE_DET = P1^3;

sbit SDA = P1^6; // I2C master 0 is on P1.6 and P1.7, which is connected to ANX7625
sbit SCL = P1^7;

sbit CLK_SRC_DET = P2^2;  // enable NBC12429 or not
sbit LED_D_EXM   = P2^3;
sbit SW5_1       = P2^6;
sbit SW5_2       = P1^4;
sbit SW5_3       = P1^0;
sbit SW5_4       = P0^7;

sbit VBUS_DISCHARGE_CRTL_N = P2^4;  // 0: enable discharge; 1: disable discharge
sbit AP_VBUS_CTRL = P2^7;

sbit M_bit8 = P3^4;
#define  M_bits7_0  P4  // note: P4 port is not bit-addressable.

sbit N0 = P3^5;
sbit N1 = P3^6;

sbit P_LOAD_N = P3^7;  // active low P_LOAD input of NBC12429
// END of EFM8UB20F64G MCU pin configuration, which is board dependent
/* ========================================================================== */

// macros for EFM8UB20F64G MCU
#define BAUDRATE    115200UL // baud rate in bps
#define US_PER_CHARACTER ((float)1000000 * (1 + 8 + 1) / BAUDRATE * 2)
#define TMR1_RELOAD     (unsigned char)(256 - (float)SYSCLK / 2 / BAUDRATE + 0.5)

// macro TMR2_RELOAD controls I2C timing
// SCL frequency = (48000 / 3 / (256 - TMR2_RELOAD)) kHz
/* Timer2 reload value @48MHz
 * 0x00 -   62.5 kHz
 * 0x20 -   71.4 kHz
 * 0x40 -   83.3 kHz
 * 0x60 -  100 kHz  (verified on MI-2 EVB)
 * 0x70 -  111.1 kHz
 * 0x80 -  125 kHz
 * 0x90 -  142.9 kHz
 * 0xA0 -  166.7 kHz
 * 0xB0 -  200 kHz
 * 0xC0 -  250 kHz
 * 0xD0 -  333.3 kHz
 * 0xD8 -  400 kHz  (verified on MI-2 EVB)
 * 0xE0 -  500 kHz
 * 0xE1 -  516.1 kHz
 * 0xE2 -  533.3 kHz
 * 0xE3 -  551.7 kHz
 * 0xE4 -  571.4 kHz
 * 0xE5 -  592.6 kHz
 * 0xE6 -  615.4 kHz
 * 0xE7 -  640 kHz
 * 0xE8 -  666.7 kHz (MI-1 fails at this SCL frequency)
 * 0xEC -  800 kHz
 * 0xEE -  888.9 kHz
 * 0xF0 - 1000 kHz  (verified on MI-2 EVB)
 * 0xF1 - 1066.7 kHz  (verified on MI-2 EVB)
 * 0xF2 - 1142.9 kHz  (Measured SCL frequency with Corelis I2C analyzer BusPro-I: 688kHz, possibly caused by clock stretching)
 * 0xF4 - 1333.3 kHz  (Measured SCL frequency with Corelis I2C analyzer BusPro-I: 802kHz, possibly caused by clock stretching)
 */
#define TMR2_RELOAD  0xF0    // SMBus frequency
#define mdelay(t)        DELAY_US(1000*t)

/* ================= board dependent macros ============================== */
// 1. General control logic
#define IS_ALERT()  (!ALERT_N)

#define LED_D_EXM_ON()     LED_D_EXM = 0
#define LED_D_EXM_OFF()    LED_D_EXM = 1

// 2. I2C addresses of LED display driver MAX6958 and battery gas gauge LTC2943
#define MAX6958_ADDR 0x70
#define LTC2943_ADDR 0xC8

// 3. Silicon Labs EFM8UB2 MCU related
//    a) serial communication interface
#define CHECK_TRANSMIT_INTERRUPT()  (SCON0_TI)
#define CHECK_RECEIVE_INTERRUPT()   (SCON0_RI)
#define CLEAR_TRANSMIT_INTERRUPT()  (SCON0_TI = 0)
#define CLEAR_RECEIVE_INTERRUPT()   (SCON0_RI = 0)
#define TRIGGER_TRANSMIT()          (SCON0_TI = 1)
#define WRITE_SBUF(c)               (SBUF0 = c)
#define READ_SBUF()                 (SBUF0)

//    b) serial communication buffer
#define SERIAL_RECV_BUF_SIZE    (256 + 256)
#define SERIAL_SEND_BUF_SIZE    (256 + 256)
#define CMD_LINE_SIZE           (128)
#define CMD_NAME_SIZE           (16)

// 4. system board configuration
#define SYSCLK  48000000UL  // System clock frequency in Hz, 48MHz
#define XTAL_FRQ  27000000UL  // MI-2 clock frequency in Hz: 27 MHz
#define XTAL_FRQ_MHz  ((unsigned char)(XTAL_FRQ/1000000UL))      // MI-2 clock frequency in MHz

#define POWERCYCLE_DELAY      250  // in miliseconds
#define RESET_RELEASE_DELAY    50  // in miliseconds

/* note: On POWER_EN and RESET_N pins, there's an inverter on the EVB. */
#define ANX7625_POWER_ON()      POWER_EN = 0
#define ANX7625_POWER_DOWN()    POWER_EN = 1
#define ANX7625_RESET()          RESET_N = 1
#define ANX7625_RESET_RELEASE()  RESET_N = 0

#define ENABLE_5V_VBUS_OUT()      AP_VBUS_CTRL = 0
#define DISABLE_5V_VBUS_OUT()     AP_VBUS_CTRL = 1

#define ENABLE_5to20V_VBUS_IN()   AP_VBUS_CTRL = 1
#define DISABLE_5to20V_VBUS_IN()  AP_VBUS_CTRL = 0


/* ========================================================================== */
// I2C addresses of Analogix's chip modules, which is chip dependent
// TODO: update this when register spec is ready
#define TCPC_I2C_ADDR    (0x58)
#define USBC_I2C_ADDR    (0x50)
// END of I2C addresses of Analogix's chip modules, which is chip dependent
/* ========================================================================== */


// misc macros which are bringup program dependent
#define  MAX_BYTE_COUNT_PER_RECORD_FLASH    16


////////////////////////////////////////////////////////////////////////////////
// general timer
#define CYCLE_TIMER0(us)    (unsigned int)(65536UL - (float)SYSCLK / 48 / 1000000UL * (us) + 0.5)
#define START_TIMER0(us)    do{TL0 =  CYCLE_TIMER0(us) % 256; TH0 =  CYCLE_TIMER0(us) / 256; TCON_TR0 = 1;}while(0)
#define DELAY_US(t)         do{START_TIMER0(t); while(!TCON_TF0); TCON_TR0 = 0; TCON_TF0 = 0;}while(0)
#define STOP_TIMER0()       TCON_TR0 = 0
#define CHECK_TIMER0()      (TCON_TF0)
#define CLEAR_TIMER0()      TCON_TF0 = 0
#define STOP_AND_CLEAR_TIMER0() do{TCON_TR0 = 0; TCON_TF0 = 0;}while(0)

//#define START_TIMER2(us)    do{TMR2 = (unsigned int)(65536UL - (float)SYSCLK / 12 / 1000000UL * us + 0.5); TMR2CN0_TR2 = 1;}while(0)
//#define START_TIMER2_N(us)  do{TMR2RL = (unsigned int)(65536UL - (float)SYSCLK / 12 / 1000000UL * us + 0.5); TMR2 = TMR2RL; TMR2CN0_TR2 = 1;}while(0)
//#define STOP_TIMER2()       do{TMR2CN0_TR2 = 0; TMR2CN0_TF2H = 0;}while(0)
//#define CHECK_TIMER2()      (TMR2CN0_TF2H)
//#define CLEAR_TIMER2()      (TMR2CN0_TF2H = 0)
//#define DELAY_US(t)         do{START_TIMER2(t); while(TMR2CN0_TF2H == 0); TMR2CN0_TF2H = 0; TMR2CN0_TR2 = 0;}while(0)

#define START_TIMER3_N(us)  do{TMR3RL = (unsigned int)(65536UL - (float)SYSCLK / 12 / 1000000UL * us + 0.5); TMR3 = TMR3RL; TMR3CN0 |= 1<<TMR3CN_TR3;}while(0)
#define CHECK_TIMER3()      BIT_IS_0(TMR3CN0, TMR3CN_TF3H)
#define CLEAR_TIMER3()      BITCLEAR(TMR3CN0, TMR3CN_TF3H)
#define STOP_TIMER3()       BITCLEAR(TMR3CN0, TMR3CN_TR3)
#define STOP_AND_CLEAR_TIMER3()       do{TMR3CN0 &= ~(BITMASK(TMR3CN_TF3H) | BITMASK(TMR3CN_TR3));}while(0)

//#define CYCLE_PCA_TIMER0(us)  (unsigned int)(65536UL - (float)SYSCLK / 12 / 1000000UL * (us) + 0.5)
//#define START_PCA_TIMER0(us)  do{PCA0 = CYCLE_PCA_TIMER0(us); PCA0CN0_CR = 1;}while(0)
//#define CHECK_PCA_TIMER0()    (PCA0CN0_CF)
//#define CLEAR_PCA_TIMER0()    (PCA0CN0_CF = 0)
//#define STOP_AND_CLEAR_PCA_TIMER0() do{PCA0CN0_CR = 0; PCA0CN0_CF = 0;}while(0)

#define CYCLE_PCA_TIMER(us)         (unsigned int)((float)SYSCLK / 12 / 1000000UL * (us) + 0.5)
// PCA timer0 is used in SOP' message transaction
#define CHECK_PCA_TIMER0()          (PCA0CN0_CCF0)
#define CLEAR_PCA_TIMER0()          (PCA0CN0_CCF0 = 0)
#define RELOAD_PCA_TIMER0(us)       do{PCA0CP0 += CYCLE_PCA_TIMER(us);}while(0)
#define RELOAD_AND_CLEAR_PCA_TIMER0(us)       do{PCA0CP0 += CYCLE_PCA_TIMER(us); PCA0CN0_CCF0 = 0;}while(0)

// PCA timer1 is used in LED D_EXM and ADC
#define CHECK_PCA_TIMER1()          (PCA0CN0_CCF1)
#define CLEAR_PCA_TIMER1()          (PCA0CN0_CCF1 = 0)
#define RELOAD_PCA_TIMER1(us)       do{PCA0CP1 += CYCLE_PCA_TIMER(us);}while(0)
#define RELOAD_AND_CLEAR_PCA_TIMER1(us)       do{PCA0CP1 += CYCLE_PCA_TIMER(us); PCA0CN0_CCF1 = 0;}while(0)

// PCA timer2 is used in USBC for CC debounce and PD debounce
#define CHECK_PCA_TIMER2()          (PCA0CN0_CCF2)
#define CLEAR_PCA_TIMER2()          (PCA0CN0_CCF2 = 0)
#define RELOAD_PCA_TIMER2(us)       do{PCA0CP2 += CYCLE_PCA_TIMER(us);}while(0)
#define RELOAD_AND_CLEAR_PCA_TIMER2(us)       do{PCA0CP2 += CYCLE_PCA_TIMER(us); PCA0CN0_CCF2 = 0;}while(0)

#define OFFSET(s, m)    (unsigned char)&(((s code *)0)->m)

#endif  /* __CONFIG_H__ */

