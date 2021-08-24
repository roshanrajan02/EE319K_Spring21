// Sound.c
// This module contains the SysTick ISR that plays sound
// Runs on TM4C123
// Program written by: Roshan Rajan, Huy Nguyen
// Date Created: 3/6/17 
// Last Modified: 1/17/21 
// Lab number: 6
// Hardware connections
// TO STUDENTS "REMOVE THIS LINE AND SPECIFY YOUR HARDWARE********

// Code files contain the actual implemenation for public functions
// this file also contains an private functions and private data
#include <stdint.h>
#include "DAC.h"
#include "../inc/tm4c123gh6pm.h"

// 4-bit 32-element sine wave
uint8_t sinwave[32] = {
	8,9,11,12,13,14,14,15,15,15,14,
  14,13,12,11,9,8,7,5,4,3,2,
  2,1,1,1,2,2,3,4,5,7
};
	uint8_t ind = 0;
void Sound_Off(void);

// **************Sound_Init*********************
// Initialize digital outputs and SysTick timer
// Called once, with sound/interrupts initially off
// Input: none
// Output: none
void Sound_Init(void){
  // write this
	DAC_Init();
	NVIC_ST_CTRL_R = 0;
	NVIC_ST_CURRENT_R = 0;
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00ffffff)|0x20000000;
  
}

// **************Sound_Start*********************
// Start sound output, and set Systick interrupt period 
// Sound continues until Sound_Start called again, or Sound_Off is called
// This function returns right away and sound is produced using a periodic interrupt
// Input: interrupt period
//           Units of period to be determined by YOU
//           Maximum period to be determined by YOU
//           Minimum period to be determined by YOU
//         if period equals zero, disable sound output
// Output: none
void Sound_Start(uint32_t period){
  // write this
	if(period == 0){
		Sound_Off();
		return;
	}
	NVIC_ST_RELOAD_R = period  - 1; 
	NVIC_ST_CTRL_R = 7;
}

// **************Sound_Voice*********************
// Change voice
// EE319K optional
// Input: voice specifies which waveform to play
//           Pointer to wave table
// Output: none
void Sound_Voice(const uint8_t *voice){
  // optional
}
// **************Sound_Off*********************
// stop outputing to DAC
// Output: none
void Sound_Off(void){
  // write this
	NVIC_ST_CTRL_R = 0;
		return;
};
// **************Sound_GetVoice*********************
// Read the current voice
// EE319K optional
// Input: 
// Output: voice specifies which waveform to play
//           Pointer to current wavetable
const uint8_t *Sound_GetVoice(void){
 // write this
 // optional
  return 0;
}

// Interrupt service routine
// Executed every 12.5ns*(period)

void SysTick_Handler(void){
    // write this

	ind++;
	ind = ind & 0x0000001F;
	DAC_Out(sinwave[ind]); 
}