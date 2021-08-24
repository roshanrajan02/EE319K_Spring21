// Lab8.c
// Runs on TM4C123
// Student names: Roshan Rajan
// Last modification date: 04/12/2021
// Last Modified: 1/17/2021 

// Specifications:
// Measure distance using slide pot, sample at 10 Hz
// maximum distance can be any value from 1.5 to 2cm
// minimum distance is 0 cm
// Calculate distance in fixed point, 0.01cm
// Analog Input connected to PD2=ADC5
// displays distance on SSD1306
// PF3, PF2, PF1 are heartbeats (use them in creative ways)
// 

#include <stdint.h>
#include "SSD1306.h"
#include "TExaS.h"
#include "ADC.h"
#include "print.h"
#include "../inc/tm4c123gh6pm.h"
#include "../inc/CortexM.h"

//*****the first three main programs are for debugging *****
// main1 tests just the ADC and slide pot, use debugger to see data
// main2 adds the LCD to the ADC and slide pot, ADC data is on ST7735
// main3 adds your convert function, position data is no ST7735

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))
#define PF4       (*((volatile uint32_t *)0x40025040))

// **************SysTick_Init*********************
// Initialize Systick periodic interrupts
// Input: interrupt period
//        Units of period are 12.5ns
//        Maximum is 2^24-1
//        Minimum is determined by length of ISR
// Output: none
void SysTick_Init(unsigned long period){
  // write this
	NVIC_ST_CTRL_R = 0;                   // disable SysTick during setup
  NVIC_ST_RELOAD_R = period;  			
  NVIC_ST_CURRENT_R = 0;                // any write to current clears it
  NVIC_ST_CTRL_R = 0x7;                 // enable SysTick with core clock
}

// Initialize Port F so PF1, PF2 and PF3 are heartbeats
void PortF_Init(void){
  SYSCTL_RCGCGPIO_R |= 0x20;      // activate port F
  while((SYSCTL_PRGPIO_R&0x20) != 0x20){};
  GPIO_PORTF_DIR_R |=  0x0E;   // output on PF3,2,1 (built-in LED)
  GPIO_PORTF_PUR_R |= 0x10;
  GPIO_PORTF_DEN_R |=  0x1E;   // enable digital I/O on PF
}
int32_t Data;         // 12-bit ADC
uint32_t Position;    // 32-bit fixed-point 0.01 cm
// use main1 to test ADC hardware and ADC software
int main1(void){      // single step this program and look at Data
  TExaS_Init();       // Bus clock is 80 MHz 
  ADC_Init(SAC_NONE); // turn on ADC, set channel to 5
  while(1){                
    Data = ADC_In();  // sample 12-bit channel 5
  }
}
// your function to convert ADC sample to distance (0.01cm)
//**********place your calibration data here*************
// distance PD2       ADC  fixed point
// 0.00cm   0.000V     0        0
// 0.50cm   0.825V  1024      500
// 1.00cm   1.650V  2048     1000
// 1.50cm   2.475V  3072     1500  
// 2.00cm   3.300V  4095     2000  
uint32_t Convert(uint32_t data){
  return ((192 * data)/4096) ; // replace this line with your Lab 8 solution
}
uint32_t startTime,stopTime;
uint32_t ADCtime,OutDectime,ConvertTime,OutFixTime; // in usec
// use main2 to measure execution times
// use main2 to choose the sac value for your system
// use main2 to calibrate position as a function of ADC data
int main2(void){
  TExaS_Init();       // Bus clock is 80 MHz
  NVIC_ST_RELOAD_R = 0x00FFFFFF; // maximum reload value
  NVIC_ST_CURRENT_R = 0;          // any write to current clears it
  NVIC_ST_CTRL_R = 5;
  ADC_Init(SAC_32);  // turn on ADC, set channel to 5
 // 32-point averaging
  SSD1306_Init(SSD1306_SWITCHCAPVCC);
  SSD1306_OutClear(); 
  while(1){  // use SysTick to measure execution times 
    SSD1306_SetCursor(0,0);
    SSD1306_OutString("Lab 8, main2\nsac=5"); 
    // ----measure ADC conversion time----
    startTime = NVIC_ST_CURRENT_R;
    Data = ADC_In();  // sample 12-bit channel 5
    stopTime = NVIC_ST_CURRENT_R;
    ADCtime = ((startTime-stopTime)&0x0FFFFFF)/80;    // usec
    SSD1306_SetCursor(0,2);
    // ----measure LCD_OutDec time----
    startTime = NVIC_ST_CURRENT_R;
    LCD_OutDec(Data);
    SSD1306_OutString("   ");  // spaces cover up characters from last output
    stopTime = NVIC_ST_CURRENT_R;
    OutDectime = ((startTime-stopTime)&0x0FFFFFF)/80; // usec
    // ----measure Convert time----
    startTime = NVIC_ST_CURRENT_R;
    Position = Convert(Data);  // your Lab 8 program
    stopTime = NVIC_ST_CURRENT_R;
    ConvertTime = ((startTime-stopTime)&0x0FFFFFF)/80; // usec 
    // ----measure LCD_OutFix time----
    startTime = NVIC_ST_CURRENT_R;
    LCD_OutFix(Position);
    stopTime = NVIC_ST_CURRENT_R;
    OutFixTime = ((startTime-stopTime)&0x0FFFFFF)/80; // usec 
    //------show results
    SSD1306_SetCursor(0,3); SSD1306_OutString("ADC(us)= ");     LCD_OutDec(ADCtime);
    SSD1306_SetCursor(0,4); SSD1306_OutString("OutDec(us)= ");  LCD_OutDec(OutDectime);SSD1306_OutString("  ");
    SSD1306_SetCursor(0,5); SSD1306_OutString("Convert(us)= "); LCD_OutDec(ConvertTime);
    SSD1306_SetCursor(0,6); SSD1306_OutString("OutFix(us)= ");  LCD_OutDec(OutFixTime);
  }
}
// use main3 to determine 
// See the Lab8_Accuracy  section in chapter 14 of the ebook
// http://users.ece.utexas.edu/%7Evalvano/Volume1/E-Book/C14_ADCdataAcquisition.htm
// Collect five measurements with your distance measurement system.
int main3(void){
  TExaS_Init();     // Bus clock is 80 MHz
  SSD1306_Init(SSD1306_SWITCHCAPVCC);
  SSD1306_OutClear(); 
  SSD1306_OutString("Lab 8, main3");  
  ADC_Init(SAC_32);  // turn on ADC, set channel to 5
 // 32-point averaging
  while(1){  
    Data = ADC_In();  // sample 12-bit channel 5
    Position = Convert(Data);
    SSD1306_SetCursor(0,1); // second row
    LCD_OutDec(Data); SSD1306_OutString("   ");
    SSD1306_SetCursor(0,2); // third row
    LCD_OutFix(Position);   // your Lab 7 solution
  }
}


