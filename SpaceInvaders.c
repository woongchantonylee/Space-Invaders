// SpaceInvaders.c
// Runs on LM4F120/TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the EE319K Lab 10

// Last Modified: 4/30/2019 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php
/* This example accompanies the books
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2018

   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2018

 Copyright 2018 by Jonathan W. Valvano, valvano@mail.utexas.edu
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
// 1*R resistor DAC bit 3 on PB3 (most significant bit)
// LED on PB4
// LED on PB5

// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO), high for data, low for command
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "ST7735.h"
#include "Random.h"
#include "PLL.h"
#include "ADC.h"
#include "Images.h"
#include "Sound.h"
#include "Timer0.h"
#include "Timer1.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Delay100ms(uint32_t count); // time delay in 0.1 seconds
void PortC_Init(void);
void SysTick_Init(void);
void printScoreboard(void);
void convert(uint32_t in);
void gamestateInit(void);
void enemyRevive(void);
void enemyInit(void);
void bunkerInit(void);
void playerInit(void);
void bulletInit(void);
void GameOver(void);
void displaySprites(void);
void displayBullets(void);

struct Sprite {
	uint8_t x;
	uint8_t y;
	uint8_t width;
	uint8_t height;
	uint8_t HP; //health
	const unsigned short *images[3];
	uint8_t state; //0 alive, 1 moving, 2 dying, 3 dead
};typedef struct Sprite Sprite_t;

uint8_t numLives; //contains number of lives
uint8_t numPauses;
uint8_t dead;
uint8_t alldead;
uint8_t pass;
uint32_t score; //contains player score
uint32_t playerPos; //x-coordinate of player ship
uint8_t eDir; //1 - enemies moving left, 0 - enemies moving right
uint8_t enemyYcheckpoint; //keeps track of how far enemy has gotten
uint8_t moveCount; //counter to move enemies every second
uint8_t over; //indicates whether game is over
int8_t dif; //difficulty setting
int8_t edif; //difficulty setting with bullet rate
uint8_t eBulletInd; //keeps track of active enemy bullets
uint8_t eBulletInd2; //keeps track of active enemy bullets
uint8_t eCount; //counter to shoot bullets from enemies

Sprite_t EnemySquadron[30]; //contains enemy sprites
Sprite_t Bunkers[3]; //contains bunker sprites
Sprite_t Player; //contains player sprite
Sprite_t eBullets[6]; //contains enemy bullets
Sprite_t pBullet; //contains player bullet

int main(void){
  PLL_Init(Bus80MHz);    // Bus clock is 80 MHz
  ADC_Init();    // initialize to sample ADC
	PortC_Init();  // initialize port C buttons
	Output_Init();
	Sound_Init();
	gamestateInit();  //reset game to 'start' state
	ST7735_FillScreen(0x0000);
	ST7735_DrawBitmap(4,65,Logo,120,49);
	ST7735_DrawBitmap(4,65,Logo,120,49);
	ST7735_SetCursor(9, 8);
  ST7735_OutString("= 10 pts");
	ST7735_SetCursor(9, 10);
  ST7735_OutString("= 20 pts");
	ST7735_SetCursor(9, 12);
  ST7735_OutString("= 30 pts");
	ST7735_DrawBitmap(28,85,SmallEnemy10pointA,14,8);
	ST7735_DrawBitmap(28,105,SmallEnemy20pointA,14,8);
	ST7735_DrawBitmap(28,125,SmallEnemy30pointA,14,8);
	while((GPIO_PORTC_DATA_R & 0x10) == 0) {}
	while((GPIO_PORTC_DATA_R & 0x10) != 0) {}
	ST7735_FillScreen(0x0000);
	SysTick_Init();
/*
	ST7735_FillScreen(0x0000); // set screen to black
	printScoreboard();
	displaySprites();
  Delay100ms(50); // delay 5 sec at 80 MHz
*/
  while(1){
		convert(ADC_In()); //potentiometer position update
		alldead = 0;
		if(numLives == 0) { //When one runs out of lives, it's GAME OVER
			GameOver();
		}
		for(uint8_t i = 0; i < 30; i++) {
			if(EnemySquadron[i].y >= 138 && EnemySquadron[i].state < 2) { //When an enemy moves to y = 138, it's GAME OVER
				GameOver();
			}
			if(EnemySquadron[i].state != 3) {
				alldead = 1;
			}
		}
		if(alldead == 0) {
			ST7735_FillScreen(0x0000);
			enemyRevive();
		}
		pass = 1;
		while((GPIO_PORTC_DATA_R & 0x10) > 0 ) { //Restart game if C.0 button is pressed
			if(numPauses > 0 && over == 0 && pass == 1) {
				over = 2;
				pass = 0;
				ST7735_SetCursor(7, 7);
				ST7735_OutString("PAUSED!");
				numPauses--;
			} else if(over == 2 && pass == 1) {
				over = 0;
				pass = 0;
				ST7735_SetCursor(7, 7);
				ST7735_OutString("       ");
			}
		}
		if(GPIO_PORTC_DATA_R & 0x20 && pBullet.state == 3) { //fire bullet when C.1 button pressed
			pBullet.state = 0;
		}
		if(dead == 1) {
			dead = 0;
			numLives--;
		}
		eBulletInd = (eBulletInd+1)%6; //"random" index selector for bullets
		eBulletInd2	= (eBulletInd2+1)%5; //"random" index selector for bullets
  }
}

