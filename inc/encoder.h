/*
 * encoder.h
 *
 *  Created on: Apr 7, 2017
 *      Author: michele
 */

#ifndef INC_ENCODER_H_
#define INC_ENCODER_H_

void encoder_module_init(void);
void encoder_module_register_callbacks(void);
void encoder_module_handle_run(void);
typedef struct _type_encoder_main_info
{
	uint32_t num_err_too_high_input_freq;
	uint32_t num_err_get_encoder;
	uint32_t freq_Hz;
	uint32_t valid;
	uint32_t num_updates;
}type_encoder_main_info;
void refresh_encoder_info(type_encoder_main_info *pdst);

#endif /* INC_ENCODER_H_ */
