/*
 * alarms.c
 *
 *  Created on: Apr 19, 2017
 *      Author: michele
 */
#include "board.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "system_tick.h"
#include "led.h"
#include "relais.h"
#include "alarms.h"

#define def_NOTactive_alarm_relais_status 1
#define def_active_alarm_relais_status (!def_NOTactive_alarm_relais_status)

typedef enum
{
	enum_LED_status_off = 0,
	enum_LED_status_on,
	enum_LED_status_flash_1Hz,
	enum_LED_status_flash_2Hz,
	enum_LED_status_numof
}enum_LED_status;

typedef enum
{
	enum_alarm_status_idle = 0,
	enum_alarm_status_init,
	enum_alarm_status_run,
	enum_alarm_status_ends,
	enum_alarm_status_numof
}enum_alarm_status;

typedef struct _type_table_alarm_LED
{
	enum_alarm_1496 a;
	enum_LED l;
	enum_LED_status s;
}type_table_alarm_LED;

static const type_table_alarm_LED table_alarm_LED[]=
{
// alarm none
	{.a = enum_alarm_none, 			.l = enum_LED_green,.s = enum_LED_status_flash_1Hz		},
	{.a = enum_alarm_none, 			.l = enum_LED_red, 	.s = enum_LED_status_off			},
// alarm enum_alarm_balance_X
	{.a = enum_alarm_balance_X, 	.l = enum_LED_green,.s = enum_LED_status_flash_1Hz		},
	{.a = enum_alarm_balance_X, 	.l = enum_LED_red, 	.s = enum_LED_status_flash_1Hz		},
// alarm enum_alarm_balance_Y
	{.a = enum_alarm_balance_Y, 	.l = enum_LED_green,.s = enum_LED_status_flash_1Hz		},
	{.a = enum_alarm_balance_Y, 	.l = enum_LED_red, 	.s = enum_LED_status_flash_2Hz		},
// alarm enum_alarm_balance_XY
	{.a = enum_alarm_balance_XY, 	.l = enum_LED_green,.s = enum_LED_status_flash_1Hz		},
	{.a = enum_alarm_balance_XY, 	.l = enum_LED_red, 	.s = enum_LED_status_on				},
// alarm enum_alarm_invalid_dipsw
	{.a = enum_alarm_invalid_dipsw, .l = enum_LED_green,.s = enum_LED_status_on				},
	{.a = enum_alarm_invalid_dipsw, .l = enum_LED_red, 	.s = enum_LED_status_on				},
// alarm enum_alarm_SW
	{.a = enum_alarm_SW, 			.l = enum_LED_green,.s = enum_LED_status_off			},
	{.a = enum_alarm_SW, 			.l = enum_LED_red, 	.s = enum_LED_status_on				},
// alarm enum_alarm_HW
	{.a = enum_alarm_HW, 			.l = enum_LED_green,.s = enum_LED_status_off			},
	{.a = enum_alarm_HW, 			.l = enum_LED_red, 	.s = enum_LED_status_on				},
};


const type_table_alarm_LED *find_alarm_LED_status(enum_alarm_1496 a, enum_LED l)
{
	const type_table_alarm_LED * p_ret = NULL;
	unsigned int i;
	for (i = 0; !p_ret && i < sizeof(table_alarm_LED) / sizeof(table_alarm_LED[0]); i++)
	{
		const type_table_alarm_LED * p = &table_alarm_LED[i];
		if (p->a == a && p->l == l)
		{
			p_ret = p;
		}
	}
	return p_ret;
}
typedef struct _type_LED_status
{
	enum_LED_status s;
	uint32_t cnt;
	uint32_t out_value;
}type_LED_status;

typedef enum
{
	enum_stato_relais_alarm_idle = 0,
	enum_stato_relais_alarm_init,
	enum_stato_relais_alarm_check,
	enum_stato_relais_alarm_init_active,
	enum_stato_relais_alarm_wait_active,
	enum_stato_relais_alarm_init_not_active,
	enum_stato_relais_alarm_wait_not_active,

	enum_stato_relais_alarm_numof
}enum_stato_relais_alarm;

