/*
 * uart.h
 *
 *  Created on: Apr 5, 2017
 *      Author: michele
 */

#ifndef INC_UART_H_
#define INC_UART_H_

void init_uart(void);
unsigned int tx_uart(uint8_t *p, uint32_t numof);
unsigned int rx_uart(uint8_t *p, uint32_t numof);
void deinit_uart(void);

#endif /* INC_UART_H_ */
