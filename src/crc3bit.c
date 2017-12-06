/*
 * crc3bit.c
 *
 *  Created on: Apr 19, 2017
 *      Author: michele
 */

#include "board.h"
#include <stdio.h>
#include <stdint.h>


#define def_crc3bit_divisor 11
#define numbitsdivisor 4
#define numbitscrc 3
#define def_max_crc ((1 << numbitscrc) - 1)
#define def_mask_crc ((1 << numbitscrc) - 1)
#define numbitsinput 6
#define def_max_n_input ((1 << numbitsinput) - 1)
#define numshift_divisor (numbitsinput - numbitsdivisor + numbitscrc)
#define divisor_shifted (def_crc3bit_divisor << numshift_divisor)
#define hi_bit_set_mask (1 << (numbitsinput + numbitscrc - 1))

static uint32_t crc3table_1_to_32[32] =
{
3,6,5,7,4,1,2,5,6,3,0,2,1,4,7,1,2,7,4,6,5,0,3,4,7,2,1,3,0,5,6,2
};

uint32_t calc_crc_3bit_of_6bit_number(uint32_t n)
{
	uint32_t xcrc = (n << numbitscrc);
	uint32_t i;
	for (i = 0; i < numbitsinput; i++)
	{
		if (xcrc & hi_bit_set_mask)
		{
			xcrc = xcrc ^ divisor_shifted;
		}
		xcrc <<= 1;
	}
	xcrc = (xcrc >> numbitsinput) & def_mask_crc;
	return xcrc;
}

uint32_t is_OK_crc_3bit_of_6bit_number(uint32_t n, uint32_t crc)
{
	if (crc > def_max_crc)
	{
		return 0;
	}
	if (n > def_max_n_input)
	{
		return 0;
	}
	uint32_t xcrc = (n << numbitscrc);
	xcrc += crc;
	uint32_t i;
	for (i = 0; i < numbitsinput; i++)
	{
		if (xcrc & hi_bit_set_mask)
		{
			xcrc = xcrc ^ divisor_shifted;
		}
		xcrc <<= 1;
	}
	xcrc = (xcrc >> numbitsinput) & def_mask_crc;

	return (xcrc == 0) ? 1 : 0;
}


unsigned int isOK_test_crc3(void)
{
	uint32_t isOK = 1;
	uint32_t i;
	for (i = 0; i < 32; i++)
	{
		if (!is_OK_crc_3bit_of_6bit_number(i + 1, crc3table_1_to_32[i]))
		{
			isOK = 0;
			while(1)
			{

			}
		}
	}
	return isOK;
}
