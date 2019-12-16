// Note: This header file is copied from
// C:\Keil_v5\C51\INC\SiLABS\EFM8UB2\inc or
// C:\SiliconLabs\SimplicityStudio\v3\developer\sdks\si8051\v3\Device\EFM8UB2\inc,
// and then more macros are added to facilitate firmware development.
//------------------------------------------------------------------------------
// Copyright 2014 Silicon Laboratories, Inc.
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Silicon Laboratories End User
// License Agreement which accompanies this distribution, and is available at
// http://developer.silabs.com/legal/version/v11/Silicon_Labs_Software_License_Agreement.txt
// Original content and implementation provided by Silicon Laboratories.
//------------------------------------------------------------------------------
//Supported Devices:
//  EFM8UB20F32G_QFN32
//  EFM8UB20F32G_QFP32
//  EFM8UB20F32G_QFP48
//  EFM8UB20F64G_QFN32
//  EFM8UB20F64G_QFP32
//  EFM8UB20F64G_QFP48

#ifndef __SI_EFM8UB2_DEFS_H__
#define __SI_EFM8UB2_DEFS_H__

#include "si_toolchain.h"

//-----------------------------------------------------------------------------
// Register Definitions
//-----------------------------------------------------------------------------
SI_SFR (ACC,      0xE0); ///< Accumulator
SI_SFR (ADC0CF,   0xBC); ///< ADC0 Configuration
    #define ADC0CF_ADLJST   2 ///< ADC0 Left Justify Select
        #define RIGHT_JUSTIFIED 0
        #define LEFT_JUSTIFIED  1
    #define ADC0CF_ADSC     3 ///< SAR Clock Divider
SI_SFR (ADC0CN0,  0xE8); ///< ADC0 Control
    #define _ADC0CN0_ADCM    0 ///< Start of Conversion Mode Select
    #define _ADC0CN0_ADWINT  3 ///< Window Compare Interrupt Flag
    #define _ADC0CN0_ADBUSY  4 ///< ADC Busy
    #define _ADC0CN0_ADINT   5 ///< Conversion Complete Interrupt Flag
    #define _ADC0CN0_ADTM    6 ///< Track Mode
    #define _ADC0CN0_ADEN    7 ///< ADC Enable
SI_SFR (ADC0GTH,  0xC4); ///< ADC0 Greater-Than High Byte
SI_SFR (ADC0GTL,  0xC3); ///< ADC0 Greater-Than Low Byte
SI_SFR (ADC0H,    0xBE); ///< ADC0 Data Word High Byte
SI_SFR (ADC0L,    0xBD); ///< ADC0 Data Word Low Byte
SI_SFR (ADC0LTH,  0xC6); ///< ADC0 Less-Than High Byte
SI_SFR (ADC0LTL,  0xC5); ///< ADC0 Less-Than Low Byte
SI_SFR (AMX0N,    0xBA); ///< AMUX0 Negative Multiplexer Selection
    #define AMX0N_ADC0N0  0x00 ///< Select ADC0N.0
    #define AMX0N_ADC0N1  0x01 ///< Select ADC0N.1
    #define AMX0N_ADC0N2  0x02 ///< Select ADC0N.2
    #define AMX0N_ADC0N3  0x03 ///< Select ADC0N.3
    #define AMX0N_ADC0N4  0x04 ///< Select ADC0N.4
    #define AMX0N_ADC0N5  0x05 ///< Select ADC0N.5
    #define AMX0N_ADC0N6  0x06 ///< Select ADC0N.6
    #define AMX0N_ADC0N7  0x07 ///< Select ADC0N.7
    #define AMX0N_ADC0N8  0x08 ///< Select ADC0N.8
    #define AMX0N_ADC0N9  0x09 ///< Select ADC0N.9
    #define AMX0N_ADC0N10 0x0A ///< Select ADC0N.10
    #define AMX0N_ADC0N11 0x0B ///< Select ADC0N.11
    #define AMX0N_ADC0N12 0x0C ///< Select ADC0N.12
    #define AMX0N_ADC0N13 0x0D ///< Select ADC0N.13
    #define AMX0N_ADC0N14 0x0E ///< Select ADC0N.14
    #define AMX0N_ADC0N15 0x0F ///< Select ADC0N.15
    #define AMX0N_ADC0N16 0x10 ///< Select ADC0N.16
    #define AMX0N_ADC0N17 0x11 ///< Select ADC0N.17
    #define AMX0N_ADC0N18 0x12 ///< Select ADC0N.18
    #define AMX0N_ADC0N19 0x13 ///< Select ADC0N.19
    #define AMX0N_ADC0N20 0x14 ///< Select ADC0N.20
    #define AMX0N_ADC0N21 0x15 ///< Select ADC0N.21
    #define AMX0N_ADC0N22 0x16 ///< Select ADC0N.22
    #define AMX0N_ADC0N23 0x17 ///< Select ADC0N.23
    #define AMX0N_ADC0N24 0x18 ///< Select ADC0N.24
    #define AMX0N_ADC0N25 0x19 ///< Select ADC0N.25
    #define AMX0N_ADC0N26 0x1A ///< Select ADC0N.26
    #define AMX0N_ADC0N27 0x1B ///< Select ADC0N.27
    #define AMX0N_ADC0N28 0x1C ///< Select ADC0N.28
    #define AMX0N_ADC0N29 0x1D ///< Select ADC0N.29
    #define AMX0N_VREF    0x1E ///< Internal Voltage Reference
    #define AMX0N_GND     0x1F ///< Ground (single-ended mode)
    #define AMX0N_ADC0N32 0x20 ///< Select ADC0N.32
    #define AMX0N_ADC0N33 0x21 ///< Select ADC0N.33
    #define AMX0N_ADC0N34 0x22 ///< Select ADC0N.34
