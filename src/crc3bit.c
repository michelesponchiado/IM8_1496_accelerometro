/*
 * crc3bit.c
 *
 *  Created on: Apr 19, 2017
 *      Author: michele
 */

#if !def_local_main
	#include "board.h"
#endif
#include <stdio.h>
#include <stdint.h>
#if !def_local_main
	#include "crc3bit.h"
#endif


#if def_use2bits_for_crc
	#define def_crc3bit_divisor 1
	#define numbitsdivisor 3
	#define numbitscrc 2
	#define def_max_crc ((1 << numbitscrc) - 1)
	#define def_mask_crc ((1 << numbitscrc) - 1)
	#define numbitsinput 6
#else

	#define def_crc3bit_divisor 11
	#define numbitsdivisor 4
	#define numbitscrc 3
	#define def_max_crc ((1 << numbitscrc) - 1)
	#define def_mask_crc ((1 << numbitscrc) - 1)
	#define numbitsinput 6
#endif

#define def_max_n_input ((1 << numbitsinput) - 1)
#define numshift_divisor (numbitsinput - numbitsdivisor + numbitscrc)
#define divisor_shifted (def_crc3bit_divisor << numshift_divisor)
#define hi_bit_set_mask (1 << (numbitsinput + numbitscrc - 1))

#if def_use2bits_for_crc
	static uint32_t crc3table_1_to_64[64] =
	{
		1,  2,  3,  1,  0,  3,  2,  2,
		3,  0,  1,  3,  2,  1,  0,  1,
		0,  3,  2,  0,  1,  2,  3,  3,
		2,  1,  0,  2,  3,  0,  1,  2,
		3,  0,  1,  3,  2,  1,  0,  0,
		1,  2,  3,  1,  0,  3,  2,  3,
		2,  1,  0,  2,  3,  0,  1,  1,
		0,  3,  2,  0,  1,  2,  3,  0
	};
#else
	static uint32_t crc3table_1_to_32[32] =
	{
		3,6,5,7,4,1,2,5,6,3,0,2,1,4,7,1,
		2,7,4,6,5,0,3,4,7,2,1,3,0,5,6,2
	};
#endif

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
#if def_use2bits_for_crc
	for (i = 0; i < 64; i++)
	{
		if (!is_OK_crc_3bit_of_6bit_number(i + 1, crc3table_1_to_64[i]))
#else
	for (i = 0; i < 32; i++)
	{
		if (!is_OK_crc_3bit_of_6bit_number(i + 1, crc3table_1_to_32[i]))
#endif
		{
			isOK = 0;
			while(1)
			{

			}
		}
	}
	return isOK;
}

#if def_local_main
#if def_use2bits_for_crc
#include <stdlib.h>
#include <stdio.h>
	int print_and_check_crc2bit(void)
	{
		int ret = 0;
		FILE *f = fopen("crctable.txt", "wb");
		if (!f)
		{
			return -1;
		}
		unsigned int i;
		for (i = 1; (ret >= 0) && (i <= 64); i++)
		{
			unsigned int crc = calc_crc_3bit_of_6bit_number(i);
			unsigned int index = i;
			if (fprintf(f, "%2u, ", crc) < 0)
			{
				ret = -2;
			}
			if (crc3table_1_to_64[i - 1] != crc)
			{
				fprintf(f, "error table index %2u: crc %2u, table = %2u\r\n", index, crc, crc3table_1_to_64[i - 1]);
				ret = -10;
			}
			else if ((i < 64) && !is_OK_crc_3bit_of_6bit_number(i, crc3table_1_to_64[i - 1]))
			{
				fprintf(f, "error OK crc index %2u: crc %2u, table = %2u\r\n", index, crc, crc3table_1_to_64[i - 1]);
				ret = -11;
			}
		}
		if (fclose(f) < 0)
		{
			if (ret >= 0)
			{
				ret = -3;
			}
		}

		return ret;
	}
#endif
#endif
