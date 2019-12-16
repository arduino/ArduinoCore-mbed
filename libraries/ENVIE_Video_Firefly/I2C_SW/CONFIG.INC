; file: CONFIG.INC
; description: assembly language header file for software emulated I2C
;

FOSC    EQU     48000000  ; clock frequency of EFM8UB20F64G MCU: 48MHz

; Standard SFR Symbols
P0      DATA    80H

I2C_SCL EQU     P0.2
I2C_SDA EQU     P0.3

I2C_STANDARD_MODE   EQU     0
I2C_FAST_MODE       EQU     1
I2C_FAST_MODE_PLUS  EQU     0

