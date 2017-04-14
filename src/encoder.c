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
#include "encoder.h"


// 6'000 rpm --> 100 round/second --> 10ms/round
// max 500ms delay --> 16 packs of 32ms each

#define def_max_counting_freq_kHz 8

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
	uint64_t last_count_tick[def_freq_calc_num_packet];
	uint32_t num_counts_per_period;
	uint32_t freq_Hz;
	uint32_t freq_mHz;
	uint32_t min_freq_Hz;
	uint32_t max_freq_Hz;
	volatile uint32_t num_updates;
	uint32_t valid;
	uint32_t num_err_too_high_input_freq;
	uint32_t num_err_get_encoder;
	uint64_t prev_update_freq_ms;
	uint32_t update_disable;
	type_encoder_main_info main_info;
}type_encoder_interrupt;

static type_encoder_interrupt encoder_interrupt;


uint8_t is_OK_get_current_encoder(uint8_t *pvalue)
{
	unsigned int isOK = 0;
	unsigned int num_try;
	for (num_try = 0; !isOK && num_try < 10; num_try++)
	{
#define def_max_consec_reading_steady 5
		volatile uint32_t u[def_max_consec_reading_steady];
		unsigned int i;
		for (i = 0; i < def_max_consec_reading_steady; i++)
		{
			uint32_t u_current = Chip_GPIO_GetPortValue(LPC_GPIO_PORT, 2);
			u[i] = (u_current >> 4) & 0xff;
		}
		isOK = 1;
		uint32_t u_first = u[0];
		for (i = 1; isOK && i < def_max_consec_reading_steady; i++)
		{
			if (u[i] != u_first)
			{
				isOK = 0;
			}
		}
		if (isOK)
		{
			*pvalue = u_first;
		}
	}
	if (!isOK)
	{
		encoder_interrupt.num_err_get_encoder++;
	}
	return isOK;
}



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
		if (delta > def_max_counting_freq_kHz)
		{
			encoder_interrupt.num_err_too_high_input_freq++;
			delta = 0;
		}
		uint64_t now = get_tick_count();
		if (delta)
		{
			encoder_interrupt.last_count_tick[encoder_interrupt.idx_counts] = now;
		}
		encoder_interrupt.cur_count += delta;
		if (++encoder_interrupt.idx_cur_count >= def_freq_calc_period_ms)
		{
			uint32_t latest_idx_counts = encoder_interrupt.idx_counts;
			encoder_interrupt.counts[encoder_interrupt.idx_counts] = encoder_interrupt.cur_count;
			encoder_interrupt.idx_cur_count = 0;
			encoder_interrupt.cur_count = 0;
			if (++encoder_interrupt.idx_counts >= def_freq_calc_num_packet)
			{
				encoder_interrupt.idx_counts = 0;
				encoder_interrupt.valid = 1;
			}
			uint32_t oldest_idx_counts = encoder_interrupt.idx_counts;
			if (encoder_interrupt.valid)
			{
				uint32_t num_counts_per_period = 0;
				unsigned int i;
				unsigned int idx = oldest_idx_counts;
				for (i = 0; i < def_freq_calc_num_packet - 1; i++)
				{
					if (++idx >= def_freq_calc_num_packet)
					{
						idx = 0;
					}
					num_counts_per_period += encoder_interrupt.counts[idx];
				}
				uint64_t oldest_time = encoder_interrupt.last_count_tick[oldest_idx_counts];
				uint64_t latest_time = encoder_interrupt.last_count_tick[latest_idx_counts];
				encoder_interrupt.last_count_tick[oldest_idx_counts] = latest_time;
				uint32_t delta_time_ms = latest_time - oldest_time;

				encoder_interrupt.num_counts_per_period = num_counts_per_period;
				if (delta_time_ms)
				{
					encoder_interrupt.freq_mHz = (num_counts_per_period * 1000 * 1000) / delta_time_ms;
				}
				else
				{
					encoder_interrupt.freq_mHz = 0;
				}
				encoder_interrupt.freq_Hz = encoder_interrupt.freq_mHz / 1000;
				if (encoder_interrupt.min_freq_Hz > encoder_interrupt.freq_Hz)
				{
					encoder_interrupt.min_freq_Hz = encoder_interrupt.freq_Hz;
				}
				if (encoder_interrupt.max_freq_Hz < encoder_interrupt.freq_Hz)
				{
					encoder_interrupt.max_freq_Hz = encoder_interrupt.freq_Hz;
				}
			}
			encoder_interrupt.num_updates++;
			{
				type_encoder_main_info *p = &encoder_interrupt.main_info;
				if (encoder_interrupt.update_disable == 0)
				{
					p->freq_Hz = encoder_interrupt.freq_Hz;
					p->freq_mHz = encoder_interrupt.freq_mHz;
					p->num_err_get_encoder = encoder_interrupt.num_err_get_encoder;
					p->num_err_too_high_input_freq = encoder_interrupt.num_err_too_high_input_freq;
					p->valid = encoder_interrupt.valid;
					p->num_updates = encoder_interrupt.num_updates;
					p->min_freq_Hz = encoder_interrupt.min_freq_Hz;
					p->max_freq_Hz = encoder_interrupt.max_freq_Hz;
				}
			}
		}
	}
}

void refresh_encoder_info(type_encoder_main_info *pdst)
{
	encoder_interrupt.update_disable = 1;
		*pdst = encoder_interrupt.main_info;
	encoder_interrupt.update_disable = 0;
}

void init_encoder_stats(void)
{
	encoder_interrupt.min_freq_Hz = UINT32_MAX;
	encoder_interrupt.max_freq_Hz = 0;
}


void encoder_module_handle_run(void)
{
#if 0
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
			if (n == encoder_interrupt.num_updates)
			{
				char status_encoder[64];
				int n_chars = 0;
#if 1
				if (encoder_interrupt.valid)
				{
					n_chars = snprintf(status_encoder, sizeof(status_encoder), "freq [Hz]: %u, nloop: %u, err_hi_freq: %u, err_get_enc: %u\r\n"
							,freq_Hz
							,encoder_interrupt.num_updates
							,encoder_interrupt.num_err_too_high_input_freq
							,encoder_interrupt.num_err_get_encoder
							);
				}
				else
				{
					n_chars = snprintf(status_encoder, sizeof(status_encoder), "freq [Hz]: INVALID, nloop: %u, err_hi_freq: %u, err_get_enc: %u\r\n"
							,encoder_interrupt.num_updates
							,encoder_interrupt.num_err_too_high_input_freq
							,encoder_interrupt.num_err_get_encoder
							);
				}
#else
				uint32_t val8 = encoder_interrupt.prev_value;
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
#endif
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
	init_encoder_stats();

}
void encoder_module_register_callbacks(void)
{
	register_tick_callback(enum_tick_callback_type_encoder, handle_encoder_interrupt);
}
void reset_encoder_stats(void)
{
	init_encoder_stats();
}
