/*
 * encoder.c
 *
 *  Created on: Apr 5, 2017
 *      Author: michele
 */

#include "board.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "system_tick.h"
#include "uart.h"


uint8_t is_OK_get_current_encoder(uint8_t *pvalue)
{
	unsigned int isOK = 0;
	unsigned int num_try;
	for (num_try = 0; !isOK && num_try < 10; num_try++)
	{
		volatile uint32_t u0;
		u0 = Chip_GPIO_GetPortValue(LPC_GPIO_PORT, 2);
		uint32_t u0_base = (u0 >> 4) & 0xff;
		volatile uint32_t u1;
		u1 = Chip_GPIO_GetPortValue(LPC_GPIO_PORT, 2);
		uint32_t u1_base = (u1 >> 4) & 0xff;
		if (u0_base == u1_base)
		{
			*pvalue = u0_base;
			isOK = 1;
		}
	}
	return isOK;
}

// 6'000 rpm --> 100 round/second --> 10ms/round
// max 500ms delay --> 16 packs of 32ms each

#define def_freq_calc_period_num_shift 5 		// 1 << 5 = 32
#define def_freq_calc_num_packet_num_shift 5	// 1 << 4 = 16
#define def_freq_total_period_num_shift (def_freq_calc_period_num_shift + def_freq_calc_num_packet_num_shift)
#define def_freq_calc_period_ms (1 << def_freq_calc_period_num_shift)
#define def_freq_calc_num_packet (1 << def_freq_calc_num_packet_num_shift)
#define def_freq_total_period_ms (def_freq_calc_period_ms * def_freq_calc_num_packet)
typedef struct _type_encoder_interrupt
{
	uint8_t  prev_value;
	uint8_t  idx_h_value;
	uint8_t  h_value[64];
	uint32_t idx_cur_count;
	uint32_t cur_count;
	uint32_t idx_counts;
	uint32_t counts[def_freq_calc_num_packet];
	uint32_t num_counts_per_period;
	uint32_t freq_Hz;
	volatile uint32_t num_updates;
	uint32_t valid;
	uint32_t num_err_delta;
	uint64_t prev_update_freq_ms;
}type_encoder_interrupt;

static type_encoder_interrupt encoder_interrupt;

void handle_encoder_interrupt(void)
{
	uint8_t new_value = 0;
	if (is_OK_get_current_encoder(&new_value))
	{
		uint8_t delta = 0;

		encoder_interrupt.h_value[encoder_interrupt.idx_h_value] = encoder_interrupt.prev_value;
		if (++encoder_interrupt.idx_h_value >= sizeof(encoder_interrupt.h_value) / sizeof(encoder_interrupt.h_value[0]))
		{
			encoder_interrupt.idx_h_value = 0;
		}
		delta = new_value - encoder_interrupt.prev_value;
		encoder_interrupt.prev_value = new_value;
		if (delta > 8)
		{
			encoder_interrupt.num_err_delta++;
			delta = 0;
		}
		encoder_interrupt.cur_count += delta;
		if (++encoder_interrupt.idx_cur_count >= def_freq_calc_period_ms)
		{
			encoder_interrupt.counts[encoder_interrupt.idx_counts] = encoder_interrupt.cur_count;
			encoder_interrupt.idx_cur_count = 0;
			encoder_interrupt.cur_count = 0;
			if (++encoder_interrupt.idx_counts >= def_freq_calc_num_packet)
			{
				encoder_interrupt.idx_counts = 0;
				encoder_interrupt.valid = 1;
			}
			if (encoder_interrupt.valid)
			{
				uint32_t num_counts_per_period = 0;
				unsigned int i;
				for (i = 0; i < def_freq_calc_num_packet; i++)
				{
					num_counts_per_period += encoder_interrupt.counts[i];
				}
				encoder_interrupt.num_counts_per_period = num_counts_per_period;
				encoder_interrupt.freq_Hz = (encoder_interrupt.num_counts_per_period * 1000) >> def_freq_total_period_num_shift;
			}
			encoder_interrupt.num_updates++;
		}
	}
}

void encoder_module_handle_run(void)
{
	uint64_t now_ms = get_tick_count();
	if (now_ms - encoder_interrupt.prev_update_freq_ms >= 500)
	{
		encoder_interrupt.prev_update_freq_ms = now_ms;
//		tx_uart((uint8_t*)"hello\r\n", 7);
#if 1
		unsigned int i;
		for (i = 0; i < 10; i++)
		{
			uint32_t n = encoder_interrupt.num_updates;
			uint32_t freq_Hz = encoder_interrupt.freq_Hz;
			uint32_t val8 = encoder_interrupt.prev_value;
			if (n == encoder_interrupt.num_updates)
			{
				char status_encoder[64];
#if 1
				int n_chars = snprintf(status_encoder, sizeof(status_encoder), "freq [Hz]: %u, nc: %3u\r\n"
						,freq_Hz
						,encoder_interrupt.num_counts_per_period
						);
#else
				int n_chars = snprintf(status_encoder, sizeof(status_encoder), "cnt: 0x%02X %c%c%c%c%c%c%c%c, freq [Hz]: %u, nc: %3u\r\n"
						,val8
						,(val8 & 0x80) ? '1' : '0'
						,(val8 & 0x40) ? '1' : '0'
						,(val8 & 0x20) ? '1' : '0'
						,(val8 & 0x10) ? '1' : '0'
						,(val8 & 0x8) ? '1' : '0'
						,(val8 & 0x4) ? '1' : '0'
						,(val8 & 0x2) ? '1' : '0'
						,(val8 & 0x1) ? '1' : '0'
						,freq_Hz
						,encoder_interrupt.num_counts_per_period
						);
#endif
				if (n_chars > sizeof(status_encoder))
				{
					n_chars = sizeof(status_encoder);
				}
				tx_uart((uint8_t*)status_encoder, n_chars);
				break;
			}
		}
#endif
	}
}

void encoder_module_init(void)
{
	memset(&encoder_interrupt, 0, sizeof(encoder_interrupt));
	unsigned int i;
	for (i = 0; i < 10; i++)
	{
		uint8_t new_value = 0;
		if (is_OK_get_current_encoder(&new_value))
		{
			encoder_interrupt.prev_value = new_value;
			break;
		}
	}

}
void encoder_module_register_callbacks(void)
{
	register_tick_callback(enum_tick_callback_type_encoder, handle_encoder_interrupt);
}