SI_SFR (AMX0P,    0xBB); ///< AMUX0 Positive Multiplexer Selection
    #define AMX0P_ADC0P0  0x00 ///< Select ADC0P.0
    #define AMX0P_ADC0P1  0x01 ///< Select ADC0P.1
    #define AMX0P_ADC0P2  0x02 ///< Select ADC0P.2
    #define AMX0P_ADC0P3  0x03 ///< Select ADC0P.3
    #define AMX0P_ADC0P4  0x04 ///< Select ADC0P.4
    #define AMX0P_ADC0P5  0x05 ///< Select ADC0P.5
    #define AMX0P_ADC0P6  0x06 ///< Select ADC0P.6
    #define AMX0P_ADC0P7  0x07 ///< Select ADC0P.7
    #define AMX0P_ADC0P8  0x08 ///< Select ADC0P.8
    #define AMX0P_ADC0P9  0x09 ///< Select ADC0P.9
    #define AMX0P_ADC0P10 0x0A ///< Select ADC0P.10
    #define AMX0P_ADC0P11 0x0B ///< Select ADC0P.11
    #define AMX0P_ADC0P12 0x0C ///< Select ADC0P.12
    #define AMX0P_ADC0P13 0x0D ///< Select ADC0P.13
    #define AMX0P_ADC0P14 0x0E ///< Select ADC0P.14
    #define AMX0P_ADC0P15 0x0F ///< Select ADC0P.15
    #define AMX0P_ADC0P16 0x10 ///< Select ADC0P.16
    #define AMX0P_ADC0P17 0x11 ///< Select ADC0P.17
    #define AMX0P_ADC0P18 0x12 ///< Select ADC0P.18
    #define AMX0P_ADC0P19 0x13 ///< Select ADC0P.19
    #define AMX0P_ADC0P20 0x14 ///< Select ADC0P.20
    #define AMX0P_ADC0P21 0x15 ///< Select ADC0P.21
    #define AMX0P_ADC0P22 0x16 ///< Select ADC0P.22
    #define AMX0P_ADC0P23 0x17 ///< Select ADC0P.23
    #define AMX0P_ADC0P24 0x18 ///< Select ADC0P.24
    #define AMX0P_ADC0P25 0x19 ///< Select ADC0P.25
    #define AMX0P_ADC0P26 0x1A ///< Select ADC0P.26
    #define AMX0P_ADC0P27 0x1B ///< Select ADC0P.27
    #define AMX0P_ADC0P28 0x1C ///< Select ADC0P.28
    #define AMX0P_ADC0P29 0x1D ///< Select ADC0P.29
    #define AMX0P_TEMP    0x1E ///< Temperature sensor
    #define AMX0P_VDD     0x1F ///< VDD Supply Voltage
    #define AMX0P_ADC0P32 0x20 ///< Select ADC0P.32
    #define AMX0P_ADC0P33 0x21 ///< Select ADC0P.33
    #define AMX0P_ADC0P34 0x22 ///< Select ADC0P.34
SI_SFR (B,        0xF0); ///< B Register
SI_SFR (CKCON0,   0x8E); ///< Clock Control 0
    #define CKCON0_SCA  0 ///< Timer 0/1 Prescale
        #define SYSCLK_DIV_12   0 ///< System clock divided by 12
        #define SYSCLK_DIV_4    1 ///< System clock divided by 4
        #define SYSCLK_DIV_48   2 ///< System clock divided by 48
        #define EXTOSC_DIV_8    3 ///< External oscillator divided by 8 (synchronized with the system clock)
    #define CKCON0_T0M  2 ///< Timer 0 Clock Select
        #define T0M_PRESCALE 0   ///< Timer 0 uses the clock defined by the prescale field, SCA.
        #define T0M_SYSCLK   1   ///< Timer 0 uses the system clock
    #define CKCON0_T1M  3 ///< Timer 1 Clock Select
        #define T1M_PRESCALE 0   ///< Timer 1 uses the clock defined by the prescale field, SCA.
        #define T1M_SYSCLK   1   ///< Timer 1 uses the system clock
    #define CKCON0_T2ML 4 ///< Timer 2 Low Byte Clock Select
        #define T2ML_EXTERNAL_CLOCK 0 ///< Timer 2 low byte uses the clock defined by T2XCLK in TMR2CN0
        #define T2ML_SYSCLK         1 ///< Timer 2 low byte uses the system clock.
    #define CKCON0_T3ML 6 ///< Timer 3 Low Byte Clock Select
        #define T3ML_EXTERNAL_CLOCK 0 ///< Timer 3 low byte uses the clock defined by T2XCLK in TMR2CN0
        #define T3ML_SYSCLK         1 ///< Timer 3 low byte uses the system clock.
SI_SFR (CKCON1,   0xE4); ///< Clock Control 1
SI_SFR (CLKSEL,   0xA9); ///< Clock Select
    #define CLKSEL_CLKSL    0 ///< System Clock Source Select Bits
        #define DIVIDED_HFOSC_DIV_4 0 ///< Clock (SYSCLK) derived from the Internal High-Frequency Oscillator / 4 and scaled per the IFCN bits in register HFO0CN
        #define EXTOSC              1 ///< Clock (SYSCLK) derived from the External Oscillator circuit
        #define HFOSC_DIV_2         2 ///< Clock (SYSCLK) derived from the Internal High-Frequency Oscillator / 2
        #define HFOSC               3 ///< Clock (SYSCLK) derived from the Internal High-Frequency Oscillator
        #define LFOSC               4 ///< Clock (SYSCLK) derived from the Internal Low-Frequency Oscillator and scaled per the OSCLD bits in register OSCLCN.
SI_SFR (CMP0CN0,  0x9B); ///< Comparator 0 Control 0
SI_SFR (CMP0MD,   0x9D); ///< Comparator 0 Mode
SI_SFR (CMP0MX,   0x9F); ///< Comparator 0 Multiplexer Selection
SI_SFR (CMP1CN0,  0x9A); ///< Comparator 1 Control 0
SI_SFR (CMP1MD,   0x9C); ///< Comparator 1 Mode
SI_SFR (CMP1MX,   0x9E); ///< Comparator 1 Multiplexer Selection
SI_SFR (DPH,      0x83); ///< Data Pointer High
SI_SFR (DPL,      0x82); ///< Data Pointer Low
SI_SFR (EIE1,     0xE6); ///< Extended Interrupt Enable 1
SI_SFR (EIE2,     0xE7); ///< Extended Interrupt Enable 2
SI_SFR (EIP1,     0xF6); ///< Extended Interrupt Priority 1
SI_SFR (EIP2,     0xF7); ///< Extended Interrupt Priority 2
SI_SFR (EMI0CF,   0x85); ///< External Memory Configuration
SI_SFR (EMI0CN,   0xAA); ///< External Memory Interface Control
SI_SFR (EMI0TC,   0x84); ///< External Memory Timing Control
SI_SFR (FLKEY,    0xB7); ///< Flash Lock and Key
SI_SFR (FLSCL,    0xB6); ///< Flash Scale
    #define FLSCL_FLRT  4
        #define SYSCLK_BELOW_25_MHZ 0
        #define SYSCLK_BELOW_48_MHZ 1
SI_SFR (HFO0CAL,  0xB3); ///< High Frequency Oscillator Calibration
SI_SFR (HFO0CN,   0xB2); ///< High Frequency Oscillator Control
    #define HFO0CN_IFCN 0 ///< Oscillator Frequency Divider Control
        #define SYSCLK_DIV_8 0 ///< SYSCLK can be derived from Internal H-F Oscillator divided by 8 (1.5 MHz)
        #define SYSCLK_DIV_4 1 ///< SYSCLK can be derived from Internal H-F Oscillator divided by 4 (3 MHz)
        #define SYSCLK_DIV_2 2 ///< SYSCLK can be derived from Internal H-F Oscillator divided by 2 (6 MHz)
        #define SYSCLK_DIV_1 3 ///< SYSCLK can be derived from Internal H-F Oscillator divided by 1 (12 MHz).
