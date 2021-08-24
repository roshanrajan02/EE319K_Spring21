// ADC.c
// Runs on TM4C123
// Provide functions that initialize ADC0
// Last Modified: 1/16/2021
// Student names: change this to your names or look very silly
// Last modification date: change this to the last modification date or look very silly

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"


// ADC initialization function 
// Input: none
// Output: none
// measures from PD2, analog channel 5

void ADC_Init(uint8_t p){ 
     SYSCTL_RCGCGPIO_R |= 0x08;
//Wait for clock to stabilize
    while((SYSCTL_RCGCGPIO_R&0x08) == 0) {};
//Setup PD2
    GPIO_PORTD_DEN_R &= ~0x04;
    GPIO_PORTD_AFSEL_R |= 0x04;
    GPIO_PORTD_AMSEL_R |= 0x04;
    GPIO_PORTD_DIR_R &= ~0x04;
//Initialize ADC
    volatile unsigned long delay;
    SYSCTL_RCGCADC_R |= 0x01;         // 6) activate ADC0 
  delay = SYSCTL_RCGCADC_R;         // extra time to stabilize
  delay = SYSCTL_RCGCADC_R;         // extra time to stabilize
  delay = SYSCTL_RCGCADC_R;         // extra time to stabilize
  delay = SYSCTL_RCGCADC_R;         // extra time to stabilize
  ADC0_PC_R      = 0x01;            // 7) configure for 125K 
  ADC0_SSPRI_R      = 0x0123;          // 8) Seq 3 is highest priority
  ADC0_ACTSS_R  &= ~0x0008;         // 9) disable sample sequencer 3
  ADC0_EMUX_R   &= ~0xF000;         // 10) seq3 is software trigger
  ADC0_SSMUX3_R  = (ADC0_SSMUX3_R & 0xFFFFFFF0)+5;  // 11) Ain9 (PE4)
  ADC0_SSCTL3_R  = 0x0006;          // 12) no TS0 D0, yes IE0 END0
  ADC0_IM_R     &= ~0x0008;         // 13) disable SS3 interrupts
  ADC0_ACTSS_R  |= 0x0008;          // 14) enable sample sequencer 3


}

//------------ADC_In------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
// measures from PD2, analog channel 5
uint32_t ADC_In(void){ 

    uint32_t data = 0;
    ADC0_PSSI_R = 0x0008;
    while((ADC0_RIS_R & 0x08) == 0) {};

//Grab Data from ADC

    data = ADC0_SSFIFO3_R&0xFFF;
    ADC0_ISC_R = 0x0008;
  return data; 
}
