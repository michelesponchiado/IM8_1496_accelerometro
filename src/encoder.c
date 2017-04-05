/*
 * encoder.c
 *
 *  Created on: Apr 5, 2017
 *      Author: michele
 */

#include "board.h"
#include <stdio.h>

uint8_t is_OK_get_current_encoder(uint32_t *pvalue)
{
	unsigned int isOK = 0;
	unsigned int num_try;
	for (num_try = 0; !isOK && num_try < 10; num_try++)
	{
		volatile uint32_t u0;
		u0 = Chip_GPIO_GetPortValue(LPC_GPIO_PORT, 2);
		uint32_t u0_base = (u0 >> 4) & 0xff;
		volatile uint32_t u1;
		u1 = Chip_GPIO_GetPortValue(LPC_GPIO_PORT, 2);
		uint32_t u1_base = (u1 >> 4) & 0xff;
		if (u0_base == u1_base)
		{
			*pvalue = u0_base;
			isOK = 1;
		}
	}
	return isOK;
}
