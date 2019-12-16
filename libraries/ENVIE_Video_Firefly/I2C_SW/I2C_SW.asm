; file: I2C_SW.asm
; description: software-emulated I2C implementation

        $INCLUDE (I2C_SW.inc)
        
        PUBLIC I2C1_Start
        PUBLIC I2C1_Restart
        PUBLIC _I2C1_WriteByte
        PUBLIC I2C1_ReadByte
        PUBLIC I2C1_Stop
        PUBLIC ?I2C1_ReadByte?BIT
        
        $REGUSE I2C1_Start()
        $REGUSE I2C1_Restart()
        $REGUSE _I2C1_WriteByte(A,C,R7)
        $REGUSE I2C1_ReadByte(A,C,R7)
        $REGUSE I2C1_Stop()
        
        I2C_SW_BIT_SEG SEGMENT BIT OVERLAYABLE
        RSEG I2C_SW_BIT_SEG
        ?I2C1_ReadByte?BIT:      DBIT    1
        
        I2C_SW_CODE_SEG SEGMENT CODE
        RSEG I2C_SW_CODE_SEG
;*******************************************************************************
; Function Name: I2C1_Start
; Description: 
; Parameter:
; Return: 
; Note: SCL and SDA should be HIGH before call I2C1_Start
;       SCL is LOW when return
;*******************************************************************************
        I2C1_Start:
        DELAY_T_BUF(2 + 8 + 7) ; bus free time before next start, SETB, RET, LCALL
        CLR     I2C_SDA
        DELAY_T_HD_STA(2) ; hold time start condition (t_HD:STA)
        CLR     I2C_SCL
        RET
;*******************************************************************************
; Function Name: I2C1_Restart
; Description: 
; Parameter:
; Return: 
; Note: SCL should be LOW before call I2C1_Restart
;*******************************************************************************
        I2C1_Restart:
        SETB    I2C_SDA ; ensure data is high
        DELAY_T_LOW(2 + 8 + 7 + 2)
        SETB    I2C_SCL
        DELAY_T_SU_STA(2) ; set-up time for a repeated START condition (tSU;STA)
        CLR     I2C_SDA ; the high->low transition
        DELAY_T_HD_STA(2);
        CLR     I2C_SCL
        RET
;*******************************************************************************
; Function Name: I2C_WriteByte
; Description: 
; Parameter: byte [in] 
; Return: ACK bit
; Note: SCL should be LOW before call I2C_WriteByte
;       SCL is LOW when return
;*******************************************************************************
        _I2C1_WriteByte:
        MOV     A, R7
        RLC     A
        MOV     I2C_SDA, C      ; bit 7
        DELAY_T_LOW(1 + 1 + 2)
        SETB    I2C_SCL
        RLC     A
        DELAY_T_HIGH(2 + 1)
        
        CLR     I2C_SCL
        DELAY_T_HD_DAT(2)
        MOV     I2C_SDA, C      ; bit 6
        DELAY_T_LOW(2 + 2)
        SETB    I2C_SCL
        RLC     A
        DELAY_T_HIGH(2 + 1)
        
        CLR     I2C_SCL
        DELAY_T_HD_DAT(2)
        MOV     I2C_SDA, C      ; bit 5
        DELAY_T_LOW(2 + 2)
        SETB    I2C_SCL
        RLC     A
        DELAY_T_HIGH(2 + 1)
        
        CLR     I2C_SCL
        DELAY_T_HD_DAT(2)
        MOV     I2C_SDA, C      ; bit 4
        DELAY_T_LOW(2 + 2)
        SETB    I2C_SCL
        RLC     A
        DELAY_T_HIGH(2 + 1)
        
        CLR     I2C_SCL
        DELAY_T_HD_DAT(2)
        MOV     I2C_SDA, C      ; bit 3
        DELAY_T_LOW(2 + 2)
        SETB    I2C_SCL
        RLC     A
        DELAY_T_HIGH(2 + 1)
        
        CLR     I2C_SCL
        DELAY_T_HD_DAT(2)
        MOV     I2C_SDA, C      ; bit 2
        DELAY_T_LOW(2 + 2)
        SETB    I2C_SCL
        RLC     A
        DELAY_T_HIGH(2 + 1)
        
        CLR     I2C_SCL
        DELAY_T_HD_DAT(2)
        MOV     I2C_SDA, C      ; bit 1
        DELAY_T_LOW(2 + 2)
        SETB    I2C_SCL
        RLC     A
        DELAY_T_HIGH(2 + 1)
        
        CLR     I2C_SCL
        DELAY_T_HD_DAT(2)
        MOV     I2C_SDA, C      ; bit 0
        DELAY_T_LOW(2 + 2)
        SETB    I2C_SCL
        DELAY_T_HIGH(2)
        
        CLR     I2C_SCL
        DELAY_T_HD_DAT(2)
        SETB    I2C_SDA ; change to input - listen for ack
        DELAY_T_LOW(2 + 2)
        SETB    I2C_SCL ; clk #9 for ack, float clock high
        MOV     C, I2C_SDA
        DELAY_T_HIGH(2 + 2)
        CLR     I2C_SCL
        RET