SI_SFR (IE,       0xA8); ///< Interrupt Enable
SI_SFR (IP,       0xB8); ///< Interrupt Priority
SI_SFR (IT01CF,   0xE4); ///< INT0/INT1 Configuration
    #define IT01CF_IN0SL    0
        #define IN0SL_P0_0  0
        #define IN0SL_P0_1  1
        #define IN0SL_P0_2  2
        #define IN0SL_P0_3  3
        #define IN0SL_P0_4  4
        #define IN0SL_P0_5  5
        #define IN0SL_P0_6  6
        #define IN0SL_P0_7  7
    #define IT01CF_IN0PL    3
        #define IN0PL_ACTIVE_LOW    0
        #define IN0PL_ACTIVE_HIGH   1
    #define IT01CF_IN1SL    4
        #define IN1SL_P0_0  0
        #define IN1SL_P0_1  1
        #define IN1SL_P0_2  2
        #define IN1SL_P0_3  3
        #define IN1SL_P0_4  4
        #define IN1SL_P0_5  5
        #define IN1SL_P0_6  6
        #define IN1SL_P0_7  7
    #define IT01CF_IN1PL    7
        #define IN1PL_ACTIVE_LOW    0
        #define IN1PL_ACTIVE_HIGH   1
SI_SFR (LFO0CN,   0x86); ///< Low Frequency Oscillator Control
SI_SFR (P0,       0x80); ///< Port 0 Pin Latch
SI_SFR (P0MDIN,   0xF1); ///< Port 0 Input Mode
    #define P0MDIN_B0   0
        #define ANALOG  0
        #define DIGITAL 1
    #define P0MDIN_B1   1
    #define P0MDIN_B2   2
    #define P0MDIN_B3   3
    #define P0MDIN_B4   4
    #define P0MDIN_B5   5
    #define P0MDIN_B6   6
    #define P0MDIN_B7   7
SI_SFR (P0MDOUT,  0xA4); ///< Port 0 Output Mode
    #define P0MDOUT_B0  0
        #define OPEN_DRAIN  0
        #define PUSH_PULL   1
    #define P0MDOUT_B1  1
    #define P0MDOUT_B2  2
    #define P0MDOUT_B3  3
    #define P0MDOUT_B4  4
    #define P0MDOUT_B5  5
    #define P0MDOUT_B6  6
    #define P0MDOUT_B7  7
SI_SFR (P0SKIP,   0xD4); ///< Port 0 Skip
    #define P0SKIP_B0   0
        #define NOT_SKIPPED 0
        #define SKIPPED     1
    #define P0SKIP_B1   1
    #define P0SKIP_B2   2
    #define P0SKIP_B3   3
    #define P0SKIP_B4   4
    #define P0SKIP_B5   5
    #define P0SKIP_B6   6
    #define P0SKIP_B7   7
SI_SFR (P1,       0x90); ///< Port 1 Pin Latch
SI_SFR (P1MDIN,   0xF2); ///< Port 1 Input Mode
    #define P1MDIN_B0   0
    #define P1MDIN_B1   1
    #define P1MDIN_B2   2
    #define P1MDIN_B3   3
    #define P1MDIN_B4   4
    #define P1MDIN_B5   5
    #define P1MDIN_B6   6
    #define P1MDIN_B7   7
SI_SFR (P1MDOUT,  0xA5); ///< Port 1 Output Mode
    #define P1MDOUT_B0  0
    #define P1MDOUT_B1  1
    #define P1MDOUT_B2  2
    #define P1MDOUT_B3  3
    #define P1MDOUT_B4  4
    #define P1MDOUT_B5  5
    #define P1MDOUT_B6  6
    #define P1MDOUT_B7  7
SI_SFR (P1SKIP,   0xD5); ///< Port 1 Skip
    #define P1SKIP_B0   0
    #define P1SKIP_B1   1
    #define P1SKIP_B2   2
    #define P1SKIP_B3   3
    #define P1SKIP_B4   4
    #define P1SKIP_B5   5
    #define P1SKIP_B6   6
    #define P1SKIP_B7   7
SI_SFR (P2,       0xA0); ///< Port 2 Pin Latch
SI_SFR (P2MDIN,   0xF3); ///< Port 2 Input Mode
    #define P2MDIN_B0   0
    #define P2MDIN_B1   1
    #define P2MDIN_B2   2
    #define P2MDIN_B3   3
    #define P2MDIN_B4   4
    #define P2MDIN_B5   5
    #define P2MDIN_B6   6
    #define P2MDIN_B7   7
SI_SFR (P2MDOUT,  0xA6); ///< Port 2 Output Mode
    #define P2MDOUT_B0  0
    #define P2MDOUT_B1  1
    #define P2MDOUT_B2  2
    #define P2MDOUT_B3  3
    #define P2MDOUT_B4  4
    #define P2MDOUT_B5  5
    #define P2MDOUT_B6  6
    #define P2MDOUT_B7  7
SI_SFR (P2SKIP,   0xD6); ///< Port 2 Skip
    #define P2SKIP_B0   0
    #define P2SKIP_B1   1
    #define P2SKIP_B2   2
    #define P2SKIP_B3   3
    #define P2SKIP_B4   4
    #define P2SKIP_B5   5
    #define P2SKIP_B6   6
    #define P2SKIP_B7   7
SI_SFR (P3,       0xB0); ///< Port 3 Pin Latch
SI_SFR (P3MDIN,   0xF4); ///< Port 3 Input Mode
    #define P3MDIN_B0   0
    #define P3MDIN_B1   1
    #define P3MDIN_B2   2
    #define P3MDIN_B3   3
    #define P3MDIN_B4   4
    #define P3MDIN_B5   5
    #define P3MDIN_B6   6
    #define P3MDIN_B7   7
SI_SFR (P3MDOUT,  0xA7); ///< Port 3 Output Mode
    #define P3MDOUT_B0  0
    #define P3MDOUT_B1  1
    #define P3MDOUT_B2  2
    #define P3MDOUT_B3  3
    #define P3MDOUT_B4  4
    #define P3MDOUT_B5  5
    #define P3MDOUT_B6  6
    #define P3MDOUT_B7  7
SI_SFR (P3SKIP,   0xDF); ///< Port 3 Skip
    #define P3SKIP_B0   0
    #define P3SKIP_B1   1
    #define P3SKIP_B2   2
    #define P3SKIP_B3   3
    #define P3SKIP_B4   4
    #define P3SKIP_B5   5
    #define P3SKIP_B6   6
    #define P3SKIP_B7   7
SI_SFR (P4,       0xC7); ///< Port 4 Pin Latch
SI_SFR (P4MDIN,   0xF5); ///< Port 4 Input Mode
SI_SFR (P4MDOUT,  0xAE); ///< Port 4 Output Mode
SI_SFR (PCA0CN0,  0xD8); ///< PCA Control 0

