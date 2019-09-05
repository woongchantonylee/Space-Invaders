// ADC.c
// Runs on LM4F120/TM4C123
// Provide functions that initialize ADC0
// Student names: Woongchan Lee and Ben Pretzer
// Last modification date: 4/22/2019

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

// ADC initialization function 
// Input: none
// Output: none
void ADC_Init(void){ 
	SYSCTL_RCGCGPIO_R |= 0x08;	//clock on for port D
	while ((SYSCTL_PRGPIO_R&0x08) == 0) {};
	for(uint32_t i = 0; i < 10000; i++) {}
	GPIO_PORTD_DIR_R &= ~0x04; //PD2 input
	GPIO_PORTD_AFSEL_R |= 0x04; //enable alternate function on PD2
	GPIO_PORTD_DEN_R &= ~0x04; //disable digital I/O on PD2
	GPIO_PORTD_AMSEL_R |= 0x04; //enable analog function on PD2
	SYSCTL_RCGCADC_R |= 0x01; //activate ADC0
	uint32_t delay = SYSCTL_RCGCADC_R;
	for(uint32_t i = 0; i < 1000000; i++) {}
	ADC0_PC_R = 0x01;
	ADC0_SSPRI_R = 0x0123; //highest priority
	ADC0_ACTSS_R &= ~0x0008; //disable sample sequencer 3
	ADC0_EMUX_R &= ~0xF000; //sequence 3 is software trigger
	ADC0_SSMUX3_R = (ADC0_SSMUX3_R&0xFFFFFFF0) + 5; //channel Ain5 for PD2
	ADC0_SSCTL3_R = 0x0006; //no TS0 D0, yes IE0 END0
	ADC0_IM_R &= ~0x0008;
	ADC0_ACTSS_R |= 0x0008; //enable sample sequencer 3
}

//------------ADC_In------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
uint32_t ADC_In(void){  
	uint32_t result;
	ADC0_PSSI_R = 0x0008;            
  while((ADC0_RIS_R&0x08)==0){};   // 2) wait for conversion
  result = ADC0_SSFIFO3_R&0xFFF;   // 3) read result
  ADC0_ISC_R = 0x0008;             // 4) acknowledge completion
  return result;
}
