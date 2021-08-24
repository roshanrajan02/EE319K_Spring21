;****************** main.s ***************
; Program written by: Valvano, solution
; Date Created: 2/4/2017
; Last Modified: 1/17/2021
; Brief description of the program
;   The LED toggles at 2 Hz and a varying duty-cycle
; Hardware connections (External: One button and one LED)
;  PE1 is Button input  (1 means pressed, 0 means not pressed)
;  PE2 is LED output (1 activates external LED on protoboard)
;  PF4 is builtin button SW1 on Launchpad (Internal) 
;        Negative Logic (0 means pressed, 1 means not pressed)
; Overall functionality of this system is to operate like this
;   1) Make PE2 an output and make PE1 and PF4 inputs.
;   2) The system starts with the the LED toggling at 2Hz,
;      which is 2 times per second with a duty-cycle of 30%.
;      Therefore, the LED is ON for 150ms and off for 350 ms.
;   3) When the button (PE1) is pressed-and-released increase
;      the duty cycle by 20% (modulo 100%). Therefore for each
;      press-and-release the duty cycle changes from 30% to 70% to 70%
;      to 90% to 10% to 30% so on
;   4) Implement a "breathing LED" when SW1 (PF4) on the Launchpad is pressed:
;      a) Be creative and play around with what "breathing" means.
;         An example of "breathing" is most computers power LED in sleep mode
;         (e.g., https://www.youtube.com/watch?v=ZT6siXyIjvQ).
;      b) When (PF4) is released while in breathing mode, resume blinking at 2Hz.
;         The duty cycle can either match the most recent duty-
;         cycle or reset to 30%.
;      TIP: debugging the breathing LED algorithm using the real board.
; PortE device registers
GPIO_PORTE_DATA_R  EQU 0x400243FC
GPIO_PORTE_DIR_R   EQU 0x40024400
GPIO_PORTE_AFSEL_R EQU 0x40024420
GPIO_PORTE_DEN_R   EQU 0x4002451C
; PortF device registers
GPIO_PORTF_DATA_R  EQU 0x400253FC
GPIO_PORTF_DIR_R   EQU 0x40025400
GPIO_PORTF_AFSEL_R EQU 0x40025420
GPIO_PORTF_PUR_R   EQU 0x40025510
GPIO_PORTF_DEN_R   EQU 0x4002551C
GPIO_PORTF_LOCK_R  EQU 0x40025520
GPIO_PORTF_CR_R    EQU 0x40025524
GPIO_LOCK_KEY      EQU 0x4C4F434B  ; Unlocks the GPIO_CR register
SYSCTL_RCGCGPIO_R  EQU 0x400FE608

       IMPORT  TExaS_Init
       THUMB
       AREA    DATA, ALIGN=2
;global variables go here
		

       AREA    |.text|, CODE, READONLY, ALIGN=2
       THUMB

       EXPORT  Start

Start
 ; TExaS_Init sets bus clock at 80 MHz
     BL  TExaS_Init
; voltmeter, scope on PD3
; Initialization goes here
; Turn the Clock on Port E and Port F
	LDR  R0, =SYSCTL_RCGCGPIO_R
	LDRB R1, [R0]
	ORR  R1, #0x30
	STRB R1, [R0]
;Wait for the clock to stabilize in 25 ns
	NOP
	NOP
;Set the direction of pins 1 and 2 in Port E and the direction of pin 4 on Port F
	LDR  R0, =GPIO_PORTE_DIR_R
	LDRB R1, [R0]
	AND  R1, #0xFC				;Checks if bit 2 is 1 already
	ORR  R1, #0x04				
	STRB R1, [R0]
	LDR  R0, =GPIO_PORTF_DATA_R
	LDRB R1, [R0]
	AND  R1, #0xEF				;Checks if bit 4 is 1 already
	STRB R1, [R0]
;Configure pins for Port E and Port F for Digital Function
	LDR  R0, =GPIO_PORTE_DEN_R
	LDRB R1, [R0]
	ORR  R1, #0x06
	STRB R1, [R0]
	LDR  R0, =GPIO_PORTF_DEN_R
	LDRB R1, [R0]
	ORR  R1, #0x10
	STRB R1, [R0]
;Enable Pull Up Resistor for Port F Pin 4 
	LDR  R0, =GPIO_PORTF_PUR_R
	LDRB R1, [R0]
	ORR  R1, #0x10
	STRB R1, [R0]
	CPSIE  I; TExaS voltmeter, scope runs on interrupts
	MOV  R11, #300			;sets inital values for ON duty cycle of Port E
	MOV  R12, #700			;sets initial value for OFF duty cycle of 
loop
		
