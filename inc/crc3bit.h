/*
 * crc3bit.h
 *
 *  Created on: Apr 19, 2017
 *      Author: michele
 */

#ifndef INC_CRC3BIT_H_
#define INC_CRC3BIT_H_

uint32_t calc_crc_3bit_of_6bit_number(uint32_t n);
uint32_t is_OK_crc_3bit_of_6bit_number(uint32_t n, uint32_t crc);
void test_crc3(void);

#endif /* INC_CRC3BIT_H_ */
