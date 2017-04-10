/*
 * accelerometer.h
 *
 *  Created on: Apr 7, 2017
 *      Author: michele
 */

#ifndef INC_ACCELEROMETER_H_
#define INC_ACCELEROMETER_H_

void test_accelerometer(void);
void accelerometer_module_init(void);
void accelerometer_module_handle_run(void);

typedef struct _type_accelerometer_main_info
{
	int32_t acc_mms2[2];
	int32_t min_acc_mms2[2];
	int32_t max_acc_mms2[2];
	uint32_t num_readOK;
	uint32_t numerr_send1;
	uint32_t numerr_send2;
}type_accelerometer_main_info;
void refresh_accelerometer_info(type_accelerometer_main_info *pdst);

#endif /* INC_ACCELEROMETER_H_ */