void SysTick_Handler(void) {
	if(over == 1) {
		
	}
	else if(over == 2) {
		
	}
	else {
		//display
		printScoreboard();
		displaySprites();
		displayBullets();
		dead = 0;
/******* Sprite Appearance *******/
		if(Player.state == 2) { //player death animation
			Player.state = 0;
			for(uint8_t i = 0; i < 6; i++) { //disable bullets
				eBullets[i].state = 3;
			}
			Delay100ms(10);
			return;
		}

/******* Sprite Movement *******/
		int8_t moveX = 0;
		int8_t moveY = 0;
		moveCount = (moveCount+1)%dif;
		if(eDir) { //if eDir is 1, move left
			moveX = -2;
		} else { //if eDir is 0, move right
			moveX = 2;
		}
		if(moveCount == 0) { //move all enemies to next position
			for(uint8_t i = 0; i < 30; i++) {
				if(EnemySquadron[i].x == 114 && eDir == 0 && EnemySquadron[i].state < 2) { //if enemies hit right wall, move down and reverse direction
					eDir = 1;
					moveX = 0;
					moveY = 2;
					enemyYcheckpoint += 2;
					edif -= 4;
					dif -= 2;
					if(dif < 6) dif = 6;
					if(edif < 0) edif = 0;
					break;
				}
				if(EnemySquadron[i].x == 0 && eDir == 1 && EnemySquadron[i].state < 2) { //if enemies hit left wall, move down and reverse direction
					eDir = 0;
					moveX = 0;
					moveY = 2;
					enemyYcheckpoint += 2;
					edif -= 4;
					dif -= 2;
					if(dif < 6) dif = 6;
					if(edif < 0) edif = 0;
					break;
				}
			}
			for(uint8_t i = 0; i < 30; i++) {
				if(moveX > 0) {
					ST7735_DrawBitmap(EnemySquadron[i].x,EnemySquadron[i].y,block,2,8);
				}
				else if(moveX < 0) {
					ST7735_DrawBitmap((EnemySquadron[i].x+EnemySquadron[i].width)-2,EnemySquadron[i].y,block,2,8);
				}
				if(EnemySquadron[i].state == 0) {
					EnemySquadron[i].state = 1;
				} else if(EnemySquadron[i].state == 1) {
					EnemySquadron[i].state = 0;
				}
				EnemySquadron[i].x += moveX;
				EnemySquadron[i].y += moveY;
				if((i < 6) && (eBullets[i].state != 0)) {
					eBullets[i].x = EnemySquadron[i].x + 6;
					eBullets[i].y = EnemySquadron[i].y;
				}
			}
		}
		
/******* Bullet Movement *******/
		if(pBullet.state == 0 && pBullet.y > 20) { //move active player bullets
			ST7735_DrawBitmap(pBullet.x,pBullet.y,block,2,3);
			pBullet.y -= 3;
		} else if(pBullet.state == 0 && pBullet.y <= 20) { 
			ST7735_DrawBitmap(pBullet.x,pBullet.y,block,2,6);
			pBullet.x = Player.x+6;
			pBullet.y = 155;
			pBullet.state = 3;
		}
		for(uint8_t i = 0; i < 6; i++) { //move active enemy bullets
			if(eBullets[i].state == 0) {
				if(eBullets[i].y >= 160) {
					eBullets[i].state = 3;
					ST7735_DrawBitmap(eBullets[i].x,eBullets[i].y,block,2,6);
				}
				ST7735_DrawBitmap(eBullets[i].x,eBullets[i].y-3,block,2,3);
				eBullets[i].y += 3;
			}
		}

		eCount = (eCount+1)%edif; //fire an enemy bullet at interval
		if(eCount == 0) {
			if(EnemySquadron[eBulletInd+eBulletInd2*6].state < 2 && eBullets[eBulletInd].state == 3) {
				eBullets[eBulletInd].state = 0;
				eBullets[eBulletInd].y = EnemySquadron[eBulletInd+eBulletInd2*6].y;
			}
		}
		
/******* Collision Check *******/
		for(uint8_t i = 0; i < 30; i++) { //check player bullet collision with enemy sprites
			if(EnemySquadron[i].state < 2) {
				if((EnemySquadron[i].x+EnemySquadron[i].width) > pBullet.x && (EnemySquadron[i].x) < pBullet.x) {
					if((EnemySquadron[i].y) > pBullet.y && (EnemySquadron[i].y-EnemySquadron[i].height) < pBullet.y) {
						EnemySquadron[i].state = 3;
						ST7735_DrawBitmap(EnemySquadron[i].x,EnemySquadron[i].y,block,14,8);
						ST7735_DrawBitmap(pBullet.x,pBullet.y+3,block,2,9);
						pBullet.x = Player.x+6;
						pBullet.y = 155;
						pBullet.state = 3;
						if(i<12) {
							score += 10;
						} else if(i<24) {
							score += 20;
						} else {
							score += 30;
						}
					}
					else if((EnemySquadron[i].y) > (pBullet.y-pBullet.height) && (EnemySquadron[i].y-EnemySquadron[i].height) < (pBullet.y-pBullet.height)) {
						EnemySquadron[i].state = 3;
						ST7735_DrawBitmap(pBullet.x,pBullet.y+3,block,2,9);
						pBullet.x = Player.x+6;
						pBullet.y = 155;
						pBullet.state = 3;
						if(i<12) {
							score += 10;
						} else if(i<24) {
							score += 20;
						} else {
							score += 30;
						}
					}
				}
				if(EnemySquadron[i].state != 3 && (EnemySquadron[i].x+EnemySquadron[i].width) > (pBullet.x+pBullet.width) && (EnemySquadron[i].x) < (pBullet.x+pBullet.width)) {
					if((EnemySquadron[i].y) > pBullet.y && (EnemySquadron[i].y-EnemySquadron[i].height) < pBullet.y) {
						EnemySquadron[i].state = 3;
						ST7735_DrawBitmap(pBullet.x,pBullet.y+3,block,2,9);
						pBullet.x = Player.x+6;
						pBullet.y = 155;
						pBullet.state = 3;
						if(i<12) {
							score += 10;
						} else if(i<24) {
							score += 20;
						} else {
							score += 30;
						}
					}
					else if((EnemySquadron[i].y) > (pBullet.y-pBullet.height) && (EnemySquadron[i].y-EnemySquadron[i].height) < (pBullet.y-pBullet.height)) {
						EnemySquadron[i].state = 3;
						ST7735_DrawBitmap(pBullet.x,pBullet.y+3,block,2,9);
						pBullet.x = Player.x+6;
						pBullet.y = 155;
						pBullet.state = 3;
						if(i<12) {
							score += 10;
						} else if(i<24) {
							score += 20;
						} else {
							score += 30;
						}
					}
				}
			}
		}
		for(uint8_t i = 0; i < 3; i++) { //check enemy bullet collision with bunker sprites
			if(Bunkers[i].state < 3) {
				for(uint8_t j = 0; j < 6; j++) {
					if(eBullets[j].state < 3) {
						if((Bunkers[i].x+Bunkers[i].width) > eBullets[j].x && (Bunkers[i].x) < eBullets[j].x) {
							if((Bunkers[i].y) > eBullets[j].y && (Bunkers[i].y-Bunkers[i].height) < eBullets[j].y) {
								Bunkers[i].HP--;
								eBullets[j].state = 3;
								ST7735_DrawBitmap(Bunkers[i].x,EnemySquadron[i].y,block,18,5);
								ST7735_DrawBitmap(eBullets[j].x,eBullets[j].y,block,2,9);
							}
							else if((Bunkers[i].y) > (eBullets[j].y-eBullets[j].height) && (Bunkers[i].y-Bunkers[i].height) < (eBullets[j].y-eBullets[j].height)) {
								Bunkers[i].HP--;
								eBullets[j].state = 3;
								ST7735_DrawBitmap(Bunkers[i].x,EnemySquadron[i].y,block,18,5);
								ST7735_DrawBitmap(eBullets[j].x,eBullets[j].y,block,2,9);
							}
						}
						if((Bunkers[i].x+Bunkers[i].width) > (eBullets[j].x+eBullets[j].width) && (Bunkers[i].x) < (eBullets[j].x+eBullets[j].width)) {
							if((Bunkers[i].y) > eBullets[j].y && (Bunkers[i].y+Bunkers[i].height) < eBullets[j].y) {
								Bunkers[i].HP--;
								eBullets[j].state = 3;
								ST7735_DrawBitmap(Bunkers[i].x,EnemySquadron[i].y,block,18,5);
								ST7735_DrawBitmap(eBullets[j].x,eBullets[j].y,block,2,9);
							}
							else if((Bunkers[i].y) > (eBullets[j].y-eBullets[j].height) && (Bunkers[i].y-Bunkers[i].height) < (eBullets[j].y-eBullets[j].height)) {
								Bunkers[i].HP--;
								eBullets[j].state = 3;
								ST7735_DrawBitmap(eBullets[j].x,eBullets[j].y,block,2,9);
								ST7735_DrawBitmap(Bunkers[i].x,EnemySquadron[i].y,block,18,5);
							}
						}
					}
				}
			}
		}
		for(uint8_t i = 0; i < 3; i++) { //update bunker state
			if(Bunkers[i].HP == 12) {
				Bunkers[i].state = 0;
			} if(Bunkers[i].HP < 12 && Bunkers[i].HP > 5) {
				Bunkers[i].state = 1;
			} if(Bunkers[i].HP < 6 && Bunkers[i].HP > 0) {
				Bunkers[i].state = 2;
			} if(Bunkers[i].HP == 0) {
				Bunkers[i].state = 3;
			}
		}
		for(uint8_t j = 0; j < 6; j++) { //check enemy bullet collision with player sprite
			if(eBullets[j].state < 3 && Player.state < 2) {
				if((Player.x+Player.width) > eBullets[j].x && (Player.x) < eBullets[j].x) {
					if((Player.y) > eBullets[j].y && (Player.y-Player.height) < eBullets[j].y) {
						Player.state = 2;
						dead = 1;
						eBullets[j].state = 3;
						ST7735_DrawBitmap(eBullets[j].x,eBullets[j].y,block,2,9);
					}
					else if((Player.y) > (eBullets[j].y-eBullets[j].height) && (Player.y-Player.height) < (eBullets[j].y-eBullets[j].height)) {
						Player.state = 2;
						dead = 1;
						eBullets[j].state = 3;
						ST7735_DrawBitmap(eBullets[j].x,eBullets[j].y,block,2,9);
					}
				}
				if((Player.x+Player.width) > (eBullets[j].x+eBullets[j].width) && (Player.x) < (eBullets[j].x+eBullets[j].width)) {
					if((Player.y) > eBullets[j].y && (Player.y-Player.height) < eBullets[j].y) {
						Player.state = 2;
						dead = 1;
						eBullets[j].state = 3;
						ST7735_DrawBitmap(eBullets[j].x,eBullets[j].y,block,2,9);
					}
					else if((Player.y) > (eBullets[j].y-eBullets[j].height) && (Player.y-Player.height) < (eBullets[j].y-eBullets[j].height)) {
						Player.state = 2;
						dead = 1;
						eBullets[j].state = 3;
						ST7735_DrawBitmap(eBullets[j].x,eBullets[j].y,block,2,9);
					}
				}
			}
		}
	}
}

