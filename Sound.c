// Sound.c
// Runs on any computer
// Sound assets based off the original Space Invaders 
// Import these constants into your SpaceInvaders.c for sounds!
// Jonathan Valvano
// November 17, 2014
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "Sound.h"
#include "DAC.h"
#include "Timer0.h"
		
void Sound_Init(void){
	DAC_Init();
	Timer0_Init(7256);
};
