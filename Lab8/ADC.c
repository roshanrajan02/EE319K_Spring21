// ADC.c
// Runs on TM4C123
// Provide functions that initialize ADC0, channel 5, PD2

// Student names: Roshan Rajan
// Last modification date: 4/10/2021

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

// ADC initialization function 
// Initialize ADC for PD2, analog channel 5
// Input: sac sets hardware averaging
// Output: none
// Activating hardware averaging will improve SNR
// Activating hardware averaging will slow down conversion
// Max sample rate: <=125,000 samples/second
// Sequencer 0 priority: 1st (highest)
// Sequencer 1 priority: 2nd
// Sequencer 2 priority: 3rd
// Sequencer 3 priority: 4th (lowest)
// SS3 triggering event: software trigger
// SS3 1st sample source: Ain5 (PD2)
// SS3 interrupts: flag set on completion no interrupt requested
void ADC_Init(uint32_t sac){ 
		uint32_t delay;
		sac = 3;
		SYSCTL_RCGCADC_R |= 0x0001;
		SYSCTL_RCGCGPIO_R |=0x08;
		while((SYSCTL_PRGPIO_R&0x08) == 0){};
		GPIO_PORTD_DIR_R &= ~0x04;
		GPIO_PORTD_AFSEL_R |= 0x04;
		GPIO_PORTD_DEN_R &= ~0x04;
		GPIO_PORTD_AMSEL_R |= 0x04;
		SYSCTL_RCGCADC_R |= 0x01;
		delay = SYSCTL_RCGCADC_R;
    delay = SYSCTL_RCGCADC_R;
    delay = SYSCTL_RCGCADC_R;
    delay = SYSCTL_RCGCADC_R;
    ADC0_PC_R = 0x01;
    ADC0_SSPRI_R = 0x0123;
    ADC0_ACTSS_R &= ~0x0008;
    ADC0_EMUX_R &= ~0xF000;
    ADC0_SSMUX3_R = (ADC0_SSMUX3_R&0xFFFFFFF0) +5;
    ADC0_SSCTL3_R = 0x0006;
    ADC0_IM_R = ~0x0008;
    ADC0_ACTSS_R |= 0x0008;
		ADC0_SAC_R = sac;
}

//------------ADC_In------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
uint32_t ADC_In(void){  
  uint32_t data = 0; 
	ADC0_PSSI_R = 0x0008;
	while((ADC0_RIS_R&0x08)==0){};
	data = ADC0_SSFIFO3_R&0xFFF;
	ADC0_ISC_R = 0x0008;
	return data;
}