//Systick timer initialized at 30Hz
void SysTick_Init(void) {
	NVIC_ST_CTRL_R = 0; // disable SysTick during setup
  NVIC_ST_RELOAD_R = 2666666;// reload value
  NVIC_ST_CURRENT_R = 0; // any write to current clears it
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x40000000; // priority 1
  NVIC_ST_CTRL_R = 0x0007; // enable SysTick with core clock and interrupts
}

//converts ADC input into x position of player ship
void convert(uint32_t in) {
	playerPos = (in*114)/4095;
	Player.x = playerPos;
	if(pBullet.state != 0)
		pBullet.x = playerPos + 6;
}

//Return game to original gamestate
void gamestateInit(void) {
	over = 0;
	numLives = 3;
	numPauses = 3;
	score = 0;
	eDir = 0; //enemies start moving right
	enemyYcheckpoint = 80; //enemy y-position start point
	moveCount = 0;
	eBulletInd = 0;
	eBulletInd2 = 0;
	dif = 24;
	edif = 100;
	eCount = 0;
	enemyInit();
	bunkerInit();
	playerInit();
	bulletInit();
	ST7735_FillScreen(0x0000);
}

//prints number of lives and live score to LCD
void printScoreboard(void) {
	ST7735_SetCursor(0, 0);
  ST7735_OutString("Lives:");
	ST7735_SetCursor(6, 0);
  LCD_OutDec(numLives);
	ST7735_SetCursor(8, 0);
  ST7735_OutString("Score:");
	ST7735_SetCursor(15, 0);
	LCD_OutDec(score);
}

