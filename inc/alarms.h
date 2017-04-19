/*
 * alarms.h
 *
 *  Created on: Apr 19, 2017
 *      Author: michele
 */

#ifndef INC_ALARMS_H_
#define INC_ALARMS_H_


typedef enum
{
	enum_alarm_none = 0,
	enum_alarm_balance_X,
	enum_alarm_balance_Y,
	enum_alarm_balance_XY,
	enum_alarm_invalid_dipsw,
	enum_alarm_SW,
	enum_alarm_HW,

	enum_alarm_numof
}enum_alarm_1496;

void module_init_alarms(void);
void reset_current_alarm(void);
void accum_current_alarm(enum_alarm_1496 a);
void set_current_alarm(void);
void vhandle_alarms(void);
void start_handle_alarms(void);
void init_new_alarm(enum_alarm_1496 a);

#endif /* INC_ALARMS_H_ */
