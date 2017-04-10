/*
 * led.h
 *
 *  Created on: Apr 10, 2017
 *      Author: michele
 */

#ifndef INC_LED_H_
#define INC_LED_H_

typedef enum
{
	enum_LED_red = 0,
	enum_LED_green,

	enum_LED_numof
}enum_LED;

void set_led(enum_LED led, uint32_t value);
void led_test(void);
void led_init(void);

#endif /* INC_LED_H_ */