typedef struct _type_handle_alarms
{
	enum_alarm_1496 current_alarm;
	enum_alarm_1496 alarm_1496;
	enum_alarm_1496 edge_alarm_1496;
	enum_alarm_status s;
	uint32_t cur_alarm_duration_ms;
	uint32_t max_alarm_duration_ms;
	uint64_t base_time_ms;
	type_LED_status LED[enum_LED_numof];

	enum_stato_relais_alarm relais;
	unsigned int duration_relais_ms;
	uint64_t base_relais_ms;
}type_handle_alarms;

static void set_new_LED_status(type_LED_status *p, enum_LED_status s)
{
	p->s = s;
	p->cnt = 0;
	if (p->s == enum_LED_status_on)
	{
		p->out_value = 1;
	}
	else
	{
		p->out_value = 0;
	}
}

static unsigned int update_LED_status(type_LED_status *p, uint32_t delta_ms)
{
	unsigned int needs_update = 0;
	p->cnt += delta_ms;
	switch(p->s)
	{
		case enum_LED_status_flash_1Hz:
		{
			if (p->cnt >= 500)
			{
				p->out_value = !p->out_value;
				needs_update = 1;
				p->cnt -= 500;
			}
			break;
		}
		case enum_LED_status_flash_2Hz:
		{
			if (p->cnt >= 250)
			{
				p->out_value = !p->out_value;
				needs_update = 1;
				p->cnt -= 250;
			}
			break;
		}
		case enum_LED_status_on:
		case enum_LED_status_off:
		default:
		{
			break;
		}
	}
	return needs_update;
}

static type_handle_alarms handle_alarms;



void reset_current_alarm(void)
{
	handle_alarms.current_alarm = enum_alarm_none;
}
void accum_current_alarm(enum_alarm_1496 a)
{
	if (a > handle_alarms.current_alarm)
	{
		handle_alarms.current_alarm = a;
	}
}
static uint32_t get_alarm_duration_ms(enum_alarm_1496 a)
{
	uint32_t duration_ms = 0;
	switch(a)
	{
		case enum_alarm_none:
		case enum_alarm_invalid_dipsw:
		default:
		{
			duration_ms = 1000;
			break;
		}
		case enum_alarm_balance_X:
		case enum_alarm_balance_Y:
		case enum_alarm_balance_XY:
		case enum_alarm_SW:
		{
			duration_ms = 60 * 1000;
			break;
		}
		case enum_alarm_HW:
		{
			duration_ms = 1000;
			break;
		}
	}

	return duration_ms;
}

void init_new_alarm(enum_alarm_1496 a)
{
	if (a >= enum_alarm_numof)
	{
		a = enum_alarm_none;
	}
	handle_alarms.alarm_1496 = a;
	handle_alarms.edge_alarm_1496 = a;
	handle_alarms.cur_alarm_duration_ms = 0;
	handle_alarms.s = enum_alarm_status_init;
	handle_alarms.max_alarm_duration_ms = get_alarm_duration_ms(a);
	enum_LED e;
	for (e = (enum_LED)0; e < enum_LED_numof; e++)
	{
		const type_table_alarm_LED * p;
		p = find_alarm_LED_status(a, e);
		if (p)
		{
			set_new_LED_status(&handle_alarms.LED[e], p->s);
		}
	}
}

void set_current_alarm(void)
{
	enum_alarm_1496 a = handle_alarms.current_alarm;
	if (a >= enum_alarm_numof)
	{
		return;
	}
	if (a != enum_alarm_none)
	{
		handle_alarms.edge_alarm_1496 = a;
	}
	if (handle_alarms.alarm_1496 < a)
	{
		init_new_alarm (a);
	}
	else if ((a != enum_alarm_none) && (handle_alarms.alarm_1496 == a))
	{
		handle_alarms.cur_alarm_duration_ms = 0;
	}
}





