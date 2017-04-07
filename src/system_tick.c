/*
 * system_tick.c
 *
 *  Created on: Apr 7, 2017
 *      Author: michele
 */

#include <stdlib.h>
#include <string.h>
#include "board.h"
#include "system_tick.h"

typedef struct _type_registered_callbacks_elem
{
	uint32_t valid;
	type_tick_callback fun;
}type_registered_callbacks_elem;

type_registered_callbacks_elem registered_callbacks[enum_tick_callback_type_numof];

void register_tick_callback(enum_tick_callback_type e, type_tick_callback fun)
{
	if (e >= enum_tick_callback_type_numof)
	{
		return;
	}
	type_registered_callbacks_elem * p = &registered_callbacks[e];
	p->fun = fun;
	p->valid = 1;
}

void unregister_tick_callback(enum_tick_callback_type e)
{
	if (e >= enum_tick_callback_type_numof)
	{
		return;
	}
	type_registered_callbacks_elem * p = &registered_callbacks[e];
	p->fun = NULL;
	p->valid = 0;
}


static volatile uint64_t tick_cnt;

uint64_t get_tick_count(void)
{
	return tick_cnt;
}

void system_tick_module_init(void)
{
	memset(&registered_callbacks, 0, sizeof(registered_callbacks));
	tick_cnt = 0;
	// set a system tick every ms
	SysTick_Config(Chip_Clock_GetSystemClockRate() / 1000);
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/
/**
 * @brief	SysTick Interrupt Handler
 * @return	Nothing
 * @note	Systick interrupt handler
 */
void SysTick_Handler(void)
{
	tick_cnt ++;
	{
		enum_tick_callback_type e;
		for (e = (enum_tick_callback_type)0; e < enum_tick_callback_type_numof; e++)
		{
			type_registered_callbacks_elem * p = &registered_callbacks[e];
			if (p->fun && p->valid)
			{
				p->fun();
			}
		}
	}
}
