// FiFo.c
// Runs on TM4C123
// Provide functions that implement the Software FiFo Buffer
// Last Modified: 1/17/2021 
// Student names: Roshan Rajan
// Last modification date: change this to the last modification date or look very silly
#include <stdint.h>
#define success 1
#define fail 0
#define SIZE 7
char fifo[SIZE];
// Declare state variables for FiFo
//        size, buffer, put and get indexes
uint8_t GetI;
uint8_t PutI;
uint8_t numElements;
// *********** FiFo_Init**********
// Initializes a software FIFO of a
// fixed size and sets up indexes for
// put and get operations
void Fifo_Init() {
//Complete this
	GetI = 0;
	PutI = 0;
	numElements = 0;
}

// *********** FiFo_Put**********
// Adds an element to the FIFO
// Input: Character to be inserted
// Output: 1 for success and 0 for failure
//         failure is when the buffer is full
uint32_t Fifo_Put(char data) {
  //Complete this routine
 
     if( (PutI + 1)%SIZE == GetI) {
         return 0; 
     }
     fifo[PutI] = data; 
     PutI = (PutI+1)%SIZE;
		 numElements++;
   return 1;
}
// *********** Fifo_Get**********
// Gets an element from the FIFO
// Input: none
// Output: removed character from FIFO
//         0 failure is when the buffer is empty
char Fifo_Get(void){
	char data;
  if( PutI == GetI) {
         return 0; 
     }

     data = fifo[GetI]; 
     GetI = (GetI+1)%SIZE;
		 numElements--;
   return data;
}

// *********** Fifo_Status**********
// returns number of elements in the FIFO
// Input: none
// Output: number of entries in FIFO
//         0 failure is when the FIFO is empty
uint32_t Fifo_Status(void){
  //Complete this routine
  if (numElements == 0){
		return 0;
	}
	return numElements;
}