; main engine goes here
		;initial check if Port F pin 4 is 1
		LDR  R0, =GPIO_PORTF_DATA_R
		LDR  R1, [R0]
		AND  R1, #0x10
		MOV  R3, #0
		CMP  R1, R3
		BNE  PortECheck
		;immediate check after to see if button has been released
		LDR  R0, =GPIO_PORTF_DATA_R
		LDR  R1, [R0]
		AND  R1, #0x10
		MOV  R3, #0
		CMP  R1, R3
		BNE  PortECheck		;continues to branch back to second check until button has released
		;initialize values used to implement breathing function
		MOV  R7, #1			;used to increment / decrement R5 
		MOV  R8, #-1		;used to increment / decrement R6
		MOV  R5, #0			;initial value for how many times ON Duty Cycle is run in Breathing Loop
		MOV  R6, #10		;initial value for how many times OFF Duty Cycle is run in Breathing Loop
Reset	MOV  R4, #14		;sets counter for how many times each variation of brightness is run within the loop
PF4On	;Turn Port E pin 2 on
		LDR  R0, =GPIO_PORTE_DATA_R
		LDRB R1, [R0]
		ORR  R1, #0x04
		STRB R1, [R0]
		;Run ON Duty cycle (R5) times
		MOV  R0, R5
OnDutyF	BL   delay
		SUBS R0, #1
		BGT  OnDutyF
		;Turn Port E pin 2 off
		LDR  R0, =GPIO_PORTE_DATA_R
		LDRB R1, [R0]
		AND  R1, #0xFB
		STRB R1, [R0]
		;Run OFF Duty Cycle (R6) times
		MOV  R0, R6
OfDutyF BL   delay
		SUBS R0, #1
		BGT  OfDutyF
		;Check if the counter equals 0
		SUBS R4, #1
		MOV  R0, #0
		CMP  R4, R0
		BNE  PF4On
		;Checks if R5 has reached 0, if R5 == 0, then begin incrementing R5 and decrementing R6
		MOV  R0, #0
		CMP  R5, R0
		BNE  NextCheck
		MOV  R7, #1
		MOV  R8, #-1
NextCheck			;Check if R5 has reached 10
		MOV  R0, #10
		CMP  R5, R0
		BNE  NoChange
		;Changes values in incrementing and decrementing so the light will get dimmer once reaching peak brightness
		MOV  R7, #-1
		MOV  R8, #1
NoChange
		ADD  R5, R7, R5
		ADD  R6, R8, R6
		;Checks if Port F pin 4 is still on
		LDR  R0, =GPIO_PORTF_DATA_R
		LDR  R1, [R0]
		AND  R1, #0x10
		MOV  R3, #0
		CMP  R1, R3
		BEQ  Reset
		
;Duty Cycle Incrementer
PortECheck
;initial check if PE1 is 1 (button has been pressed)
		LDR  R0, =GPIO_PORTE_DATA_R
		LDR  R1, [R0]
		AND  R1, #0x02
		MOV  R3, #0
		CMP  R1, R3
		BEQ  Skip
;check if PE1 is still 1 (if button has been released exit Check2 loop, else stay in loop until button has been released)
Check2	LDR  R0, =GPIO_PORTE_DATA_R
		LDR  R1, [R0]
		AND  R1, #0x02
		MOV  R3, #0
		CMP  R1, R3
		BNE  Check2
;check if value for ON duty cycle has reached 900, indicating that it should be reset back to 100 mow
		MOV  R10, #800
		CMP  R11, R10
		BLE  Increment
		MOV  R11, #100			;resets number of times ON duty cycle is run
		MOV  R12, #900			;resets number of times OFF duty cycle is run 
		B Skip
Increment
		ADD  R11, #200			;increments number of times ON duty cycle is run
		SUBS R12, #200			;decrements number of times OFF duty cycle is run
Skip	;turn on Port E	pin 2 
		LDR  R0, =GPIO_PORTE_DATA_R
		LDRB R1, [R0]
		ORR  R1, #0x04
		STRB R1, [R0]
		
		MOV  R0, R11
dutyE1	BL   delay
		SUBS R0, #1
		BNE  dutyE1
		;turn off Port E pin 2 
		LDR  R0, =GPIO_PORTE_DATA_R
		LDRB R1, [R0]
		AND  R1, #0xFB
		STRB R1, [R0]
		
		MOV  R0, R12
dutyE2	BL   delay
		SUBS R0, #1
		BNE  dutyE2
		
		B    loop
    
delay
		PUSH {R0, LR}
		MOV R0, #10000
Augment SUBS R0, #1
		BNE Augment
		POP {R0, LR}
		BX LR
		
     ALIGN      ; make sure the end of this section is aligned
     END        ; end of file

