/*
 * accelerometer.c
 *
 *  Created on: Apr 7, 2017
 *      Author: michele
 */

#include <stdlib.h>
#include <string.h>
#include "board.h"
#include "i2c.h"
#include "accelerometer.h"
#include "system_tick.h"

#define def_center_acc_value 2048

#define def_num_shift_stats_values 7
#define def_num_stats_values (1 << def_num_shift_stats_values)
typedef struct _type_acc_io_stats
{
	int32_t min, max, avg;
	uint32_t idx_cur;
	int64_t accum_cur;
	int32_t value_cur;
	uint32_t idx;
	int32_t accum;
	int32_t values[def_num_stats_values];
	int32_t values_ordered[def_num_stats_values];
	uint32_t nsamples;
	uint32_t valid;
}type_acc_io_stats;

typedef enum
{
	enum_acc_io_status_init = 0,
	enum_acc_io_status_latch,
	enum_acc_io_status_read,
	enum_acc_io_status_numof,
}enum_acc_io_status;

typedef struct _type_acc_io
{
	enum_acc_io_status status;
	uint16_t Xacc;
	uint16_t Yacc;
	int16_t Xacc_unbiased;
	int16_t Yacc_unbiased;
	int retcodeTransfer;
	int retcodeSend;
	int retcodeCmdRead;
	I2C_XFER_T xfer;
	uint8_t txBuff[16];
	uint8_t rxBuff[16];

	type_acc_io_stats statsX, statsY;
	uint32_t numerr_send1, numerr_send2;
	uint32_t num_readOK;

	uint64_t last_tick_latch_ms;
}type_acc_io;

static void init_stats(type_acc_io_stats * p)
{
	memset(p, 0, sizeof(*p));
	p->min = INT32_MAX;
	p->max = INT32_MIN;
}

static int int32_tcmp(const void *pa, const void *pb)
{
	int32_t ia = *(int32_t*)pa;
	int32_t ib = *(int32_t*)pb;
	return (ia - ib);
}

static void update_stats(type_acc_io_stats * p, int16_t new_value)
{
	uint32_t do_update = 0;
#if 1
	p->value_cur = new_value;
	do_update = 1;
#else
	{
		uint32_t idx_cur = p->idx_cur;
		p->accum_cur += new_value;
#define def_num_shift_filter_min_max_stats 1
#define def_num_filter_min_max_stats (1 << def_num_shift_filter_min_max_stats)
		if (++idx_cur >= def_num_filter_min_max_stats)
		{
			idx_cur = 0;
			p->value_cur = p->accum_cur >> def_num_shift_filter_min_max_stats;
			p->accum_cur = 0;
		}
		p->idx_cur = idx_cur;
		do_update = 1;
	}
#endif
	if (!do_update)
	{
		return;
	}
	uint32_t idx = p->idx;
	if (idx >= def_num_stats_values)
	{
		init_stats(p);
		idx = 0;
	}
	p->accum -= p->values[p->idx];
	p->accum += p->value_cur;
	p->values[p->idx] = p->value_cur;
	if (++idx >= def_num_stats_values)
	{
		idx = 0;
	}
	p->idx = idx;
	p->avg = p->accum / def_num_stats_values;
	memcpy(&p->values_ordered[0], &p->values[0], sizeof(p->values_ordered));
	qsort(&p->values_ordered[0], def_num_stats_values, sizeof(p->values_ordered[0]), int32_tcmp);
	p->min = p->values_ordered[0];
	p->max = p->values_ordered[def_num_stats_values - 1];

	p->nsamples++;
	if (p->nsamples >= 2 * def_num_stats_values)
	{
		p->valid = 1;
	}
}

#if 0
static void rewind_stats(type_acc_io_stats * p, int32_t rewind_val)
{
	init_stats(p);
	update_stats(p, rewind_val);
}
#endif

static type_acc_io acc_io;

void accelerometer_module_init(void)
{
	memset(&acc_io, 0, sizeof(acc_io));
	init_stats(&acc_io.statsX);
	init_stats(&acc_io.statsY);
}

