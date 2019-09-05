// put prototypes for public functions, explain what it does
// put your names here, date
#include <stdint.h>

void ADC_Init(void);

//------------ADC_In------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
uint32_t ADC_In(void);