SI_SFR (PCA0CPH0, 0xFC); ///< PCA Channel 0 Capture Module High Byte
SI_SFR (PCA0CPH1, 0xEA); ///< PCA Channel 1 Capture Module High Byte
SI_SFR (PCA0CPH2, 0xEC); ///< PCA Channel 2 Capture Module High Byte
SI_SFR (PCA0CPH3, 0xEE); ///< PCA Channel 3 Capture Module High Byte
SI_SFR (PCA0CPH4, 0xFE); ///< PCA Channel 4 Capture Module High Byte
SI_SFR (PCA0CPL0, 0xFB); ///< PCA Channel 0 Capture Module Low Byte
SI_SFR (PCA0CPL1, 0xE9); ///< PCA Channel 1 Capture Module Low Byte
SI_SFR (PCA0CPL2, 0xEB); ///< PCA Channel 2 Capture Module Low Byte
SI_SFR (PCA0CPL3, 0xED); ///< PCA Channel 3 Capture Module Low Byte
SI_SFR (PCA0CPL4, 0xFD); ///< PCA Channel 4 Capture Module Low Byte
SI_SFR (PCA0CPM0, 0xDA); ///< PCA Channel 0 Capture/Compare Mode
    #define PCA0CPM_ECCF    0 ///< Capture/Compare Flag Interrupt Enable
    #define PCA0CPM_PWM     1 ///< Pulse Width Modulation Mode Enable
    #define PCA0CPM_TOG     2 ///< Toggle Function Enable
    #define PCA0CPM_MAT     3 ///< Match Function Enable
    #define PCA0CPM_CAPN    4 ///< Capture Negative Function Enable
    #define PCA0CPM_CAPP    5 ///< Capture Positive Function Enable
    #define PCA0CPM_ECOM    6 ///< Comparator Function Enable
    #define PCA0CPM_PWM16   7 ///< 16-bit Pulse Width Modulation Enable
SI_SFR (PCA0CPM1, 0xDB); ///< PCA Channel 1 Capture/Compare Mode
SI_SFR (PCA0CPM2, 0xDC); ///< PCA Channel 2 Capture/Compare Mode
SI_SFR (PCA0CPM3, 0xDD); ///< PCA Channel 3 Capture/Compare Mode
SI_SFR (PCA0CPM4, 0xDE); ///< PCA Channel 4 Capture/Compare Mode
SI_SFR (PCA0H,    0xFA); ///< PCA Counter/Timer High Byte
SI_SFR (PCA0L,    0xF9); ///< PCA Counter/Timer Low Byte
SI_SFR (PCA0MD,   0xD9); ///< PCA Mode
    #define PCA0MD_ECF      0 //< PCA Counter/Timer Overflow Interrupt Enable
    #define PCA0MD_CPS      1 ///< PCA Counter/Timer Pulse Select
    #define PCA0MD_CPS_SYSCLK_DIV_12    0

    #define PCA0MD_WDLCK    5 ///< Watchdog Timer Lock
    #define PCA0MD_WDTE     6 ///< Watchdog Timer Enable
    #define PCA0MD_CIDL     7 ///< PCA Counter/Timer Idle Control
SI_SFR (PCON0,    0x87); ///< Power Control
SI_SFR (PFE0CN,   0xAF); ///< Prefetch Engine Control
SI_SFR (PSCTL,    0x8F); ///< Program Store Control
SI_SFR (PSW,      0xD0); ///< Program Status Word
SI_SFR (REF0CN,   0xD1); ///< Voltage Reference Control
    #define REF0CN_REFBE    0 ///< Internal Reference Buffer Enable
    #define REF0CN_TEMPE    2 ///< Temperature Sensor Enable
    #define REF0CN_REFSL    3 ///< Voltage Reference Select
    #define REF0CN_REGOVR   4 ///< Regulator Reference Override
    #define REF0CN_REFBGS   7 ///< Reference Buffer Gain Select
SI_SFR (REG01CN,  0xC9); ///< Voltage Regulator Control
SI_SFR (RSTSRC,   0xEF); ///< Reset Source
    #define RSTSRC_PORSF    1 ///< Power-On / Supply Monitor Reset Flag, and Supply Monitor Reset Enable
    #define RSTSRC_MCDRSF   2 ///< Missing Clock Detector Enable and Flag
    #define RSTSRC_SWRSF    4 ///< Software Reset Force and Flag
SI_SFR (SBCON1,   0xAC); ///< UART1 Baud Rate Generator Control
SI_SFR (SBRLH1,   0xB5); ///< UART1 Baud Rate Generator High Byte
SI_SFR (SBRLL1,   0xB4); ///< UART1 Baud Rate Generator Low Byte
SI_SFR (SBUF0,    0x99); ///< UART0 Serial Port Data Buffer
SI_SFR (SBUF1,    0xD3); ///< UART1 Serial Port Data Buffer
SI_SFR (SCON0,    0x98); ///< UART0 Serial Port Control
    #define _SCON0_REN   4 ///< Receive Enable
    #define _SCON0_SMODE 7 ///< Serial Port 0 Operation Mode
        #define SMODE_8_BIT 0 ///< 8-bit UART with Variable Baud Rate (Mode 0).
        #define SMODE_9_BIT 1 ///< 9-bit UART with Variable Baud Rate (Mode 1).
SI_SFR (SCON1,    0xD2); ///< UART1 Serial Port Control
SI_SFR (SFRPAGE,  0xBF); ///< SFR Page
SI_SFR (SMB0ADM,  0xCE); ///< SMBus 0 Slave Address Mask
SI_SFR (SMB0ADR,  0xCF); ///< SMBus 0 Slave Address
SI_SFR (SMB0CF,   0xC1); ///< SMBus 0 Configuration
    #define SMB0CF_SMBCS    0 ///< SMBus Clock Source Selection
        #define SMBCS_TIMER0        0 ///< Timer 0 Overflow.
        #define SMBCS_TIMER1        1 ///< Timer 1 Overflow.
        #define SMBCS_TIMER2_HIGH   2 ///< Timer 2 High Byte Overflow
        #define SMBCS_TIMER2_LOW    3 ///< Timer 2 Low Byte Overflow.
    #define SMB0CF_EXTHOLD  4 ///< SMBus Setup and Hold Time Extension Enable
    #define SMB0CF_INH      6 ///< SMBus Slave Inhibit
    #define SMB0CF_ENSMB    7 ///< SMBus Enable
SI_SFR (SMB0CN0,  0xC0); ///< SMBus 0 Control
SI_SFR (SMB0DAT,  0xC2); ///< SMBus 0 Data
SI_SFR (SMB1ADM,  0xCE); ///< SMBus 1 Slave Address Mask
SI_SFR (SMB1ADR,  0xCF); ///< SMBus 1 Slave Address
SI_SFR (SMB1CF,   0xC1); ///< SMBus 1 Configuration
SI_SFR (SMB1CN0,  0xC0); ///< SMBus 1 Control
SI_SFR (SMB1DAT,  0xC2); ///< SMBus 1 Data
SI_SFR (SMBTC,    0xB9); ///< SMBus Timing and Pin Control
SI_SFR (SMOD1,    0xE5); ///< UART1 Mode
SI_SFR (SP,       0x81); ///< Stack Pointer
SI_SFR (SPI0CFG,  0xA1); ///< SPI0 Configuration
SI_SFR (SPI0CKR,  0xA2); ///< SPI0 Clock Rate
SI_SFR (SPI0CN0,  0xF8); ///< SPI0 Control
SI_SFR (SPI0DAT,  0xA3); ///< SPI0 Data
SI_SFR (TCON,     0x88); ///< Timer 0/1 Control
SI_SFR (TH0,      0x8C); ///< Timer 0 High Byte
SI_SFR (TH1,      0x8D); ///< Timer 1 High Byte
SI_SFR (TL0,      0x8A); ///< Timer 0 Low Byte
SI_SFR (TL1,      0x8B); ///< Timer 1 Low Byte
SI_SFR (TMOD,     0x89); ///< Timer 0/1 Mode
    #define TMOD_T0M    0 ///< Timer 0 Mode Select
        #define T0M_MODE0   0 ///< Mode 0, 13-bit Counter/Timer
        #define T0M_MODE1   1 ///< Mode 1, 16-bit Counter/Timer
        #define T0M_MODE2   2 ///< Mode 2, 8-bit Counter/Timer with Auto-Reload
        #define T0M_MODE3   3 ///< Mode 3, Two 8-bit Counter/Timers
    #define TMOD_T1M    4 ///< Timer 1 Mode Select
        #define T1M_MODE0   0 ///< Mode 0, 13-bit Counter/Timer
        #define T1M_MODE1   1 ///< Mode 1, 16-bit Counter/Timer
        #define T1M_MODE2   2 ///< Mode 2, 8-bit Counter/Timer with Auto-Reload
        #define T1M_MODE3   3 ///< Mode 3, Two 8-bit Counter/Timers
