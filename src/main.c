/*
 * main.c
 *
 *  Created on: Apr 5, 2017
 *      Author: michele
 */


#include "board.h"
#include <stdio.h>
#include "uart.h"
#include "i2c.h"
#include "dipswitches.h"
#include "accelerometer.h"
#include "encoder.h"
#include "system_tick.h"

//using internal oscillator
const uint32_t OscRateIn = 0;

STATIC const PINMUX_GRP_T pinmuxing[] = {
// I2C
	{(uint32_t)IOCON_PIO0_4,  (IOCON_FUNC1 | IOCON_RESERVED_BIT_6 | IOCON_RESERVED_BIT_7 | IOCON_FASTI2C_EN)}, /* PIO0_4 used for SCL */
	{(uint32_t)IOCON_PIO0_5,  (IOCON_FUNC1 | IOCON_RESERVED_BIT_6 | IOCON_RESERVED_BIT_7 | IOCON_FASTI2C_EN)}, /* PIO0_5 used for SDA */
// DIP SWITCH 1
	{(uint32_t)IOCON_PIO0_3,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_HYS_EN)}, // PIO0_3: DIP1-1
	{(uint32_t)IOCON_PIO0_6,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_HYS_EN)}, // PIO0_6: DIP1-2
	{(uint32_t)IOCON_PIO1_8,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_HYS_EN)}, // PIO1_9: DIP1-3
	{(uint32_t)IOCON_PIO1_9,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_HYS_EN)}, // PIO1_9: DIP1-4
	{(uint32_t)IOCON_PIO3_0,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_HYS_EN)}, // PIO3_0: DIP1-5
	{(uint32_t)IOCON_PIO3_1,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_HYS_EN)}, // PIO3_1: DIP1-6
	{(uint32_t)IOCON_PIO3_2,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_HYS_EN)}, // PIO3_2: DIP1-7
	{(uint32_t)IOCON_PIO3_3,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_HYS_EN)}, // PIO3_3: DIP1-8
// IN_ENC PIO1_0
	{(uint32_t)IOCON_PIO1_0,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_HYS_EN)},
// UART PIO1_6..7
	{(uint32_t)IOCON_PIO1_6,  (IOCON_FUNC1 | IOCON_RESERVED_BIT_6 | IOCON_RESERVED_BIT_7 | IOCON_MODE_INACT)}, /* PIO1_6 used for RXD */
	{(uint32_t)IOCON_PIO1_7,  (IOCON_FUNC1 | IOCON_RESERVED_BIT_6 | IOCON_RESERVED_BIT_7 | IOCON_MODE_INACT)}, /* PIO1_7 used for TXD */
// DIP SWITCH 2: PIO2_0..3
	{(uint32_t)IOCON_PIO2_0,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_HYS_EN)},
	{(uint32_t)IOCON_PIO2_1,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_HYS_EN)},
	{(uint32_t)IOCON_PIO2_2,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_HYS_EN)},
	{(uint32_t)IOCON_PIO2_3,  (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_HYS_EN)},
// red LED
	{(uint32_t)IOCON_PIO3_4,  (IOCON_FUNC0)}, // PIO3_4: red LED
// green LED
	{(uint32_t)IOCON_PIO3_5,  (IOCON_FUNC0)}, // PIO3_5: green LED
// relais PIO1_11
	{(uint32_t)IOCON_PIO1_11,  (IOCON_FUNC0)},
// encoder inputs PIO2_4..11
	{(uint32_t)IOCON_PIO2_4,  (IOCON_FUNC0)},
	{(uint32_t)IOCON_PIO2_5,  (IOCON_FUNC0)},
	{(uint32_t)IOCON_PIO2_6,  (IOCON_FUNC0)},
	{(uint32_t)IOCON_PIO2_7,  (IOCON_FUNC0)},
	{(uint32_t)IOCON_PIO2_8,  (IOCON_FUNC0)},
	{(uint32_t)IOCON_PIO2_9,  (IOCON_FUNC0)},
	{(uint32_t)IOCON_PIO2_10,  (IOCON_FUNC0)},
	{(uint32_t)IOCON_PIO2_11,  (IOCON_FUNC0)},
