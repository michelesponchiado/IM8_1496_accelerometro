/*
 * dipswitches.c
 *
 *  Created on: Apr 5, 2017
 *      Author: michele
 */

#include "board.h"
#include <stdio.h>
#include "dipswitches.h"

typedef struct _type_uint_bits
{
	unsigned int b0:1;
	unsigned int b1:1;
	unsigned int b2:1;
	unsigned int b3:1;
	unsigned int b4:1;
	unsigned int b5:1;
	unsigned int b6:1;
	unsigned int b7:1;
	unsigned int b8:1;
	unsigned int b9:1;
	unsigned int fill;
}type_uint_bits;

typedef union
{
	uint32_t uint;
	type_uint_bits b;
}uint_bits;

uint8_t read_dipswitch1(void)
{
	uint_bits p0;
	p0.uint = Chip_GPIO_GetPortValue(LPC_GPIO_PORT, 0);
	uint_bits p1;
	p1.uint = Chip_GPIO_GetPortValue(LPC_GPIO_PORT, 1);
	uint_bits p3;
	p3.uint = Chip_GPIO_GetPortValue(LPC_GPIO_PORT, 3);
	return 		(p0.b.b3 << 0)
			| 	(p0.b.b6 << 1)
			| 	(p1.b.b8 << 2)
			| 	(p1.b.b9 << 3)
			| 	(p3.b.b0 << 4)
			| 	(p3.b.b1 << 5)
			| 	(p3.b.b2 << 6)
			| 	(p3.b.b3 << 7)
			;

	return 1;
}

uint8_t read_dipswitch2(void)
{
	uint_bits p2;
	p2.uint = Chip_GPIO_GetPortValue(LPC_GPIO_PORT, 2);
	return
			  	(p2.b.b0 << 0)
			| 	(p2.b.b1 << 1)
			| 	(p2.b.b2 << 2)
			| 	(p2.b.b3 << 3)
			;

	return 1;
}
