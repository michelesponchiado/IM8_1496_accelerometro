/*
 * led.c
 *
 *  Created on: Apr 10, 2017
 *      Author: michele
 */

#include "board.h"
#include <stdio.h>
#include "system_tick.h"
#include "led.h"

void set_led(enum_LED led, uint32_t value)
{
	switch(led)
	{
		case enum_LED_red:
		case enum_LED_green:
		{
			uint32_t pio3;
			pio3 = Chip_GPIO_GetPortValue(LPC_GPIO_PORT, 3);
			uint32_t ns = 0;
			if (led == enum_LED_red)
			{
				ns = 4;
			}
			else if (led == enum_LED_green)
			{
				ns = 5;
			}
			value = !! value;
			if (value)
			{
				pio3 |= (1 << ns);
			}
			else
			{
				pio3 &= ~(1 << ns);
			}
			Chip_GPIO_SetPortValue(LPC_GPIO_PORT, 3, pio3);
			break;
		}
		default:
		{
			break;
		}
	}
}

void led_init(void)
{
	Chip_GPIO_SetDir(LPC_GPIO_PORT, 3, (1<<4)|(1<<5), 1);
}

void led_test(void)
{
	set_led(enum_LED_red, 0);
	set_led(enum_LED_green, 0);
	delay_ms(100);

	unsigned int idx_loop;
	for (idx_loop = 0; idx_loop < 4; idx_loop++)
	{
#define def_delay_led_test_ms 90
		set_led(enum_LED_red, 1);
		set_led(enum_LED_green, 0);
		delay_ms(def_delay_led_test_ms);

		set_led(enum_LED_red, 1);
		set_led(enum_LED_green, 1);
		delay_ms(def_delay_led_test_ms);

		set_led(enum_LED_red, 0);
		set_led(enum_LED_green, 1);
		delay_ms(def_delay_led_test_ms);

		set_led(enum_LED_red, 0);
		set_led(enum_LED_green, 0);
		delay_ms(def_delay_led_test_ms);
	}
}