void displaySprites(void) {
	for(uint8_t i = 0; i < 30; i++) {
		if(EnemySquadron[i].state != 3) {
			if(EnemySquadron[i].state == 2) { //enemy death explosion is (+3,0)
				ST7735_DrawBitmap(EnemySquadron[i].x+3,EnemySquadron[i].y,EnemySquadron[i].images[EnemySquadron[i].state],
																									10,9);
			} if(EnemySquadron[i].state == 3) {
				ST7735_DrawBitmap(EnemySquadron[i].x,EnemySquadron[i].y,block,EnemySquadron[i].width,EnemySquadron[i].height);
			} else {
				ST7735_DrawBitmap(EnemySquadron[i].x,EnemySquadron[i].y-8,block,14,2);
				ST7735_DrawBitmap(EnemySquadron[i].x,EnemySquadron[i].y,EnemySquadron[i].images[EnemySquadron[i].state],
																									EnemySquadron[i].width,EnemySquadron[i].height);
			}
		}
		else {
			ST7735_DrawBitmap(EnemySquadron[i].x,EnemySquadron[i].y,block,EnemySquadron[i].width,EnemySquadron[i].height);
		}
	}
	for(uint8_t i = 0; i < 3; i++) {
		if(Bunkers[i].state != 3) {
			ST7735_DrawBitmap(Bunkers[i].x,Bunkers[i].y,Bunkers[i].images[Bunkers[i].state],Bunkers[i].width,Bunkers[i].height);
		} else {
			ST7735_DrawBitmap(Bunkers[i].x,Bunkers[i].y,block,Bunkers[i].width,Bunkers[i].height);
		}
	}
	if(Player.state != 3) {
		ST7735_DrawBitmap(0,Player.y,block,128,1);
		ST7735_DrawBitmap(0,Player.y-1,block,128,1);
		ST7735_DrawBitmap(0,Player.y-2,block,128,1);
		ST7735_DrawBitmap(0,Player.y-3,block,128,1);
		ST7735_DrawBitmap(0,Player.y-4,block,128,1);
		ST7735_DrawBitmap(0,Player.y-5,block,128,1);
		ST7735_DrawBitmap(0,Player.y-6,block,128,1);
		ST7735_DrawBitmap(0,Player.y-7,block,128,1);
		ST7735_DrawBitmap(Player.x,Player.y,Player.images[Player.state],Player.width,Player.height);
	}
}

