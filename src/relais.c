/*
 * relais.c
 *
 *  Created on: Apr 10, 2017
 *      Author: michele
 */
#include "board.h"
#include <stdio.h>
#include "system_tick.h"
#include "relais.h"


//IOCON_PIO1_11

void relais_set(uint32_t value)
{
	uint32_t pio1;
	pio1 = Chip_GPIO_GetPortValue(LPC_GPIO_PORT, 1);
	value = !! value;
	if (value)
	{
		pio1 |= (1 << 11);
	}
	else
	{
		pio1 &= ~(1 << 11);
	}
	Chip_GPIO_SetPortValue(LPC_GPIO_PORT, 1, pio1);
}

void relais_init(void)
{
	Chip_GPIO_SetDir(LPC_GPIO_PORT, 1, (1<<11), 1);
}