void accelerometer_module_handle_run(void)
{
	switch(acc_io.status)
	{
		case enum_acc_io_status_init:
		default:
		{
			init_i2c();
			acc_io.xfer.txBuff = &acc_io.txBuff[0];
			acc_io.xfer.rxBuff = &acc_io.rxBuff[0];
			acc_io.xfer.rxSz = 0;
			acc_io.xfer.txSz = 0;

			acc_io.xfer.slaveAddr = 0x10; // the MXC62020GMW I2C address

			acc_io.status = enum_acc_io_status_latch;
			break;
		}
		case enum_acc_io_status_latch:
		{
			uint64_t now = get_tick_count();
			if (acc_io.last_tick_latch_ms == now)
			{
				return;
			}
			acc_io.last_tick_latch_ms = now;

			acc_io.status = enum_acc_io_status_read;
			// latch the new value
			acc_io.xfer.txBuff = &acc_io.txBuff[0];
			acc_io.xfer.txSz = 2;
			acc_io.xfer.rxSz = 0;
			acc_io.txBuff[0] = 0; // select register 0
			acc_io.txBuff[1] = 0; // select latch acceleration data
			acc_io.xfer.rxBuff = &acc_io.rxBuff[0];
			acc_io.xfer.slaveAddr = 0x10; // the MXC62020GMW I2C address
			acc_io.retcodeTransfer = do_i2c_transfer( &acc_io.xfer);
			if (acc_io.retcodeTransfer)
			{
				acc_io.numerr_send1++;
				acc_io.status = enum_acc_io_status_init;
				break;
			}
			break;
		}
		case enum_acc_io_status_read:
		{
			acc_io.status = enum_acc_io_status_latch;
			// read the current accelerations values
			acc_io.xfer.txSz = 1;
			acc_io.txBuff[0] = 1; // select register 1
			acc_io.xfer.rxSz = 4; // expect 4 data back: XH, XL, YH, YL
			acc_io.xfer.rxBuff = &acc_io.rxBuff[0];
			acc_io.xfer.txBuff = &acc_io.txBuff[0];
			acc_io.xfer.slaveAddr = 0x10; // the MXC62020GMW I2C address
			acc_io.retcodeTransfer =  do_i2c_transfer( &acc_io.xfer);
			if (acc_io.retcodeTransfer)
			{
				acc_io.numerr_send2++;
				acc_io.status = enum_acc_io_status_init;
				break;
			}

			acc_io.num_readOK++;

			{
				uint16_t Xacc = acc_io.rxBuff[0];
				Xacc <<= 8;
				Xacc |=  acc_io.rxBuff[1];
				acc_io.Xacc = Xacc;
				int16_t Xacc_unbiased;
				Xacc_unbiased = Xacc;
				Xacc_unbiased -= def_center_acc_value;
				acc_io.Xacc_unbiased = Xacc_unbiased;
				update_stats(&acc_io.statsX, Xacc_unbiased);
			}
			{
				uint16_t Yacc = acc_io.rxBuff[2];
				Yacc <<= 8;
				Yacc |=  acc_io.rxBuff[3];
				acc_io.Yacc = Yacc;
				int16_t Yacc_unbiased;
				Yacc_unbiased = Yacc;
				Yacc_unbiased -= def_center_acc_value;
				acc_io.Yacc_unbiased = Yacc_unbiased;
				update_stats(&acc_io.statsY, Yacc_unbiased);
			}
			break;
		}
	}
}

#if 0
void reset_accelerometer_stats(void)
{
	rewind_stats(&acc_io.statsX, acc_io.Xacc_unbiased);
	rewind_stats(&acc_io.statsY, acc_io.Yacc_unbiased);
}
#endif
unsigned int ui_acc_stats_valid(void)
{
	if (acc_io.statsY.valid && acc_io.statsX.valid)
	{
		return 1;
	}
	return 0;
}
void refresh_accelerometer_info(type_accelerometer_main_info *pdst)
{
	pdst->num_readOK = acc_io.num_readOK;
	pdst->numerr_send1 = acc_io.numerr_send1;
	pdst->numerr_send2 = acc_io.numerr_send2;
	pdst->acc_mms2[0] = ((acc_io.Xacc_unbiased * 10000) >> 9);
	pdst->acc_mms2[1] = ((acc_io.Yacc_unbiased * 10000) >> 9);
	pdst->min_acc_mms2[0] = (acc_io.statsX.min * 10000) >> 9;
	pdst->min_acc_mms2[1] = (acc_io.statsY.min * 10000) >> 9;
	pdst->max_acc_mms2[0] = (acc_io.statsX.max * 10000) >> 9;
	pdst->max_acc_mms2[1] = (acc_io.statsY.max * 10000) >> 9;
}

void test_accelerometer(void)
{

	accelerometer_module_init();

	while(1)
	{
		accelerometer_module_handle_run();
	}
}