int MailStatus;
uint32_t MailValue;
#define MS 80000
void SysTick_Handler(void){ // every 100 ms
  PF1 ^= 0x02;     // Heartbeat
  // write this
	SSD1306_SetCursor(0,0);
	MailValue = ADC_In();
	MailStatus = 1;
}
// final main program to create distance meter
// put your final lab8 main here
int main(void){  
  DisableInterrupts();
  TExaS_Init();    // bus clock at 80 MHz
  SSD1306_Init(SSD1306_SWITCHCAPVCC);
  ADC_Init(SAC_32);  // turn on ADC, set channel to 5
 // 32-point averaging  
  PortF_Init();
	SysTick_Init(8000000);
  // other initialization, like mailbox
  EnableInterrupts();
  while(1){
    // wait on mailbox
		while(MailStatus == 0){};
    PF3 ^= 0x08;       // Heartbeat
    MailStatus = 0;
		Position = Convert(MailValue);
		LCD_OutFix(Position);
  }
}

uint32_t Histogram[64]; // probability mass function
uint32_t Center;
// use main4 program to demonstrate CLT
int main4(void){ uint32_t i,d,sum,sac; 
  DisableInterrupts();
  TExaS_Init();    // bus clock at 80 MHz
  SSD1306_Init(SSD1306_SWITCHCAPVCC);
  ADC_Init(SAC_NONE);  // turn on ADC, set channel to 5
 // 32-point averaging  
  PortF_Init();
  while(1){
    for(sac=0; sac<7; sac++){
      ADC0_SAC_R = sac;               // 1 to 64 hardware averaging
      SSD1306_OutClear(); 
      sum = 0;
      for(i=0; i<100; i++){
        sum += ADC_In(); 
      }
      Center = sum/100; // centering allows us to zoom in
      LCD_OutDec(sac);
      for(i=0; i<128; i++) Histogram[i] = 0; // clear
      for(i=0; i<400; i++){
        Data = ADC_In();
        PF3 ^= 0x08;       // Heartbeat
        if(Data < (Center-32)){
          Histogram[0]++;
        }else if(Data >= (Center+32)){
          Histogram[63]++;
        }else{
          d = Data-Center+32;
          Histogram[d]++;
        }
        MailStatus = 0;    // mailbox is empty
      }
      SSD1306_SetPlot(0,127,0,149,SSD1306_WHITE);
      SSD1306_ClearBuffer();
      for(i=0; i<64; i++){
        if(Histogram[i] > 149) Histogram[i]=149;
        for(int j=0; j<Histogram[i];j++){
          SSD1306_DrawPoint(2*i,j);
          SSD1306_DrawPoint(2*i+1,j);
        }
      }
      SSD1306_OutBuffer();   // 30ms, update entire screen
      SSD1306_SetCursor(0,0);
      SSD1306_OutString("CLT sac="); LCD_OutDec(sac);
      while(PF4){}; // wait for touch
      SSD1306_OutClear();        
      while(PF4==0){}; // wait for release      
    }
  }
}
// main5 is graphing version
// sampling rate fs is about 4Hz
// use main5 to study the Nyquist Theorem
// wiggle the slide pot f oscillations/sec
// how fast can you wiggle it and still see oscillations?
int main5(void){ uint32_t time; 
  DisableInterrupts();
  TExaS_Init();    // bus clock at 80 MHz
  SSD1306_Init(SSD1306_SWITCHCAPVCC);
  ADC_Init(SAC_32);  // turn on ADC, set channel to 5
 // 32-point averaging  
  PortF_Init();
  SSD1306_SetPlot(0,127,0,200,SSD1306_WHITE);
  SSD1306_ClearBuffer();
  time = 0;
  EnableInterrupts();
  while(1){
    Clock_Delay1ms(227); // approximately 4Hz
    PF3 ^= 0x08;       // Heartbeat
    Data = ADC_In();
    Position = Convert(Data); 
    SSD1306_DrawPoint(time,Position);
    SSD1306_OutBuffer();   // 30ms, update entire screen
    time = time+1;
    if(time == 128){
      SSD1306_ClearBuffer();
      time = 0;
    }    
  }
}



