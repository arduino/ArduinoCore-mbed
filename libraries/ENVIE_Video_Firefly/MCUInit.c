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

#include "config.h"
#include "private_interface.h"
#ifdef USE_PD30
#include "pd_ext_message.h"
#endif

/**
  * Init power supply monitor
  */
void Monitor_Init(void)
{
  VDM0CN |= 1 << VDM0CN_VDMEN;
  while( ((VDM0CN >> VDM0CN_VDDSTAT) & 1) == 0 );  // wait for supply monitor output to change to 1
  RSTSRC = 1 << RSTSRC_PORSF;   // enable the supply monitor as a reset source
}


/**
  * This routine initializes the system clock to use the internal oscillator
  * as the system clock. The resulting frequency is 48 MHz.
  */
void SYSCLKInit(void)
{
  HFO0CN |= SYSCLK_DIV_1 << HFO0CN_IFCN;  // Configure internal oscillator for its maximum frequency, SYSCLK = 48MHz
  FLSCL |= SYSCLK_BELOW_48_MHZ << FLSCL_FLRT; // for Prefetch Engine @48MHz
  CLKSEL |= HFOSC << CLKSEL_CLKSL;   // Select HFOSC as the system clock.
  RSTSRC = 1 << RSTSRC_MCDRSF | 1 << RSTSRC_PORSF;   // Enable missing clock detector as a reset source
}


void TimerInit(void)
{
    // Timer 0 use SYSCLK/48, Timer 1 uses SYSCLK, Timer2 low 8-bit uses SYSCLK, Timer3 uses clock defined by T3XCLK in TMR3CN0.
    CKCON0 = T3ML_EXTERNAL_CLOCK<<CKCON0_T3ML | T2ML_SYSCLK<<CKCON0_T2ML | T1M_SYSCLK<<CKCON0_T1M | T0M_PRESCALE<<CKCON0_T0M | SYSCLK_DIV_48<<CKCON0_SCA;
    // timer 0 in Mode 1, 16-bit timer; timer 1 in 8-bit autoreload
    TMOD = T1M_MODE2<<TMOD_T1M | T0M_MODE1<<TMOD_T0M;
    // init Timer1
    TH1 = TMR1_RELOAD % 256;
    TL1 = TMR1_RELOAD % 256;
    IE_ET1 = 1; // enable Timer1 interrupt
    TCON_TR1 = 1; // start Timer1
    // init timer2
    TMR2L = TMR2_RELOAD;
    TMR2RL = TMR2_RELOAD; // Reload value to be used in Timer2
    // Timer2 in split 8-bit auto-reload mode, TMR2L is always running for SMBus0 clock
    TMR2CN0 = 0<<TMR2CN_T2SOF | 1<<TMR2CN_T2SPLIT | 0<<TMR2CN_TR2 | 0<<TMR2CN_T2XCLK; 
    // init timer3
//     TMR3RL = TMR3_RELOAD; // Reload value to be used in Timer3
//     TMR3 = TMR3_RELOAD;
    // Timer3 uses SYSCLK/12, Enable Timer3 in 16-bit auto-reload mode, stop timer3
    TMR3CN0 = 0<<TMR3CN_T3SOF | 0<<TMR3CN_T3SPLIT | 0<<TMR3CN_TR3 | T3XCLK_SYSCLK_DIV_12<<TMR3CN_T3XCLK;
//     ET3 = 1; // Timer3 interrupt enabled
}

/**
  * This function configures the crossbar and GPIO ports.
  * P0.0   digital  push-pull   PWR_CTRL_V10
  * P0.1   digital  push-pull   PWR_CTRL_V18
  * P0.2   digital  push-pull   I2C_SCL
  * P0.3   digital  open-drain  I2C_SDA
  * P0.4   digital  push-pull   TX
  * P0.5   digital  open-drain  RX
  * P0.6   digital  open-drain  ALERT_N
  * P0.7   digital  push-pull   GPIO
  * P1.0   digital  open-drain  MAX6958 display VBUS voltage
  * P1.1   digital  push-pull   RESET_N
  * P1.2   digital  push-pull   POWER_EN
  * P1.3   digital  open-drain  CABLE_DET
  * P1.4   analog   open-drain  NC
  * P1.5   digital  open-drain  USB_ID
  * P1.6   digital  open-drain  SMBus SDA
  * P1.7   digital  open-drain  SMBus SCL
  * P2.0   digital  open-drain  GPIO
  * P2.1   digital  open-drain  GPIO
  * P2.2   digital  open-drain  GPIO
  * P2.3   digital  push-pull   LED D_EXM
  * P2.4   digital  push-pull   VBUS_DISCHARGE_CRTL_N
  * P2.5   digital  push-pull   PWR_CTRL_V30
  * P2.6   digital  push-pull   SW5_1
  * P2.7   digital  push-pull   VBUS_CTRL
  */