void vhandle_alarms(void)
{
	switch(handle_alarms.s)
	{
		case enum_alarm_status_init:
		default:
		{
			handle_alarms.s = enum_alarm_status_run;
			handle_alarms.base_time_ms = get_tick_count();
			enum_LED e;
			for (e = (enum_LED)0; e < enum_LED_numof; e++)
			{
				set_led(e, handle_alarms.LED[e].out_value);
			}
			break;
		}
		case enum_alarm_status_run:
		{
			uint64_t now = get_tick_count();
			uint32_t delta_ms = now - handle_alarms.base_time_ms;
			handle_alarms.base_time_ms = now;
			enum_LED e;
			for (e = (enum_LED)0; e < enum_LED_numof; e++)
			{
				if (update_LED_status(&handle_alarms.LED[e], delta_ms))
				{
					set_led(e, handle_alarms.LED[e].out_value);
				}
			}
			if (handle_alarms.alarm_1496 != enum_alarm_none)
			{
				if (handle_alarms.max_alarm_duration_ms)
				{
					handle_alarms.cur_alarm_duration_ms += delta_ms;
				}
				if (handle_alarms.cur_alarm_duration_ms >= handle_alarms.max_alarm_duration_ms)
				{
					init_new_alarm(enum_alarm_none);
				}
			}
			break;
		}
	}


	switch(handle_alarms.relais)
	{
		case enum_stato_relais_alarm_idle:
		default:
		{
			handle_alarms.relais = enum_stato_relais_alarm_init;
			break;
		}
		case enum_stato_relais_alarm_init:
		{
			handle_alarms.relais = enum_stato_relais_alarm_init_not_active;
			if (handle_alarms.edge_alarm_1496 != enum_alarm_none)
			{
				handle_alarms.relais = enum_stato_relais_alarm_init_active;
			}
			break;
		}
		case enum_stato_relais_alarm_check:
		{
			if (handle_alarms.edge_alarm_1496 != enum_alarm_none)
			{
				handle_alarms.relais = enum_stato_relais_alarm_init_active;
			}
			break;
		}
		case enum_stato_relais_alarm_init_active:
		{
			relais_set(def_active_alarm_relais_status);
			handle_alarms.relais = enum_stato_relais_alarm_wait_active;
			handle_alarms.duration_relais_ms = 0;
			handle_alarms.base_relais_ms = get_tick_count();
			break;
		}
		case enum_stato_relais_alarm_wait_active:
		{
			uint64_t now = get_tick_count();
			handle_alarms.duration_relais_ms += now - handle_alarms.base_relais_ms;
			handle_alarms.base_relais_ms = now;
			if (handle_alarms.duration_relais_ms >= 1000)
			{
				handle_alarms.relais = enum_stato_relais_alarm_init_not_active;
			}
			break;
		}
		case enum_stato_relais_alarm_init_not_active:
		{
			handle_alarms.edge_alarm_1496 = enum_alarm_none;
			relais_set(def_NOTactive_alarm_relais_status);
			handle_alarms.relais = enum_stato_relais_alarm_wait_not_active;
			handle_alarms.duration_relais_ms = 0;
			handle_alarms.base_relais_ms = get_tick_count();
			break;
		}
		case enum_stato_relais_alarm_wait_not_active:
		{
			uint64_t now = get_tick_count();
			handle_alarms.duration_relais_ms += now - handle_alarms.base_relais_ms;
			handle_alarms.base_relais_ms = now;
			if (handle_alarms.duration_relais_ms >= 1000)
			{
				handle_alarms.relais = enum_stato_relais_alarm_check;
			}
			break;
		}
	}
}

void start_handle_alarms(void)
{
	init_new_alarm (handle_alarms.alarm_1496);

}
void module_init_alarms(void)
{
	memset(&handle_alarms, 0, sizeof(handle_alarms));
}