SI_SFR (TMR2CN0,  0xC8); ///< Timer 2 Control 0
    #define TMR2CN_T2SOF    4
    #define TMR2CN_T2SPLIT  3
    #define TMR2CN_TR2      2
    #define TMR2CN_T2XCLK   0
        #define T2XCLK_SYSCLK_DIV_12    0 ///< Timer 2 clock is the system clock divided by 12
        #define T2XCLK_EXTOSC_DIV_8     1 ///< Timer 2 clock is the external oscillator divided
SI_SFR (TMR2H,    0xCD); ///< Timer 2 High Byte
SI_SFR (TMR2L,    0xCC); ///< Timer 2 Low Byte
SI_SFR (TMR2RLH,  0xCB); ///< Timer 2 Reload High Byte
SI_SFR (TMR2RLL,  0xCA); ///< Timer 2 Reload Low Byte
SI_SFR (TMR3CN0,  0x91); ///< Timer 3 Control 0
    #define TMR3CN_TF3H     7 ///< Timer 3 High Byte Overflow Flag
    #define TMR3CN_T3SOF    4
    #define TMR3CN_T3SPLIT  3
    #define TMR3CN_TR3      2 ///< Timer 3 Run Control
    #define TMR3CN_T3XCLK   0
        #define T3XCLK_SYSCLK_DIV_12    0 ///< Timer 3 clock is the system clock divided by 12
        #define T3XCLK_EXTOSC_DIV_8     1 ///< Timer 3 clock is the external oscillator divided
SI_SFR (TMR3H,    0x95); ///< Timer 3 High Byte
SI_SFR (TMR3L,    0x94); ///< Timer 3 Low Byte
SI_SFR (TMR3RLH,  0x93); ///< Timer 3 Reload High Byte
SI_SFR (TMR3RLL,  0x92); ///< Timer 3 Reload Low Byte
SI_SFR (TMR4CN0,  0x91); ///< Timer 4 Control 0
SI_SFR (TMR4H,    0x95); ///< Timer 4 High Byte
SI_SFR (TMR4L,    0x94); ///< Timer 4 Low Byte
SI_SFR (TMR4RLH,  0x93); ///< Timer 4 Reload High Byte
SI_SFR (TMR4RLL,  0x92); ///< Timer 4 Reload Low Byte
SI_SFR (TMR5CN0,  0xC8); ///< Timer 5 Control 0
SI_SFR (TMR5H,    0xCD); ///< Timer 5 High Byte
SI_SFR (TMR5L,    0xCC); ///< Timer 5 Low Byte
SI_SFR (TMR5RLH,  0xCB); ///< Timer 5 Reload High Byte
SI_SFR (TMR5RLL,  0xCA); ///< Timer 5 Reload Low Byte
SI_SFR (USB0ADR,  0x96); ///< USB0 Indirect Address
SI_SFR (USB0DAT,  0x97); ///< USB0 Data
SI_SFR (USB0XCN,  0xD7); ///< USB0 Transceiver Control
SI_SFR (VDM0CN,   0xFF); ///< Supply Monitor Control
    #define VDM0CN_VDMEN    7 ///< Supply Monitor Enable
    #define VDM0CN_VDDSTAT  6 ///< Read Only, Supply Status
SI_SFR (XBR0,     0xE1); ///< Port I/O Crossbar 0
    #define XBR0_URT0E  0 ///< UART0 I/O Output Enable
    #define XBR0_SMB0E  2 ///< SMB0 I/O Enable
    #define XBR0_SYSCKE 3 ///< SYSCLK Output Enable
SI_SFR (XBR1,     0xE2); ///< Port I/O Crossbar 1
    #define XBR1_XBARE      6 ///< Crossbar Enable
    #define XBR1_WEAKPUD    7 ///< Port I/O Weak Pullup Disable
SI_SFR (XBR2,     0xE3); ///< Port I/O Crossbar 2
SI_SFR (XOSC0CN,  0xB1); ///< External Oscillator Control

//------------------------------------------------------------------------------
// 16-bit Register Definitions (may not work on all compilers)
//------------------------------------------------------------------------------
SI_SFR16 (ADC0GT,  0xC3); ///< ADC0 Greater-Than
SI_SFR16 (ADC0,    0xBD); ///< ADC0 Data Word
SI_SFR16 (ADC0LT,  0xC5); ///< ADC0 Less-Than
SI_SFR16 (DP,      0x82); ///< Data Pointer
SI_SFR16 (PCA0CP0, 0xFB); ///< PCA Channel 0 Capture Module
SI_SFR16 (PCA0CP1, 0xE9); ///< PCA Channel 1 Capture Module
SI_SFR16 (PCA0CP2, 0xEB); ///< PCA Channel 2 Capture Module
SI_SFR16 (PCA0CP3, 0xED); ///< PCA Channel 3 Capture Module
SI_SFR16 (PCA0CP4, 0xFD); ///< PCA Channel 4 Capture Module
SI_SFR16 (PCA0,    0xF9); ///< PCA Counter/Timer
SI_SFR16 (SBRL1,   0xB4); ///< UART1 Baud Rate Generator
SI_SFR16 (TMR2,    0xCC); ///< Timer 2
SI_SFR16 (TMR2RL,  0xCA); ///< Timer 2 Reload
SI_SFR16 (TMR3,    0x94); ///< Timer 3
SI_SFR16 (TMR3RL,  0x92); ///< Timer 3 Reload
SI_SFR16 (TMR4,    0x94); ///< Timer 4
SI_SFR16 (TMR4RL,  0x92); ///< Timer 4 Reload
SI_SFR16 (TMR5,    0xCC); ///< Timer 5
SI_SFR16 (TMR5RL,  0xCA); ///< Timer 5 Reload