void PortInit(void)
{
    P0MDIN = DIGITAL<<P0MDIN_B7 | DIGITAL<<P0MDIN_B6 | DIGITAL<<P0MDIN_B5 | DIGITAL<<P0MDIN_B4 | DIGITAL<<P0MDIN_B3 | DIGITAL<<P0MDIN_B2 | DIGITAL<<P0MDIN_B1 | DIGITAL<<P0MDIN_B0;
    P0MDOUT |= PUSH_PULL<<P0MDOUT_B7 | PUSH_PULL<<P0MDOUT_B4 | PUSH_PULL<<P0MDOUT_B2 | PUSH_PULL<<P0MDOUT_B1 | PUSH_PULL<<P0MDOUT_B0;
    P0SKIP |= SKIPPED<<P0SKIP_B7 | SKIPPED<<P0SKIP_B6 | SKIPPED<<P0SKIP_B3 | SKIPPED<<P0SKIP_B2 | SKIPPED<<P0SKIP_B1 | SKIPPED<<P0SKIP_B0;
    
    P1MDIN = DIGITAL<<P1MDIN_B7 | DIGITAL<<P1MDIN_B6 | DIGITAL<<P1MDIN_B5 | DIGITAL<<P1MDIN_B4 | DIGITAL<<P1MDIN_B3 | DIGITAL<<P1MDIN_B2 | DIGITAL<<P1MDIN_B1 | DIGITAL<<P1MDIN_B0;
    P1MDOUT |= PUSH_PULL<<P1MDOUT_B2 | PUSH_PULL<<P1MDOUT_B1;
    P1SKIP |= SKIPPED<<P1SKIP_B5 | SKIPPED<<P1SKIP_B4 | SKIPPED<<P1SKIP_B3 | SKIPPED<<P1SKIP_B2 | SKIPPED<<P1SKIP_B1 | SKIPPED<<P1SKIP_B0;
    
    P2MDIN = DIGITAL<<P2MDIN_B7 | DIGITAL<<P2MDIN_B6 | DIGITAL<<P2MDIN_B5 | DIGITAL<<P2MDIN_B4 | DIGITAL<<P2MDIN_B3 | DIGITAL<<P2MDIN_B2 | DIGITAL<<P2MDIN_B1 | DIGITAL<<P2MDIN_B0;
    P2MDOUT |= PUSH_PULL<<P2MDOUT_B7 | PUSH_PULL<<P2MDOUT_B6 | PUSH_PULL<<P2MDOUT_B5 | PUSH_PULL<<P2MDOUT_B4 | PUSH_PULL<<P2MDOUT_B3;
    P2SKIP = SKIPPED<<P2SKIP_B7 | SKIPPED<<P2SKIP_B6 | SKIPPED<<P2SKIP_B5 | SKIPPED<<P2SKIP_B4 | SKIPPED<<P2SKIP_B3 | SKIPPED<<P2SKIP_B2 | SKIPPED<<P2SKIP_B1 | SKIPPED<<P2SKIP_B0;

    XBR0 = 1<<XBR0_SMB0E | 1<<XBR0_URT0E; // Enable UART and SMBus
    XBR1 = 1<<XBR1_WEAKPUD | 1<<XBR1_XBARE; // Enable crossbar and disable weak pull-ups
}

void UARTInit(void)
{
    SCON0 = SMODE_8_BIT<<_SCON0_SMODE | 1<<_SCON0_REN; // 8-bit UART with Variable Baud Rate, Receive Enable
    IE_ES0 = 1; // Enable UART0 interrupts
}

void SMBusRecover(void)
{
    unsigned char i;
    
    // If slave is holding SDA low because of an improper SMBus reset or error
    while(!SDA)
    {
        // Provide clock pulses to allow the slave to advance out
        // of its current state. This will allow it to release SDA.
        XBR1 = 0x40;            // Enable Crossbar
        SCL = 0;                // Drive the clock low
        for(i = 255; i; i--);   // Hold the clock low
        SCL = 1;                // Release the clock
        while(!SCL);            // Wait for open-drain
                                // clock output to rise
        for(i = 10; i; i--);    // Hold the clock high
        XBR1 = 0x00;            // Disable Crossbar
    }
}