void displayBullets(void) {
	for(uint8_t i = 0; i < 36; i++) {
		if(eBullets[i].state == 0) {
			ST7735_DrawBitmap(eBullets[i].x,eBullets[i].y,eBullets[i].images[0],eBullets[i].width,eBullets[i].height);
		}
	}
	if(pBullet.state == 0) {
		ST7735_DrawBitmap(pBullet.x,pBullet.y,pBullet.images[0],pBullet.width,pBullet.height);
	}
}

//revives enemy squadron
//minions start from (0,24) then to (112,24) 
//moves 2 pixels every 1 second
//next minion at intervals of (+18,+14)
void enemyRevive(void) {
	if(enemyYcheckpoint >= 138) {
		enemyYcheckpoint = 136;
	}
	for(uint8_t i = 0; i < 30; i++) {
		if(eDir == 0) {
			EnemySquadron[i].x = (i%6)*18 + 12;
		} else if(eDir == 1) {
			EnemySquadron[i].x = (i%6)*18 + 8;
		}
		EnemySquadron[i].y = enemyYcheckpoint-(i/6)*14;
		EnemySquadron[i].state = 0;
		EnemySquadron[i].HP = 8;
	}
}

//initializes enemy squadron
void enemyInit(void) {
	for(uint8_t i = 0; i < 30; i++) {
		EnemySquadron[i].x = (i%6)*18;
		EnemySquadron[i].y = enemyYcheckpoint-(i/6)*14;
		EnemySquadron[i].state = 0;
		EnemySquadron[i].HP = 8;
		EnemySquadron[i].width = 14;
		EnemySquadron[i].height = 8;
		if(i<12) {
			EnemySquadron[i].images[0] = SmallEnemy10pointA;
			EnemySquadron[i].images[1] = SmallEnemy10pointB;
			EnemySquadron[i].images[2] = SmallEnemy10pointB;
		}
		else if(i>11 && i<24) {
			EnemySquadron[i].images[0] = SmallEnemy20pointA;
			EnemySquadron[i].images[1] = SmallEnemy20pointB;
			EnemySquadron[i].images[2] = SmallEnemy20pointB;
		}
		else if(i>23) {
			EnemySquadron[i].images[0] = SmallEnemy30pointA;
			EnemySquadron[i].images[1] = SmallEnemy30pointB;
			EnemySquadron[i].images[2] = SmallEnemy30pointB;
		}
	}
}