//------------------------------------------------------------------------------
// Indirect Register Definitions
//------------------------------------------------------------------------------
#define CLKREC   0x0F ///< USB0 Clock Recovery Control
#define CMIE     0x0B ///< USB0 Common Interrupt Enable
#define CMINT    0x06 ///< USB0 Common Interrupt
#define E0CNT    0x16 ///< USB0 Endpoint0 Data Count
#define E0CSR    0x11 ///< USB0 Endpoint0 Control
#define EENABLE  0x1E ///< USB0 Endpoint Enable
#define EINCSRH  0x12 ///< USB0 IN Endpoint Control High
#define EINCSRL  0x11 ///< USB0 IN Endpoint Control
#define EOUTCNTH 0x17 ///< USB0 OUT Endpoint Count High
#define EOUTCNTL 0x16 ///< USB0 OUT Endpoint Count
#define EOUTCSRH 0x15 ///< USB0 OUT Endpoint Control High
#define EOUTCSRL 0x14 ///< USB0 OUT Endpoint Control
#define FADDR    0x00 ///< USB0 Function Address
#define FIFO0    0x20 ///< USB0 Endpoint 0 FIFO Access
#define FIFO1    0x21 ///< USB0 Endpoint 1 FIFO Access
#define FIFO2    0x22 ///< USB0 Endpoint 2 FIFO Access
#define FIFO3    0x23 ///< USB0 Endpoint 3 FIFO Access
#define FRAMEH   0x0D ///< USB0 Frame Number High
#define FRAMEL   0x0C ///< USB0 Frame Number
#define IN1IE    0x07 ///< USB0 IN Endpoint Interrupt Enable
#define IN1INT   0x02 ///< USB0 IN Endpoint Interrupt
#define INDEX    0x0E ///< USB0 Endpoint Index
#define OUT1IE   0x09 ///< USB0 OUT Endpoint Interrupt Enable
#define OUT1INT  0x04 ///< USB0 OUT Endpoint Interrupt
#define POWER    0x01 ///< USB0 Power

//------------------------------------------------------------------------------
// Bit Definitions
//------------------------------------------------------------------------------

// ACC (Accumulator)
#define SFR_ACC 0xE0
SI_SBIT (ACC_ACC0, SFR_ACC, 0); ///< Accumulator Bit 0
SI_SBIT (ACC_ACC1, SFR_ACC, 1); ///< Accumulator Bit 1
SI_SBIT (ACC_ACC2, SFR_ACC, 2); ///< Accumulator Bit 2
SI_SBIT (ACC_ACC3, SFR_ACC, 3); ///< Accumulator Bit 3
SI_SBIT (ACC_ACC4, SFR_ACC, 4); ///< Accumulator Bit 4
SI_SBIT (ACC_ACC5, SFR_ACC, 5); ///< Accumulator Bit 5
SI_SBIT (ACC_ACC6, SFR_ACC, 6); ///< Accumulator Bit 6
SI_SBIT (ACC_ACC7, SFR_ACC, 7); ///< Accumulator Bit 7

// ADC0CN0 (ADC0 Control)
#define SFR_ADC0CN0 0xE8
SI_SBIT (ADC0CN0_ADCM0,  SFR_ADC0CN0, 0); ///< Start of Conversion Mode Select Bit 0
SI_SBIT (ADC0CN0_ADCM1,  SFR_ADC0CN0, 1); ///< Start of Conversion Mode Select Bit 1
SI_SBIT (ADC0CN0_ADCM2,  SFR_ADC0CN0, 2); ///< Start of Conversion Mode Select Bit 2
SI_SBIT (ADC0CN0_ADWINT, SFR_ADC0CN0, 3); ///< Window Compare Interrupt Flag
SI_SBIT (ADC0CN0_ADBUSY, SFR_ADC0CN0, 4); ///< ADC Busy
SI_SBIT (ADC0CN0_ADINT,  SFR_ADC0CN0, 5); ///< Conversion Complete Interrupt Flag
SI_SBIT (ADC0CN0_ADTM,   SFR_ADC0CN0, 6); ///< Track Mode
SI_SBIT (ADC0CN0_ADEN,   SFR_ADC0CN0, 7); ///< ADC Enable

// B (B Register)
#define SFR_B 0xF0
SI_SBIT (B_B0, SFR_B, 0); ///< B Register Bit 0
SI_SBIT (B_B1, SFR_B, 1); ///< B Register Bit 1
SI_SBIT (B_B2, SFR_B, 2); ///< B Register Bit 2
SI_SBIT (B_B3, SFR_B, 3); ///< B Register Bit 3
SI_SBIT (B_B4, SFR_B, 4); ///< B Register Bit 4
SI_SBIT (B_B5, SFR_B, 5); ///< B Register Bit 5
SI_SBIT (B_B6, SFR_B, 6); ///< B Register Bit 6
SI_SBIT (B_B7, SFR_B, 7); ///< B Register Bit 7

// IE (Interrupt Enable)
#define SFR_IE 0xA8
SI_SBIT (IE_EX0,   SFR_IE, 0); ///< External Interrupt 0 Enable
SI_SBIT (IE_ET0,   SFR_IE, 1); ///< Timer 0 Interrupt Enable
SI_SBIT (IE_EX1,   SFR_IE, 2); ///< External Interrupt 1 Enable
SI_SBIT (IE_ET1,   SFR_IE, 3); ///< Timer 1 Interrupt Enable
SI_SBIT (IE_ES0,   SFR_IE, 4); ///< UART0 Interrupt Enable
SI_SBIT (IE_ET2,   SFR_IE, 5); ///< Timer 2 Interrupt Enable
SI_SBIT (IE_ESPI0, SFR_IE, 6); ///< SPI0 Interrupt Enable
SI_SBIT (IE_EA,    SFR_IE, 7); ///< All Interrupts Enable

// IP (Interrupt Priority)
#define SFR_IP 0xB8
SI_SBIT (IP_PX0,   SFR_IP, 0); ///< External Interrupt 0 Priority Control
SI_SBIT (IP_PT0,   SFR_IP, 1); ///< Timer 0 Interrupt Priority Control
SI_SBIT (IP_PX1,   SFR_IP, 2); ///< External Interrupt 1 Priority Control
SI_SBIT (IP_PT1,   SFR_IP, 3); ///< Timer 1 Interrupt Priority Control
SI_SBIT (IP_PS0,   SFR_IP, 4); ///< UART0 Interrupt Priority Control
SI_SBIT (IP_PT2,   SFR_IP, 5); ///< Timer 2 Interrupt Priority Control
SI_SBIT (IP_PSPI0, SFR_IP, 6); ///< Serial Peripheral Interface (SPI0) Interrupt Priority Control

// P0 (Port 0 Pin Latch)
#define SFR_P0 0x80
SI_SBIT (P0_B0, SFR_P0, 0); ///< Port 0 Bit 0 Latch
SI_SBIT (P0_B1, SFR_P0, 1); ///< Port 0 Bit 1 Latch
SI_SBIT (P0_B2, SFR_P0, 2); ///< Port 0 Bit 2 Latch
SI_SBIT (P0_B3, SFR_P0, 3); ///< Port 0 Bit 3 Latch
SI_SBIT (P0_B4, SFR_P0, 4); ///< Port 0 Bit 4 Latch
SI_SBIT (P0_B5, SFR_P0, 5); ///< Port 0 Bit 5 Latch
SI_SBIT (P0_B6, SFR_P0, 6); ///< Port 0 Bit 6 Latch
SI_SBIT (P0_B7, SFR_P0, 7); ///< Port 0 Bit 7 Latch

