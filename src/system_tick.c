/*
 * system_tick.c
 *
 *  Created on: Apr 7, 2017
 *      Author: michele
 */

#include <stdlib.h>
#include <string.h>
#include "board.h"

/*****************************************************************************
 * Public functions
 ****************************************************************************/
static volatile uint32_t tick_cnt;
/**
 * @brief	SysTick Interrupt Handler
 * @return	Nothing
 * @note	Systick interrupt handler
 */
void SysTick_Handler(void)
{
	tick_cnt ++;
}
