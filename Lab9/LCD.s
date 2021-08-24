; LCD.s
; Student names: change this to your names or look very silly
; Last modification date: change this to the last modification date or look very silly

; Runs on TM4C123
; Use I2C3 to send data to SSD1306 128 by 64 pixel oLED

; As part of Lab 7, students need to implement I2C_Send2

      EXPORT   I2C_Send2
      PRESERVE8
      AREA    |.text|, CODE, READONLY, ALIGN=2
      THUMB
      ALIGN
I2C3_MSA_R  EQU 0x40023000
I2C3_MCS_R  EQU 0x40023004
I2C3_MDR_R  EQU 0x40023008
; sends two bytes to specified slave
; Input: R0  7-bit slave address
;        R1  first 8-bit data to be written.
;        R2  second 8-bit data to be written.
; Output: 0 if successful, nonzero (error code) if error
; Assumes: I2C3 and port D have already been initialized and enabled
I2C_Send2
;; --UUU-- 
; 1) wait while I2C is busy, wait for I2C3_MCS_R bit 0 to be 0
; 2) write slave address to I2C3_MSA_R, 
;     MSA bits7-1 is slave address
;     MSA bit 0 is 0 for send data
; 3) write first data to I2C3_MDR_R
; 4) write 0x03 to I2C3_MCS_R,  send no stop, generate start, enable
; add 4 NOPs to wait for I2C to start transmitting
; 5) wait while I2C is busy, wait for I2C3_MCS_R bit 0 to be 0
; 6) check for errors, if any bits 3,2,1 I2C3_MCS_R is high 
;    a) if error set I2C3_MCS_R to 0x04 to send stop 
;    b) if error return R0 equal to bits 3,2,1 of I2C3_MCS_R, error bits
; 7) write second data to I2C3_MDR_R
; 8) write 0x05 to I2C3_MCS_R, send stop, no start, enable
; add 4 NOPs to wait for I2C to start transmitting
; 9) wait while I2C is busy, wait for I2C3_MCS_R bit 0 to be 0
; 10) return R0 equal to bits 3,2,1 of I2C3_MCS_R, error bits
;     will be 0 is no error
          PUSH {R4, R5}



loopbusywait0
    LDR R4, =I2C3_MCS_R
    LDRB R4, [R4]
    AND R4, R4, #0x01 
    CMP R4, #0
    BNE loopbusywait0


    LSL R4, R0, #1 
    LDR R5, =I2C3_MSA_R
    STR R4, [R5]; 

    LDR R4, =I2C3_MDR_R
    STRB R1, [R4] 


    MOV R4, #0x03
    LDR R5, =I2C3_MCS_R
    STR R4, [R5]


    NOP
    NOP
    NOP
    NOP


loopbusywait1
    LDR R4, =I2C3_MCS_R
    LDRB R4, [R4]
    AND R4, R4, #0x0001 
    CMP R4, #0
    BNE loopbusywait1


    LDR R4, =I2C3_MCS_R
    LDRB R4, [R4]
    AND R4, #0x0E 
    LSR R4, #1 ; 
    CMP R4, #0
    BEQ seconddata
    MOV R5, #0x04
    LDR R6, =I2C3_MCS_R
    STRB R5, [R6] 
    MOV R0, R4
    B seconddata


seconddata
    LDR R4, =I2C3_MDR_R
    STRB R2, [R4]


    MOV R4, #0x05
    LDR R5, =I2C3_MCS_R
    STRB R4, [R5]


    NOP
    NOP
    NOP
    NOP


loopbusywait2
    LDR R4, =I2C3_MCS_R
    LDRB R4, [R4]
    AND R4, R4, #0x0001
    CMP R4, #0
    BNE loopbusywait2


    LDR R4, =I2C3_MCS_R
    LDRB R4, [R4]
    AND R4, #0x0E 
    LSR R4, #1 
    MOV R0, R4

    POP {R4, R5}

    BX  LR                          ;   return


    ALIGN                           ; make sure the end of this section is aligned
    END                             ; end of file
 