// P1 (Port 1 Pin Latch)
#define SFR_P1 0x90
SI_SBIT (P1_B0, SFR_P1, 0); ///< Port 1 Bit 0 Latch
SI_SBIT (P1_B1, SFR_P1, 1); ///< Port 1 Bit 1 Latch
SI_SBIT (P1_B2, SFR_P1, 2); ///< Port 1 Bit 2 Latch
SI_SBIT (P1_B3, SFR_P1, 3); ///< Port 1 Bit 3 Latch
SI_SBIT (P1_B4, SFR_P1, 4); ///< Port 1 Bit 4 Latch
SI_SBIT (P1_B5, SFR_P1, 5); ///< Port 1 Bit 5 Latch
SI_SBIT (P1_B6, SFR_P1, 6); ///< Port 1 Bit 6 Latch
SI_SBIT (P1_B7, SFR_P1, 7); ///< Port 1 Bit 7 Latch

// P2 (Port 2 Pin Latch)
#define SFR_P2 0xA0
SI_SBIT (P2_B0, SFR_P2, 0); ///< Port 2 Bit 0 Latch
SI_SBIT (P2_B1, SFR_P2, 1); ///< Port 2 Bit 1 Latch
SI_SBIT (P2_B2, SFR_P2, 2); ///< Port 2 Bit 2 Latch
SI_SBIT (P2_B3, SFR_P2, 3); ///< Port 2 Bit 3 Latch
SI_SBIT (P2_B4, SFR_P2, 4); ///< Port 2 Bit 4 Latch
SI_SBIT (P2_B5, SFR_P2, 5); ///< Port 2 Bit 5 Latch
SI_SBIT (P2_B6, SFR_P2, 6); ///< Port 2 Bit 6 Latch
SI_SBIT (P2_B7, SFR_P2, 7); ///< Port 2 Bit 7 Latch

// P3 (Port 3 Pin Latch)
#define SFR_P3 0xB0
SI_SBIT (P3_B0, SFR_P3, 0); ///< Port 3 Bit 0 Latch
SI_SBIT (P3_B1, SFR_P3, 1); ///< Port 3 Bit 1 Latch
SI_SBIT (P3_B2, SFR_P3, 2); ///< Port 3 Bit 2 Latch
SI_SBIT (P3_B3, SFR_P3, 3); ///< Port 3 Bit 3 Latch
SI_SBIT (P3_B4, SFR_P3, 4); ///< Port 3 Bit 4 Latch
SI_SBIT (P3_B5, SFR_P3, 5); ///< Port 3 Bit 5 Latch
SI_SBIT (P3_B6, SFR_P3, 6); ///< Port 3 Bit 6 Latch
SI_SBIT (P3_B7, SFR_P3, 7); ///< Port 3 Bit 7 Latch

// PCA0CN0 (PCA Control 0)
#define SFR_PCA0CN0 0xD8
SI_SBIT (PCA0CN0_CCF0, SFR_PCA0CN0, 0); ///< PCA Module 0 Capture/Compare Flag
SI_SBIT (PCA0CN0_CCF1, SFR_PCA0CN0, 1); ///< PCA Module 1 Capture/Compare Flag
SI_SBIT (PCA0CN0_CCF2, SFR_PCA0CN0, 2); ///< PCA Module 2 Capture/Compare Flag
SI_SBIT (PCA0CN0_CCF3, SFR_PCA0CN0, 3); ///< PCA Module 3 Capture/Compare Flag
SI_SBIT (PCA0CN0_CCF4, SFR_PCA0CN0, 4); ///< PCA Module 4 Capture/Compare Flag
SI_SBIT (PCA0CN0_CR,   SFR_PCA0CN0, 6); ///< PCA Counter/Timer Run Control
SI_SBIT (PCA0CN0_CF,   SFR_PCA0CN0, 7); ///< PCA Counter/Timer Overflow Flag

// PSW (Program Status Word)
#define SFR_PSW 0xD0
SI_SBIT (PSW_PARITY, SFR_PSW, 0); ///< Parity Flag
SI_SBIT (PSW_F1,     SFR_PSW, 1); ///< User Flag 1
SI_SBIT (PSW_OV,     SFR_PSW, 2); ///< Overflow Flag
SI_SBIT (PSW_RS0,    SFR_PSW, 3); ///< Register Bank Select Bit 0
SI_SBIT (PSW_RS1,    SFR_PSW, 4); ///< Register Bank Select Bit 1
SI_SBIT (PSW_F0,     SFR_PSW, 5); ///< User Flag 0
SI_SBIT (PSW_AC,     SFR_PSW, 6); ///< Auxiliary Carry Flag
SI_SBIT (PSW_CY,     SFR_PSW, 7); ///< Carry Flag

// SCON0 (UART0 Serial Port Control)
#define SFR_SCON0 0x98
SI_SBIT (SCON0_RI,    SFR_SCON0, 0); ///< Receive Interrupt Flag
SI_SBIT (SCON0_TI,    SFR_SCON0, 1); ///< Transmit Interrupt Flag
SI_SBIT (SCON0_RB8,   SFR_SCON0, 2); ///< Ninth Receive Bit
SI_SBIT (SCON0_TB8,   SFR_SCON0, 3); ///< Ninth Transmission Bit
SI_SBIT (SCON0_REN,   SFR_SCON0, 4); ///< Receive Enable
SI_SBIT (SCON0_MCE,   SFR_SCON0, 5); ///< Multiprocessor Communication Enable
SI_SBIT (SCON0_SMODE, SFR_SCON0, 7); ///< Serial Port 0 Operation Mode

// SMB0CN0 (SMBus 0 Control)
#define SFR_SMB0CN0 0xC0
SI_SBIT (SMB0CN0_SI,      SFR_SMB0CN0, 0); ///< SMBus Interrupt Flag
SI_SBIT (SMB0CN0_ACK,     SFR_SMB0CN0, 1); ///< SMBus Acknowledge
SI_SBIT (SMB0CN0_ARBLOST, SFR_SMB0CN0, 2); ///< SMBus Arbitration Lost Indicator
SI_SBIT (SMB0CN0_ACKRQ,   SFR_SMB0CN0, 3); ///< SMBus Acknowledge Request
SI_SBIT (SMB0CN0_STO,     SFR_SMB0CN0, 4); ///< SMBus Stop Flag
SI_SBIT (SMB0CN0_STA,     SFR_SMB0CN0, 5); ///< SMBus Start Flag
SI_SBIT (SMB0CN0_TXMODE,  SFR_SMB0CN0, 6); ///< SMBus Transmit Mode Indicator
SI_SBIT (SMB0CN0_MASTER,  SFR_SMB0CN0, 7); ///< SMBus Master/Slave Indicator

