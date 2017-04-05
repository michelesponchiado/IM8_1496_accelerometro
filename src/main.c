/*
 * main.c
 *
 *  Created on: Apr 5, 2017
 *      Author: michele
 */


#include "board.h"
#include <stdio.h>


//using internal oscillator
const uint32_t OscRateIn = 0;

int main(void)
{
	/* Generic Initialization */
	SystemCoreClockUpdate();
	//Board_Init();
	//Board_ADC_Init();
	//DEBUGSTR("ADC Demo\r\n");


	/* Should not run to here */
	return 0;
}
