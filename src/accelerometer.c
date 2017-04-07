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

#define def_center_acc_value 2048
typedef struct _type_acc_io_stats
{
	int32_t min, max, avg;
	uint32_t idx;
	int32_t accum;
	int32_t values[32];
	uint32_t nsamples;
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
}type_acc_io;

static void init_stats(type_acc_io_stats * p)
{
	memset(p, 0, sizeof(*p));
	p->min = INT32_MAX;
	p->max = INT32_MIN;
}

static void update_stats(type_acc_io_stats * p, int16_t new_value)
{
#define def_max_num_elem_values (sizeof(p->values) / sizeof(p->values[0]))
	uint32_t idx = p->idx;
	if (idx >= def_max_num_elem_values)
	{
		idx = 0;
	}
	p->accum -= p->values[p->idx];
	p->accum += new_value;
	p->values[p->idx] = new_value;
	p->avg = p->accum / def_max_num_elem_values;
	if (new_value < p->min)
	{
		p->min = new_value;
	}
	if (new_value > p->max)
	{
		p->max = new_value;
	}
	p->nsamples++;
}

static type_acc_io acc_io;

void accelerometer_module_init(void)
{
	memset(&acc_io, 0, sizeof(acc_io));
	init_stats(&acc_io.statsX);
	init_stats(&acc_io.statsY);
}

void accelerometer_module_handle(void)
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
			acc_io.status = enum_acc_io_status_read;
			// latch the new value
			acc_io.xfer.txBuff = &acc_io.txBuff[0];
			acc_io.xfer.txSz = 2;
			acc_io.xfer.rxSz = 0;
			acc_io.txBuff[0] = 0; // select register 0
			acc_io.txBuff[1] = 0; // select latch acceleration data
			acc_io.xfer.rxBuff = &acc_io.rxBuff[0];
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
			acc_io.retcodeTransfer =  do_i2c_transfer( &acc_io.xfer);
			if (acc_io.retcodeTransfer)
			{
				acc_io.numerr_send2++;
				acc_io.status = enum_acc_io_status_init;
				break;
			}

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

void test_accelerometer(void)
{

	accelerometer_module_init();

	while(1)
	{
		accelerometer_module_handle();
	}
}