// SMB1CN0 (SMBus 1 Control)
#define SFR_SMB1CN0 0xC0
SI_SBIT (SMB1CN0_SI,      SFR_SMB1CN0, 0); ///< SMBus Interrupt Flag
SI_SBIT (SMB1CN0_ACK,     SFR_SMB1CN0, 1); ///< SMBus Acknowledge
SI_SBIT (SMB1CN0_ARBLOST, SFR_SMB1CN0, 2); ///< SMBus Arbitration Lost Indicator
SI_SBIT (SMB1CN0_ACKRQ,   SFR_SMB1CN0, 3); ///< SMBus Acknowledge Request
SI_SBIT (SMB1CN0_STO,     SFR_SMB1CN0, 4); ///< SMBus Stop Flag
SI_SBIT (SMB1CN0_STA,     SFR_SMB1CN0, 5); ///< SMBus Start Flag
SI_SBIT (SMB1CN0_TXMODE,  SFR_SMB1CN0, 6); ///< SMBus Transmit Mode Indicator
SI_SBIT (SMB1CN0_MASTER,  SFR_SMB1CN0, 7); ///< SMBus Master/Slave Indicator

// SPI0CN0 (SPI0 Control)
#define SFR_SPI0CN0 0xF8
SI_SBIT (SPI0CN0_SPIEN,  SFR_SPI0CN0, 0); ///< SPI0 Enable
SI_SBIT (SPI0CN0_TXBMT,  SFR_SPI0CN0, 1); ///< Transmit Buffer Empty
SI_SBIT (SPI0CN0_NSSMD0, SFR_SPI0CN0, 2); ///< Slave Select Mode Bit 0
SI_SBIT (SPI0CN0_NSSMD1, SFR_SPI0CN0, 3); ///< Slave Select Mode Bit 1
SI_SBIT (SPI0CN0_RXOVRN, SFR_SPI0CN0, 4); ///< Receive Overrun Flag
SI_SBIT (SPI0CN0_MODF,   SFR_SPI0CN0, 5); ///< Mode Fault Flag
SI_SBIT (SPI0CN0_WCOL,   SFR_SPI0CN0, 6); ///< Write Collision Flag
SI_SBIT (SPI0CN0_SPIF,   SFR_SPI0CN0, 7); ///< SPI0 Interrupt Flag

// TCON (Timer 0/1 Control)
#define SFR_TCON 0x88
SI_SBIT (TCON_IT0, SFR_TCON, 0); ///< Interrupt 0 Type Select
    #define TCON_IT0_LEVEL  0
    #define TCON_IT0_EDGE   1
SI_SBIT (TCON_IE0, SFR_TCON, 1); ///< External Interrupt 0
SI_SBIT (TCON_IT1, SFR_TCON, 2); ///< Interrupt 1 Type Select
    #define TCON_IT1_LEVEL  0
    #define TCON_IT1_EDGE   1
SI_SBIT (TCON_IE1, SFR_TCON, 3); ///< External Interrupt 1
SI_SBIT (TCON_TR0, SFR_TCON, 4); ///< Timer 0 Run Control
SI_SBIT (TCON_TF0, SFR_TCON, 5); ///< Timer 0 Overflow Flag
SI_SBIT (TCON_TR1, SFR_TCON, 6); ///< Timer 1 Run Control
SI_SBIT (TCON_TF1, SFR_TCON, 7); ///< Timer 1 Overflow Flag

// TMR2CN0 (Timer 2 Control 0)
#define SFR_TMR2CN0 0xC8
SI_SBIT (TMR2CN0_T2XCLK,  SFR_TMR2CN0, 0); ///< Timer 2 External Clock Select
SI_SBIT (TMR2CN0_T2CSS,   SFR_TMR2CN0, 1); ///< Timer 2 Capture Source Select
SI_SBIT (TMR2CN0_TR2,     SFR_TMR2CN0, 2); ///< Timer 2 Run Control
SI_SBIT (TMR2CN0_T2SPLIT, SFR_TMR2CN0, 3); ///< Timer 2 Split Mode Enable
SI_SBIT (TMR2CN0_TF2CEN,  SFR_TMR2CN0, 4); ///< Timer 2 Capture Enable
SI_SBIT (TMR2CN0_TF2LEN,  SFR_TMR2CN0, 5); ///< Timer 2 Low Byte Interrupt Enable
SI_SBIT (TMR2CN0_TF2L,    SFR_TMR2CN0, 6); ///< Timer 2 Low Byte Overflow Flag
SI_SBIT (TMR2CN0_TF2H,    SFR_TMR2CN0, 7); ///< Timer 2 High Byte Overflow Flag

// TMR5CN0 (Timer 5 Control 0)
#define SFR_TMR5CN0 0xC8
SI_SBIT (TMR5CN0_T5XCLK,  SFR_TMR5CN0, 0); ///< Timer 5 External Clock Select
SI_SBIT (TMR5CN0_TR5,     SFR_TMR5CN0, 2); ///< Timer 5 Run Control
SI_SBIT (TMR5CN0_T5SPLIT, SFR_TMR5CN0, 3); ///< Timer 5 Split Mode Enable
SI_SBIT (TMR5CN0_TF5LEN,  SFR_TMR5CN0, 5); ///< Timer 5 Low Byte Interrupt Enable
SI_SBIT (TMR5CN0_TF5L,    SFR_TMR5CN0, 6); ///< Timer 5 Low Byte Overflow Flag
SI_SBIT (TMR5CN0_TF5H,    SFR_TMR5CN0, 7); ///< Timer 5 High Byte Overflow Flag

//------------------------------------------------------------------------------
// Interrupt Definitions
//------------------------------------------------------------------------------
#define INT0_IRQn    0  ///< External Interrupt 0
#define TIMER0_IRQn  1  ///< Timer 0 Overflow
#define INT1_IRQn    2  ///< External Interrupt 1
#define TIMER1_IRQn  3  ///< Timer 1 Overflow
#define UART0_IRQn   4  ///< UART 0
#define TIMER2_IRQn  5  ///< Timer 2 Overflow
#define SPI0_IRQn    6  ///< SPI0
#define SMBUS0_IRQn  7  ///< SMBus 0
#define USB0_IRQn    8  ///< USB0
#define ADC0WC_IRQn  9  ///< ADC0 Window Compare
#define ADC0EOC_IRQn 10 ///< ADC0 End of Conversion
#define PCA0_IRQn    11 ///< PCA0
#define CMP0_IRQn    12 ///< Comparator 0
#define CMP1_IRQn    13 ///< Comparator 1
#define TIMER3_IRQn  14 ///< Timer 3 Overflow
#define VBUSLVL_IRQn 15 ///< VBUS Level
#define UART1_IRQn   16 ///< UART 1
#define SMBUS1_IRQn  18 ///< SMBus 1
#define TIMER4_IRQn  19 ///< Timer 4 Overflow
#define TIMER5_IRQn  20 ///< Timer 5 Overflow

//------------------------------------------------------------------------------
// SFR Page Definitions
//------------------------------------------------------------------------------
#define LEGACY_PAGE 0x00 ///< Legacy SFR Page
#define CONFIG_PAGE 0x0F ///< System and Port Configuration Page
#define SMB1_PAGE   0x0F ///< SMBus 1 Page

//-----------------------------------------------------------------------------
// SDCC PDATA External Memory Paging Support
//-----------------------------------------------------------------------------

#if defined SDCC

SI_SFR(_XPAGE, 0xAA); // Point to the EMI0CN register

#endif

#endif  /* __SI_EFM8UB2_DEFS_H__ */

