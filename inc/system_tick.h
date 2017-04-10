/*
 * system_tick.h
 *
 *  Created on: Apr 7, 2017
 *      Author: michele
 */

#ifndef INC_SYSTEM_TICK_H_
#define INC_SYSTEM_TICK_H_

typedef void (*type_tick_callback)(void);

typedef enum
{
	enum_tick_callback_type_encoder = 0,
	enum_tick_callback_type_numof
}enum_tick_callback_type;

void register_tick_callback(enum_tick_callback_type e, type_tick_callback fun);

void unregister_tick_callback(enum_tick_callback_type e);

void system_tick_module_init(void);
uint64_t get_tick_count(void);
void delay_ms(uint32_t ms);


#endif /* INC_SYSTEM_TICK_H_ */
