// Lab10.c
// Runs on TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the EE319K Lab 10

// Last Modified: 1/16/2021 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php
/* 
 Copyright 2021 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bi-t 3 on PB3 (most significant bit)
// LED on PB4
// LED on PB5

// VCC   3.3V power to OLED
// GND   ground
// SCL   PD0 I2C clock (add 1.5k resistor from SCL to 3.3V)
// SDA   PD1 I2C data

//************WARNING***********
// The LaunchPad has PB7 connected to PD1, PB6 connected to PD0
// Option 1) do not use PB7 and PB6
// Option 2) remove 0-ohm resistors R9 R10 on LaunchPad
//******************************

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/CortexM.h"
#include "SSD1306.h"
#include "Print.h"
#include "Random.h"
#include "ADC.h"
#include "Images.h"
#include "Timer0.h"
#include "Timer1.h"
#include "TExaS.h"
#include "Switch.h"
#include "Sound.h"
#define PE3210 			(*((volatile uint32_t *)0x4002403C)) // bits 3,2,1,0
//********************************************************************************
// debuging profile, pick up to 7 unused bits and send to Logic Analyzer
#define PB54                  (*((volatile uint32_t *)0x400050C0)) // bits 5-4
#define PF321                 (*((volatile uint32_t *)0x40025038)) // bits 3-1
// use for debugging profile
#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))
#define PB5       (*((volatile uint32_t *)0x40005080)) 
#define PB4       (*((volatile uint32_t *)0x40005040)) 
extern int status;
// TExaSdisplay logic analyzer shows 7 bits 0,PB5,PB4,PF3,PF2,PF1,0 
// edit this to output which pins you use for profiling
// you can output up to 7 pins
void LogicAnalyzerTask(void){
  UART0_DR_R = 0x80|PF321|PB54; // sends at 10kHz
}
void ScopeTask(void){  // called 10k/sec
  UART0_DR_R = (ADC1_SSFIFO3_R>>4); // send ADC to TExaSdisplay
}
// edit this to initialize which pins you use for profiling
void Profile_Init(void){
  SYSCTL_RCGCGPIO_R |= 0x33;      // activate port B,F, A, E
  while((SYSCTL_PRGPIO_R&0x33) != 0x33){};
  GPIO_PORTF_DIR_R |=  0x0E;   // output on PF3,2,1 
  GPIO_PORTF_DEN_R |=  0x0E;   // enable digital I/O on PF3,2,1
  GPIO_PORTB_DIR_R |=  0x30;   // output on PB4 PB5
  GPIO_PORTB_DEN_R |=  0x30;   // enable on PB4 PB5
	
	GPIO_PORTB_DIR_R |=  0x1C;   // output on PA2 PA3 PA4
  GPIO_PORTB_DEN_R |=  0x1C;   // enable on PA2 PA3 PA4
	GPIO_PORTA_PCTL_R &= ~0x000FFF00;
}
//********************************************************************************
 
void Delay100ms(uint32_t count); // time delay in 0.1 seconds

typedef enum {dead, alive} status_t;
struct sprite {
	int32_t x;	// x cor 0 and 127
	int32_t y; 	// y cor 0 and 63
	int32_t vx;
	int32_t vy;
	const uint8_t *image1; //ptr-->image1

	status_t life; // dead/alive
};


typedef struct sprite sprite_t;

sprite_t player;
sprite_t fruit[5];
sprite_t sidebirds[2];
sprite_t bombs[2];
int NeedToDraw;
void Init(void) { int i;
	
	for(i=0;i<6;i++){
		fruit[i].x = 20*i;
		fruit[i].y = 20;
		fruit[i].image1 = berry; // array.Random(0,2)
		fruit[i].life = alive;
	}
	for(i=6;i<5;i++){
		fruit[i].life = dead;
	}

		
	player.x = 55;
	player.y = 60;
	player.image1 = basket;
	player.life = alive;
	
}
int score;

void Collisions(void){
		int i;
    uint32_t x1,y1,x2,y2;
    x2 = player.x;
    y2 = player.y;

    for(i = 0; i < 5; i++){
        if (fruit[i].life == alive){
            x1 = fruit[i].x + 1; // init 3
            y1 = fruit[i].y + 1; //init 3
            if((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2) < 50){
                fruit[i].life = dead;
								if(fruit[i].image1 == apple)
									score += 3;
								if(fruit[i].image1 == cherry)
									score += 5;
								if(fruit[i].image1 == berry)
									score += 1;
                return;
            }
        }
    }
		
		for(i = 0; i < 2; i++){
        if (sidebirds[i].life == alive){
            x1 = sidebirds[i].x;
            y1 = sidebirds[i].y;
            if((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2) < 225){
                player.life = dead;
								Sound_Play(GameOver,17500);
                return;
            }
        }
    }
		
		for(i = 0; i < 2; i++){
				if (bombs[i].life == alive){
						x1 = bombs[i].x + 3;
						y1 = bombs[i].y + 3;
						if((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2) < 50){
								Sound_Play(GameOver,17500);
                player.life = dead;
						}
				}
		}
		
		SSD1306_SetCursor(13, 0);
		SSD1306_OutString("Score:");
		SSD1306_SetCursor(19,0);
		SSD1306_OutUDec2(score);
}

void Move(void) {uint32_t adcData; int i;
	for(i=0; i<5; i++){
		if(fruit[i].life == alive){
			NeedToDraw = 1;
			if(fruit[i].y < 62){
				fruit[i].y += 1;
			}else{
				fruit[i].life = dead;
			}
		}
	}
	
		for(i = 0; i < 2; i++){
        if(sidebirds[i].life == alive){
          NeedToDraw = 1;
          if(sidebirds[i].x > 1){
            sidebirds[i].x -=1;
          }
          else{
              sidebirds[i].life = dead;
          }
        }
		}
		for(i=0; i<2; i++){
			if(bombs[i].life == alive){
				NeedToDraw = 1;
				if(bombs[i].y < 62){
					bombs[i].y += 1;
				}	
				else{
					bombs[i].life = dead;
				}
			}
		}
		
		
	
	if(player.life == alive){
		NeedToDraw = 1;
		adcData = ADC_In(); // 0 to 4095
		player.x = ((127-19)*adcData)/4096;
	}
	
}
void MoveUp(void){
	if(player.y > 3){
	player.y -= 3;
	}
}

void MoveDown(void){
	if(player.y < 60){
	player.y += 3;
	}
}

const unsigned char *images[] = {apple, berry, cherry}; 

void Bird_Init(){
	uint32_t n;
	uint32_t i;
	n = Random();

	n = Random();
    if(n < 10){
        n = Random();
        n = n/4;
        i = 0;
        while((sidebirds[i].life == alive) && (i < 2)){
            i++;
        }
        if(i < 2){
            sidebirds[i].life = alive;
            sidebirds[i].x = 127;
            sidebirds[i].y = n;
            sidebirds[i].image1 = bird;
        }
    }


}

void Fruit_Init(){
	uint32_t n;
	uint32_t i;
	n = Random();
	uint8_t temp;
	temp = Random()%3;
	
	if(n < 20){
		n = Random(); //0 to 255
		n = n/2; // 0 to 127
		i = 0;
		while((fruit[i].life == alive) && (i < 5)){
			i++;
		}
		if(i < 5){
			fruit[i].life = alive;
			fruit[i].x = n;
			fruit[i].y = 0;
			fruit[i].image1 = images[temp];
		}
	}
}

void Bomb_Init(){
	uint32_t n;
	uint32_t i;
	n = Random();
	
	if(n < 20){
		n = Random(); //0 to 255
		n = n/2; // 0 to 127
		i = 0;
		while((bombs[i].life == alive) && (i < 2)){
			i++;
		}
		if(i < 2){
			bombs[i].life = alive;
			bombs[i].x = n;
			bombs[i].y = 0;
			bombs[i].image1 = bomb;
		}
	}
}

void SysTick_Handler(void){
	Bird_Init();
	Fruit_Init();
	Bomb_Init();
	static uint32_t lastup = 0;
	uint32_t up = Switch_In()&0x04; // PE2
	if((up==0x04)&&(lastup == 0)){
		MoveUp();
	}
	static uint32_t lastdown = 0;
	uint32_t down = Switch_In()&0x01; // PE0
	if((down==0x01)&&(lastdown == 0)){
		MoveDown();
	}
	Move();
	Collisions();
	lastdown = down;
	lastup = up;
}

void SysTick_Wait(uint32_t delay){
  volatile uint32_t elapsedTime;
  uint32_t startTime = NVIC_ST_CURRENT_R;
  do{
    elapsedTime = (startTime-NVIC_ST_CURRENT_R)&0x00FFFFFF;
  }
  while(elapsedTime <= delay);
}

void SysTick_Wait10ms(uint32_t delay){
  uint32_t i;
  for(i=0; i<delay; i++){
    SysTick_Wait(500000);  // wait 10ms (assumes 50 MHz clock)
  }
}


void Draw(void){ int i;
	SSD1306_ClearBuffer();
	if(player.life ==alive){
		SSD1306_DrawBMP(player.x, player.y,
				player.image1, 0, SSD1306_INVERSE);
	}
	
	for(i=0; i<5; i++){
		if(fruit[i].life ==alive){
		SSD1306_DrawBMP(fruit[i].x, fruit[i].y,
			fruit[i].image1, 0, SSD1306_INVERSE);
		}
	}
	for(i=0; i<2; i++){
		if(sidebirds[i].life ==alive){
		SSD1306_DrawBMP(sidebirds[i].x, sidebirds[i].y,
			sidebirds[i].image1, 0, SSD1306_INVERSE);
	}

	}
	for(i=0; i<2; i++){
		if(bombs[i].life ==alive){
		SSD1306_DrawBMP(bombs[i].x, bombs[i].y,
			bombs[i].image1, 0, SSD1306_INVERSE);
	}

	}

	SSD1306_OutBuffer();
	NeedToDraw = 0;
	
}

void SysTick_Init20Hz(void);
int status;
int select;
int main(void){
	score = 0;
	DisableInterrupts();
  // pick one of the following three lines, all three set to 80 MHz
  //PLL_Init();                   // 1) call to have no TExaS debugging
  TExaS_Init(&LogicAnalyzerTask); // 2) call to activate logic analyzer
  //TExaS_Init(&ScopeTask);       // 3) call to activate analog scope PD2
  SSD1306_Init(SSD1306_SWITCHCAPVCC);
  SSD1306_OutClear();   
  Random_Init(1);
  Profile_Init(); // PB5,PB4,PF3,PF2,PF1 
	SysTick_Init20Hz();
	ADC_Init(4);
	Sound_Init();
	Switch_Init();
	Init();
	
	SSD1306_ClearBuffer();
	//Title Screen
	SSD1306_DrawBMP(12, 57, TrialHome1, 0, SSD1306_WHITE);
	SSD1306_DrawBMP(90, 60, cherry, 0, SSD1306_WHITE);
	SSD1306_DrawBMP(10, 50, apple, 0, SSD1306_WHITE);
	SSD1306_DrawBMP(60, 20, berry, 0, SSD1306_WHITE);
	//Title Screen
	
	SSD1306_OutBuffer();
	SysTick_Wait10ms(350);
//Language Options Screen
	SSD1306_ClearBuffer();
	SSD1306_OutClear();
	 SSD1306_SetCursor(1, 1);
  SSD1306_OutString("Language Options");
	SSD1306_SetCursor(1, 3);
  SSD1306_OutString("PE0 for English");
	SSD1306_SetCursor(1, 5);
  SSD1306_OutString("PE2 for Spanish");
	SysTick_Wait10ms(350);
	while(Switch_In() == 0){};
	uint32_t pe0 = Switch_In() & 0x01;
	uint32_t pe2 = Switch_In() & 0x04;
	if (pe0 == 0x01 && pe2 != 0x04){
	//Fruits Screen
	select = 1;
	SSD1306_ClearBuffer();
	SSD1306_DrawBMP(105, 63, cherry, 0, SSD1306_WHITE);
	SSD1306_DrawBMP(110, 47, apple, 0, SSD1306_WHITE);
	SSD1306_DrawBMP(100, 29, berry, 0, SSD1306_WHITE);
	SSD1306_OutBuffer();	

  SSD1306_SetCursor(1, 1);
  SSD1306_OutString("FRUITS");
	SSD1306_SetCursor(1, 3);
  SSD1306_OutString("Berry = 1 point");
	SSD1306_SetCursor(1, 5);
  SSD1306_OutString("Apple = 3 points");
	SSD1306_SetCursor(1, 7);
  SSD1306_OutString("Cherry = 5 points");
	//Fruits Screen
	SysTick_Wait10ms(350);
	SSD1306_OutClear();  
	SSD1306_ClearBuffer();
	
	//Rules Screen
	SSD1306_SetCursor(0, 1);
	SSD1306_OutString("RULES:");
	SSD1306_SetCursor(0, 3);
  SSD1306_OutString("Catch as many fruit");
	SSD1306_SetCursor(0, 4);
  SSD1306_OutString("as you can while");
	SSD1306_SetCursor(0, 5);
	SSD1306_OutString("avoiding the");
	SSD1306_SetCursor(0, 6);
  SSD1306_OutString("birds and bombs.");
	//Rules Screen
	SysTick_Wait10ms(350);
	//Rules Continued Screen
	SSD1306_OutClear();  
	SSD1306_ClearBuffer();
	SSD1306_DrawBMP(33, 47, bomb, 0, SSD1306_WHITE);
	SSD1306_DrawBMP(30, 63, bird , 0, SSD1306_WHITE);
	SSD1306_OutBuffer();
	SSD1306_SetCursor(0, 1);
  SSD1306_OutString("RULES (Continued):");
	SSD1306_SetCursor(0, 2);
  SSD1306_OutString("If you touch a bomb ");
	SSD1306_SetCursor(0, 3);
  SSD1306_OutString("or bird, you LOSE. ");
	SSD1306_SetCursor(0, 5);
  SSD1306_OutString("Bomb:");
	SSD1306_SetCursor(0, 7);
  SSD1306_OutString("Bird:");
	//Rules Continued Screen
	SysTick_Wait10ms(350);
	
	SSD1306_OutClear();
	SSD1306_ClearBuffer();
	
	//CountDown
	SSD1306_SetCursor(10, 4);
	SSD1306_OutString("5");
	SysTick_Wait10ms(100);
	SSD1306_SetCursor(10, 4);
	SSD1306_OutString("4");
	SysTick_Wait10ms(100);
	SSD1306_SetCursor(10, 4);
	SSD1306_OutString("3");
	SysTick_Wait10ms(100);
	SSD1306_SetCursor(10, 4);
	SSD1306_OutString("2");
	SysTick_Wait10ms(100);
	SSD1306_SetCursor(10, 4);
	SSD1306_OutString("1");
	SysTick_Wait10ms(100);
	EnableInterrupts();
	
	
  while(player.life == alive){
		if(NeedToDraw) { 
			Draw();
		}	
  }
	DisableInterrupts();
	SSD1306_ClearBuffer();
	SSD1306_OutClear();
	SSD1306_SetCursor(6,4);
	SSD1306_OutString("Score:");
	SSD1306_SetCursor(12,4);
	SSD1306_OutUDec2(score);
	}
	else{
	select = 0;
	//Fruits Screen
	SSD1306_ClearBuffer();
	SSD1306_DrawBMP(105, 63, cherry, 0, SSD1306_WHITE);
	SSD1306_DrawBMP(110, 47, apple, 0, SSD1306_WHITE);
	SSD1306_DrawBMP(100, 29, berry, 0, SSD1306_WHITE);
	SSD1306_OutBuffer();	

  SSD1306_SetCursor(1, 1);
  SSD1306_OutString("FRUTAS");
	SSD1306_SetCursor(1, 3);
  SSD1306_OutString("Baya = 1 punto");
	SSD1306_SetCursor(1, 5);
  SSD1306_OutString("Manzana=3 puntos");
	SSD1306_SetCursor(1, 7);
  SSD1306_OutString("Cereza = 5 puntos");
	//Fruits Screen
	SysTick_Wait10ms(350);
	SSD1306_ClearBuffer();
	SSD1306_OutClear();
	SSD1306_SetCursor(0, 1);
	SSD1306_OutString("REGLAS:");
	SSD1306_SetCursor(0, 3);
  SSD1306_OutString("Atrapa tantas frutas");
	SSD1306_SetCursor(0, 4);
  SSD1306_OutString("como puedas mientras");
	SSD1306_SetCursor(0, 5);
	SSD1306_OutString("evitando el");
	SSD1306_SetCursor(0, 6);
  SSD1306_OutString("pájaros y bombas.");
	SysTick_Wait10ms(350);
	//Rules Screen
	SSD1306_OutClear();  
	SSD1306_ClearBuffer();
	SSD1306_DrawBMP(33, 47, bomb, 0, SSD1306_WHITE);
	SSD1306_DrawBMP(30, 63, bird , 0, SSD1306_WHITE);
	SSD1306_OutBuffer();
	SSD1306_SetCursor(0, 1);
  SSD1306_OutString("REGLAS:");
	SSD1306_SetCursor(0, 2);
  SSD1306_OutString("Si tocas una bomba");
	SSD1306_SetCursor(0, 3);
  SSD1306_OutString("o pájaro, pierdes.");
	SSD1306_SetCursor(0, 5);
  SSD1306_OutString("Bomba:");
	SSD1306_SetCursor(0, 7);
  SSD1306_OutString("Pájaro:");
	//Rules Continued Screen
	SysTick_Wait10ms(350);
	SSD1306_OutClear();
	SSD1306_ClearBuffer();
	
	//CountDown
	SSD1306_SetCursor(10, 4);
	SSD1306_OutString("cinco");
	SysTick_Wait10ms(100);
	SSD1306_SetCursor(10, 4);
	SSD1306_OutString("quattro");
	SysTick_Wait10ms(100);
	SSD1306_OutClear();
	SSD1306_SetCursor(10, 4);
	SSD1306_OutString("tres");
	SysTick_Wait10ms(100);
	SSD1306_OutClear();
	SSD1306_SetCursor(10, 4);
	SSD1306_OutString("dos");
	SysTick_Wait10ms(100);
	SSD1306_OutClear();
	SSD1306_SetCursor(10, 4);
	SSD1306_OutString("uno");
	SysTick_Wait10ms(100);
	EnableInterrupts();
	
	
  while(player.life == alive){
		if(NeedToDraw) { 
			Draw();
		}	
  }
	DisableInterrupts();
	SSD1306_ClearBuffer();
	SSD1306_OutClear();
	SSD1306_SetCursor(6,4);
	SSD1306_OutString("Puntaje:");
	SSD1306_SetCursor(15,4);
	SSD1306_OutUDec2(score);
	}

}




// You can't use this timer, it is here for starter code only 
// you must use interrupts to perform delays
void Delay100ms(uint32_t count){uint32_t volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
	  	time--;
    }
    count--;
  }
}

void SysTick_Init20Hz(void){
	NVIC_ST_CTRL_R = 0;
	NVIC_ST_RELOAD_R = (80000000/20)-1;
	NVIC_ST_CURRENT_R = 0;
	NVIC_ST_CTRL_R = 7;
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF) |0x40000000; // priority 2

}
