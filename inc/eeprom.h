/*
 * eeprom.h
 *
 *  Created on: Apr 10, 2017
 *      Author: michele
 */

#ifndef INC_EEPROM_H_
#define INC_EEPROM_H_

unsigned int is_OK_read_EEPROM_24LC512_byte(unsigned int byte_address, unsigned char* puc_byte_value);
unsigned int is_OK_write_EEPROM_24LC512_byte(unsigned int byte_address, unsigned char byte_value);
unsigned int is_OK_write_EEPROM_24LC512_array(unsigned int byte_address, unsigned char *byte_array, unsigned char byte_numof);

#endif /* INC_EEPROM_H_ */
