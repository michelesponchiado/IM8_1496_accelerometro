/*
 * table.h
 *
 *  Created on: Apr 10, 2017
 *      Author: michele
 */

#ifndef INC_TABLE_H_
#define INC_TABLE_H_

typedef struct _type_rom_table
{
	uint32_t crc32;	// the CRC of the entry
	uint32_t threshold_rpm_X;			// the threshold speed for X axis [rpm]
	uint32_t threshold_rpm_Y;			// the threshold speed for Y axis [rpm]
	uint32_t threshold_amplitude_X_um; 	// it is in microns (accX[m/s2] * 1000 * 1000)/((2*pi*freq)^2)
	uint32_t threshold_amplitude_Y_um; 	// it is in microns accY[m/s2] * 1000 * 1000)/((2*pi*freq)^2)
}__attribute__((packed)) type_rom_table_entry;

unsigned int is_OK_get_rom_table_entry(uint8_t idx, type_rom_table_entry *p_dst);
unsigned int is_OK_write_rom_table_entry(uint8_t idx, type_rom_table_entry *p_src);

#if __PRODUCTION_NO_EEPROM_NO_SERIAL__
#define def_table_in_code 1
#endif

//#define def_DE_LONGO_VALUES 1

#if def_DE_LONGO_VALUES
	#warning def_DE_LONGO_VALUES active!
#endif

//#define def_NUOVE_MOLLE_LM_100_VALUES 1

#if def_NUOVE_MOLLE_LM_100_VALUES
	#warning def_NUOVE_MOLLE_LM_100_VALUES active!
#endif

const char *p_get_rom_table_last_found_label(void);

#endif /* INC_TABLE_H_ */
