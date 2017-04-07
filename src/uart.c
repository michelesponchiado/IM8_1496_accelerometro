/*
 * uart.c
 *
 *  Created on: Apr 5, 2017
 *      Author: michele
 */


/*
 * @brief UART interrupt example with ring buffers
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "chip.h"
#include "board.h"
#include "string.h"
#include "uart.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/* Transmit and receive ring buffers */
STATIC RINGBUFF_T txring, rxring;

/* Transmit and receive ring buffer sizes */
#define UART_SRB_SIZE 256	/* Send */
#define UART_RRB_SIZE 256	/* Receive */

/* Transmit and receive buffers */
static uint8_t rxbuff[UART_RRB_SIZE], txbuff[UART_SRB_SIZE];

const char inst1[] = "LPC13xx UART example using ring buffers\r\n";
const char inst2[] = "Press a key to echo it back or ESC to quit\r\n";

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	UART interrupt handler using ring buffers
 * @return	Nothing
 */
void UART_IRQHandler(void)
{
	/* Want to handle any errors? Do it here. */

	/* Use default ring buffer handler. Override this with your own
	   code if you need more capability. */
	Chip_UART_IRQRBHandler(LPC_USART, &rxring, &txring);
}

/**
 * @brief	Main UART program body
 * @return	Always returns 1
 */
void init_uart(void)
{
	/* Setup UART for 115.2K8N1 */
	Chip_UART_Init(LPC_USART);
	Chip_UART_SetBaud(LPC_USART, 115200);
	Chip_UART_ConfigData(LPC_USART, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT));
	Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2));
	Chip_UART_SetRS485Flags(LPC_USART, UART_RS485CTRL_DCTRL_EN | UART_RS485CTRL_OINV_1); // enable auto direction set and use RTS HIGH as transmit enable
	Chip_UART_TXEnable(LPC_USART);

	/* Before using the ring buffers, initialize them using the ring
	   buffer init function */
	RingBuffer_Init(&rxring, rxbuff, 1, UART_RRB_SIZE);
	RingBuffer_Init(&txring, txbuff, 1, UART_SRB_SIZE);

	/* Enable receive data and line status interrupt, the transmit interrupt
	   is handled by the driver. */
	Chip_UART_IntEnable(LPC_USART, (UART_IER_RBRINT | UART_IER_RLSINT));

	/* preemption = 1, sub-priority = 1 */
	NVIC_SetPriority(UART0_IRQn, 1);
	NVIC_EnableIRQ(UART0_IRQn);

}

unsigned int tx_uart(uint8_t *p, uint32_t numof)
{
	return Chip_UART_SendRB(LPC_USART, &txring, p, numof);
}

unsigned int rx_uart(uint8_t *p, uint32_t numof)
{
	unsigned int nrx = 0;
	int avail_space = numof;
	while (avail_space > 0) {
		int nbytes = Chip_UART_ReadRB(LPC_USART, &rxring, p, avail_space);
		if (nbytes > 0)
		{
			if (nbytes > avail_space)
			{
				break;
			}
			nrx += nbytes;
			p += nbytes;
			avail_space -= nbytes;
		}
		else
		{
			break;
		}
	}
	return nrx;
}

void deinit_uart(void)
{
	NVIC_DisableIRQ(UART0_IRQn);
	Chip_UART_DeInit(LPC_USART);
}