//Initializes 3 bunkers
//3 bunkers to cover fire
//placed (17,142), (55,142), (93,142)
void bunkerInit(void) {
	Bunkers[0].x = 17;
	Bunkers[1].x = 55;
	Bunkers[2].x = 93;
	for(uint8_t i = 0; i < 3; i++) {
		Bunkers[i].y = 142;
		Bunkers[i].width = 18;
		Bunkers[i].height = 5;
		Bunkers[i].HP = 12;
		Bunkers[i].images[0] = Bunker0;
		Bunkers[i].images[1] = Bunker1;
		Bunkers[i].images[2] = Bunker2;
		Bunkers[i].state = 0;
	}
}

//Initialize player ship
//player ships spawn at (playerPos,156)
//the range of player ship goes from x = 0 to x = 114
void playerInit(void) {
	Player.x = 55;
	Player.y = 156;
	Player.width = 14;
	Player.height = 7;
	Player.images[0] = PlayerShip0;
	Player.images[1] = PlayerShip0;
	Player.images[2] = PlayerShip1;
	Player.state = 0;
}

//player bullet appears from (+6,-1) width=2 x height=6
//enemy bullet appears from (+6,+3) width=4 x height 6
void bulletInit(void) {
	for(uint8_t i = 0; i < 6; i++) {
		eBullets[i].x = i*18+6;
		eBullets[i].y = enemyYcheckpoint;
		eBullets[i].width = 2;
		eBullets[i].height = 6;
		eBullets[i].images[0] = EBullet;
		eBullets[i].images[1] = EBullet;
		eBullets[i].images[2] = EBullet;
		eBullets[i].state = 3;
	}
	pBullet.x = 0;
	pBullet.y = 155;
	pBullet.width = 2;
	pBullet.height = 6;
	pBullet.images[0] = PBullet;
	pBullet.images[1] = PBullet;
	pBullet.images[2] = PBullet;
	pBullet.state = 3;
}

//GameOver Screen
//Posts Score
void GameOver(void) {
	over = 1;
	ST7735_FillScreen(0x0000);
  ST7735_SetCursor(6, 5);
  ST7735_OutString("GAME OVER");
  ST7735_SetCursor(6, 6);
  ST7735_OutString("Nice try,");
  ST7735_SetCursor(6, 7);
  ST7735_OutString("Earthling!");
	ST7735_SetCursor(3, 8);
  ST7735_OutString("Your Score: ");
  ST7735_SetCursor(15, 8);
  LCD_OutDec(score);
	while(1) {}
}

void PortC_Init(void) {
	SYSCTL_RCGCGPIO_R |= 0x04; // activate Port C
	for(uint8_t i = 0; i < 100; i++) {} // allow time for clock to start
	GPIO_PORTC_DIR_R &= ~0x30; // C.0-1 are button inputs
	GPIO_PORTC_DEN_R |= 0x30; // enable C.0-1
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