void SMBusInit(void)
{
    // Enable SMBus, disable slave mode, enable setup & hold time extensions, use Timer 2 overflows as SMBus0 clock
    SMB0CF = 1<<SMB0CF_ENSMB | 1<<SMB0CF_INH | 1<<SMB0CF_EXTHOLD | SMBCS_TIMER2_LOW<<SMB0CF_SMBCS;
}

// set PCA0 in 16-Bit Software Timer Mode
void PCAInit(void)
{
    // Configure Watchdog Timer, PCA time base, overflow interrupt; 
    PCA0MD = 0<<PCA0MD_WDTE | PCA0MD_CPS_SYSCLK_DIV_12<<PCA0MD_CPS | 0<<PCA0MD_ECF; // disable watchdog timer, use SYSCLK/12 as time base, disable overflow interrupt
    // Module 0 = Software Timer (Compare) Mode, disable CCF flag Interrupt
    PCA0CPM0 = 1<<PCA0CPM_ECOM | 1<<PCA0CPM_MAT | 0<<PCA0CPM_ECCF;
    // Module 1 = Software Timer (Compare) Mode, disable CCF flag Interrupt
    PCA0CPM1 = 1<<PCA0CPM_ECOM | 1<<PCA0CPM_MAT | 0<<PCA0CPM_ECCF;
    // Module 2 = Software Timer (Compare) Mode, disable CCF flag Interrupt
    PCA0CPM2 = 1<<PCA0CPM_ECOM | 1<<PCA0CPM_MAT | 0<<PCA0CPM_ECCF;
    PCA0CN0_CR = 1; // run PCA0 timer
}

/**
  * This function configures and enables /INT0 (External Interrupts) as negative level-triggered.
  */
void INT0Init(void)
{
    IT01CF = IN0PL_ACTIVE_LOW<<IT01CF_IN0PL | IN0SL_P0_6<<IT01CF_IN0SL; // /INT0 active low and /INT0 on P0.6
    TCON_IT0 = 0; // /INT 0 level triggered
//    IE_EX0 = 1; // Enable /INT0 interrupts
}

void ADCInit(void)
{
    AMX0N = AMX0N_GND; // AMX0N (AMUX0 Negative Input Selection) = GND (Ground (single-ended mode)
    AMX0P = AMX0P_TEMP; // AMX0P (AMUX0 Positive Input Selection) = TEMP (Temperature sensor)
//    ADC0CN0 |= 1 << ADC0CN0_ADEN; // ADEN (ADC Enable) = ENABLED (ADC0 Enabled (active and ready for data conversions)
    // TEMPE (Temperature Sensor Enable) = ENABLED (Enable the internal Temperature Sensor)
    
//    REF0CN = 1 << REF0CN_REGOVR | 1 << REF0CN_TEMPE
}

extern unsigned int xdata on_time_task_timer;

extern unsigned char IF_RAM wait_send_pswap_response_timer;
extern unsigned char IF_RAM wait_send_dswap_response_timer;
extern unsigned char IF_RAM wait_send_gotomin_response_timer;
extern unsigned char IF_RAM wait_send_vswap_response_timer;
extern unsigned char IF_RAM wait_send_rdo_response_timer;

void timer1ms_isr(void) interrupt 3
{
    static unsigned char xdata count =0;

    // share with baud rate timer
    if (count++ < 231) return;
    count = 0;

    if(on_time_task_timer > 0) on_time_task_timer --;
    if(wait_send_pswap_response_timer > 0) wait_send_pswap_response_timer --;
#ifndef DEL_UNUSE_FEATURE
    if(wait_send_dswap_response_timer > 0) wait_send_dswap_response_timer --;
    if(wait_send_gotomin_response_timer > 0) wait_send_gotomin_response_timer --;
#endif
    if(wait_send_vswap_response_timer > 0) wait_send_vswap_response_timer --;
    if(wait_send_rdo_response_timer >0) wait_send_rdo_response_timer--;
#ifdef USE_PD30
	if(PDExtSend_timer > 0) PDExtSend_timer--;
#if USE_PDFU
	if(PDFUResponseRcvd_timer > 0) PDFUResponseRcvd_timer--;
	if(PDFUNextRequestSent_timer > 0) PDFUNextRequestSent_timer--;
	if(PDFUResponseSent_timer > 0) PDFUResponseSent_timer--;
	if(PDFUNextRequestRcvd_timer > 0) PDFUNextRequestRcvd_timer--;
	if(PDFUWait_timer > 0) PDFUWait_timer--;
#endif
#endif
}