#if 0
	{(uint32_t)IOCON_PIO0_1,  (IOCON_FUNC1 | IOCON_RESERVED_BIT_6 | IOCON_RESERVED_BIT_7 | IOCON_MODE_INACT)}, /* PIO0_1 used for CLKOUT */
	{(uint32_t)IOCON_PIO0_2,  (IOCON_FUNC1 | IOCON_RESERVED_BIT_6 | IOCON_RESERVED_BIT_7 | IOCON_MODE_PULLUP)},/* PIO0_2 used for SSEL */
	{(uint32_t)IOCON_PIO0_3,  (IOCON_FUNC1 | IOCON_RESERVED_BIT_6 | IOCON_RESERVED_BIT_7 | IOCON_MODE_INACT)}, /* PIO0_3 used for USB_VBUS */
	{(uint32_t)IOCON_PIO0_4,  (IOCON_FUNC1 | IOCON_RESERVED_BIT_6 | IOCON_RESERVED_BIT_7 | IOCON_FASTI2C_EN)}, /* PIO0_4 used for SCL */
	{(uint32_t)IOCON_PIO0_5,  (IOCON_FUNC1 | IOCON_RESERVED_BIT_6 | IOCON_RESERVED_BIT_7 | IOCON_FASTI2C_EN)}, /* PIO0_5 used for SDA */
	{(uint32_t)IOCON_PIO0_6,  (IOCON_FUNC1 | IOCON_RESERVED_BIT_6 | IOCON_RESERVED_BIT_7 | IOCON_MODE_INACT)}, /* PIO0_6 used for USB_CONNECT */
	{(uint32_t)IOCON_PIO0_8,  (IOCON_FUNC1 | IOCON_RESERVED_BIT_6 | IOCON_RESERVED_BIT_7 | IOCON_MODE_INACT)}, /* PIO0_8 used for MISO */
	{(uint32_t)IOCON_PIO0_9,  (IOCON_FUNC1 | IOCON_RESERVED_BIT_6 | IOCON_RESERVED_BIT_7 | IOCON_MODE_INACT)}, /* PIO0_9 used for MOSI */
	{(uint32_t)IOCON_PIO0_11, (IOCON_FUNC2 | IOCON_RESERVED_BIT_6 | IOCON_ADMODE_EN      | IOCON_FILT_DIS)},   /* PIO0_11 used for AD0 */
	{(uint32_t)IOCON_PIO1_6,  (IOCON_FUNC1 | IOCON_RESERVED_BIT_6 | IOCON_RESERVED_BIT_7 | IOCON_MODE_INACT)}, /* PIO1_6 used for RXD */
	{(uint32_t)IOCON_PIO1_7,  (IOCON_FUNC1 | IOCON_RESERVED_BIT_6 | IOCON_RESERVED_BIT_7 | IOCON_MODE_INACT)}, /* PIO1_7 used for TXD */
	{(uint32_t)IOCON_PIO2_11, (IOCON_FUNC1 | IOCON_RESERVED_BIT_6 | IOCON_RESERVED_BIT_7 | IOCON_MODE_INACT)}, /* PIO2_11 used for SCK */
#endif
};

/* Sets up system pin muxing */
void Board_SetupMuxing(void)
{
	/* Enable IOCON clock */
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_IOCON);

	Chip_IOCON_SetPinMuxing(LPC_IOCON, pinmuxing, sizeof(pinmuxing) / sizeof(PINMUX_GRP_T));
}


int main(void)
{
	/* Generic Initialization */
	SystemCoreClockUpdate();
	Board_SetupMuxing();

// modules initializations
	{
		init_uart();
		tx_uart((uint8_t*)"hello\r\n", 7);
	}
	init_i2c();
	encoder_module_init();
	accelerometer_module_init();
	system_tick_module_init();


	encoder_module_register_callbacks();

	// test_accelerometer();

	{
		typedef enum
		{
			enum_run_status_idle = 0,
			enum_run_status_init,
			enum_run_status_run,

			enum_run_status_numof
		}enum_run_status;
		enum_run_status s = enum_run_status_idle;

		while(1)
		{
			switch(s)
			{
				case enum_run_status_idle:
				default:
				{
					s = enum_run_status_init;
					break;
				}
				case enum_run_status_init:
				{
					accelerometer_module_init();
					s = enum_run_status_run;
					break;
				}
				case enum_run_status_run:
				{
					accelerometer_module_handle_run();
					encoder_module_handle_run();
					break;
				}
			}
		}
	}



	/* Should not run to here */
	return 0;
}