;*******************************************************************************
; Function Name: I2C1_ReadByte
; Description: 
; Parameter: ack [in]
; Return: 
; Note: SCL should be LOW before call I2C1_ReadByte
;       SCL is LOW when return
;*******************************************************************************
        I2C1_ReadByte:
        CLR     A
        SETB    I2C_SDA ; change to input
        DELAY_T_LOW(7 + 1 + 2)
        SETB    I2C_SCL
        MOV     C, I2C_SDA      ; bit 7
        DELAY_T_HIGH(2 + 2)
        
        CLR     I2C_SCL
        RLC     A
        DELAY_T_LOW(2 + 1)
        SETB    I2C_SCL
        MOV     C, I2C_SDA      ; bit 6
        DELAY_T_HIGH(2 + 2)
        
        CLR     I2C_SCL
        RLC     A
        DELAY_T_LOW(2 + 1)
        SETB    I2C_SCL
        MOV     C, I2C_SDA      ; bit 5
        DELAY_T_HIGH(2 + 2)
        
        CLR     I2C_SCL
        RLC     A
        DELAY_T_LOW(2 + 1)
        SETB    I2C_SCL
        MOV     C, I2C_SDA      ; bit 4
        DELAY_T_HIGH(2 + 2)
        
        CLR     I2C_SCL
        RLC     A
        DELAY_T_LOW(2 + 1)
        SETB    I2C_SCL
        MOV     C, I2C_SDA      ; bit 3
        DELAY_T_HIGH(2 + 2)
        
        CLR     I2C_SCL
        RLC     A
        DELAY_T_LOW(2 + 1)
        SETB    I2C_SCL
        MOV     C, I2C_SDA      ; bit 2
        DELAY_T_HIGH(2 + 2)
        
        CLR     I2C_SCL
        RLC     A
        DELAY_T_LOW(2 + 1)
        SETB    I2C_SCL
        MOV     C, I2C_SDA      ; bit 1
        DELAY_T_HIGH(2 + 2)
        
        CLR     I2C_SCL
        RLC     A
        DELAY_T_LOW(2 + 1)
        SETB    I2C_SCL
        MOV     C, I2C_SDA      ; bit 0
        DELAY_T_HIGH(2 + 2)
        
        CLR     I2C_SCL
        RLC     A
        MOV     C, ?I2C1_ReadByte?BIT     ; send acknowledge
        MOV     I2C_SDA, C
        DELAY_T_LOW(2 + 1 + 2 + 2)
        SETB    I2C_SCL ; clk #9 for ack
        MOV     R7, A
        DELAY_T_HIGH(2 + 1)
        
        CLR     I2C_SCL
        RET
;*******************************************************************************
; Function Name: I2C1_Stop
; Description: 
; Parameter: none
; Return: none
; Note: SCL should be LOW before call I2C1_Stop
;       SCL and SDA is HIGH when return
;*******************************************************************************
        I2C1_Stop:
        CLR     I2C_SDA ; ensure data is low first
        DELAY_T_LOW(2 + 8 + 7 + 2)
        SETB    I2C_SCL
        DELAY_T_SU_STO(2) ; set-up time for STOP condition (tSU;STO)
        SETB    I2C_SDA
        RET
;*******************************************************************************
        END
