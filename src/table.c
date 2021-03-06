/*
 * table.c

 *
 *  Created on: Apr 10, 2017
 *      Author: michele
 */
#if ! def_local_main
#include "board.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eeprom.h"
#include "crc3bit.h"
#endif
#include "table.h"
#if def_table_in_code || def_local_main

	typedef struct _type_code_table
	{
		unsigned int idx;
		const char *label;
		type_rom_table_entry entry;
	}type_code_table;
#if def_64_dipswitch_codes || def_use2bits_for_crc
	static const type_code_table code_table[] =
	{
			{	.idx = 1,
				.label= "LM30_MV",
				.entry =
				{
					.threshold_rpm_X = 100,
					.threshold_rpm_Y = 150,
					.threshold_amplitude_X_um = 10000,
					.threshold_amplitude_Y_um = 18500,
				}
			},

			{	.idx = 3,
				.label= "LM30_MN",
				.entry =
				{
					.threshold_rpm_X = 100,
					.threshold_rpm_Y = 150,
					.threshold_amplitude_X_um = 10200,
					.threshold_amplitude_Y_um = 19000,
				}
			},

			{	.idx = 9,
				.label= "LM55_MV",
				.entry =
				{
					.threshold_rpm_X = 100,
					.threshold_rpm_Y = 150,
					.threshold_amplitude_X_um = 9800,
					.threshold_amplitude_Y_um = 16300,
				}
			},

			{	.idx = 10,
				.label= "LM55_MV",
				.entry =
				{
					.threshold_rpm_X = 100,
					.threshold_rpm_Y = 150,
					.threshold_amplitude_X_um = 9800,
					.threshold_amplitude_Y_um = 26000,
				}
			},

			{	.idx = 13,
				.label= "LM70_MV",
				.entry =
				{
					.threshold_rpm_X =  90,
					.threshold_rpm_Y = 150,
					.threshold_amplitude_X_um = 19600,
					.threshold_amplitude_Y_um = 17500,
				}
			},

			{	.idx = 14,
				.label= "LM70_MV_EM",
				.entry =
				{
					.threshold_rpm_X =  90,
					.threshold_rpm_Y = 150,
					.threshold_amplitude_X_um = 19600,
					.threshold_amplitude_Y_um = 19000,
				}
			},

			{	.idx = 17,
				.label= "LM85_MV",
				.entry =
				{
					.threshold_rpm_X = 100,
					.threshold_rpm_Y = 150,
					.threshold_amplitude_X_um = 14000,
					.threshold_amplitude_Y_um = 10000,
				}
			},

			{	.idx = 18,
				.label= "LM85_EM_MV",
				.entry =
				{
					.threshold_rpm_X = 100,
					.threshold_rpm_Y = 150,
					.threshold_amplitude_X_um = 14000,
					.threshold_amplitude_Y_um = 14600,
				}
			},

			{	.idx = 19,
				.label= "LM85_MN",
				.entry =
				{
					.threshold_rpm_X = 100,
					.threshold_rpm_Y = 150,
					.threshold_amplitude_X_um = 16000,
					.threshold_amplitude_Y_um = 11000,
				}
			},

			{	.idx = 20,
				.label= "LM85_EM_MN",
				.entry =
				{
					.threshold_rpm_X = 100,
					.threshold_rpm_Y = 150,
					.threshold_amplitude_X_um = 16000,
					.threshold_amplitude_Y_um = 15600,
				}
			},

			{	.idx = 21,
				.label= "LM100",
				.entry =
				{
					.threshold_rpm_X = 100,
					.threshold_rpm_Y = 100,
					.threshold_amplitude_X_um = 11500,
					.threshold_amplitude_Y_um = 33500,
				}
			},

			{	.idx = 22,
				.label= "LM100_EM",
				.entry =
				{
					.threshold_rpm_X = 100,
					.threshold_rpm_Y = 100,
					.threshold_amplitude_X_um = 11500,
					.threshold_amplitude_Y_um = 35500,
				}
			},

			{	.idx = 23,
				.label= "LM100_EM2",
				.entry =
				{
					.threshold_rpm_X = 100,
					.threshold_rpm_Y = 100,
					.threshold_amplitude_X_um = 12500,
					.threshold_amplitude_Y_um = 36500,
				}
			},

			{	.idx = 24,
				.label= "LM125",
				.entry =
				{
					.threshold_rpm_X = 100,
					.threshold_rpm_Y = 100,
					.threshold_amplitude_X_um = 11000,
					.threshold_amplitude_Y_um = 32500,
				}
			},

			{	.idx = 25,
				.label= "LM125_EM",
				.entry =
				{
					.threshold_rpm_X = 100,
					.threshold_rpm_Y = 100,
					.threshold_amplitude_X_um = 11000,
					.threshold_amplitude_Y_um = 36000,
				}
			},

			{	.idx = 26,
				.label= "LM125_EM2",
				.entry =
				{
					.threshold_rpm_X = 100,
					.threshold_rpm_Y = 100,
					.threshold_amplitude_X_um = 12500,
					.threshold_amplitude_Y_um = 39000,
				}
			},

			{	.idx = 27,
				.label= "D2W30_MV",
				.entry =
				{
					.threshold_rpm_X = 150,
					.threshold_rpm_Y = 150,
					.threshold_amplitude_X_um =  8500,
					.threshold_amplitude_Y_um = 15500,
				}
			},

			{	.idx = 31,
				.label= "D2W55_MV",
				.entry =
				{
					.threshold_rpm_X = 100,
					.threshold_rpm_Y = 150,
					.threshold_amplitude_X_um =  9400,
					.threshold_amplitude_Y_um = 20000,
				}
			},
	};

#else
	static const type_code_table code_table[] =
	{
			{	.idx = 1,
				.label= "LM30_MV",
				.entry =
				{
					.threshold_rpm_X = 100,
					.threshold_rpm_Y = 150,
					.threshold_amplitude_X_um = 10000,
					.threshold_amplitude_Y_um = 18500,
				}
			},

			{	.idx = 3,
				.label= "LM30_MN",
				.entry =
				{
					.threshold_rpm_X = 100,
					.threshold_rpm_Y = 150,
					.threshold_amplitude_X_um = 10200,
					.threshold_amplitude_Y_um = 19000,
				}
			},

			{	.idx = 4,
				.label= "LM55",
				.entry =
				{
					.threshold_rpm_X = 100,
					.threshold_rpm_Y = 150,
					.threshold_amplitude_X_um = 9800,
					.threshold_amplitude_Y_um = 16300,
				}
			},

			{	.idx = 12,
				.label= "LM55_EM",
				.entry =
				{
					.threshold_rpm_X = 100,
					.threshold_rpm_Y = 150,
					.threshold_amplitude_X_um = 9800,
					.threshold_amplitude_Y_um = 26000,
				}
			},
#if def_NUOVE_MOLLE_LM_100_VALUES
			{	.idx = 16,
				.label= "LM85",
				.entry =
				{
					.threshold_rpm_X = 100,
					.threshold_rpm_Y = 150,
					.threshold_amplitude_X_um = 15600,
					.threshold_amplitude_Y_um = 10600,
				}
			},
			{	.idx = 17,
				.label= "LM85_EM",
				.entry =
				{
					.threshold_rpm_X = 100,
					.threshold_rpm_Y = 150,
					.threshold_amplitude_X_um = 15600,
					.threshold_amplitude_Y_um = 15600,
				}
			},
#else
			{	.idx = 8,
				.label= "LM70",
				.entry =
				{
					.threshold_rpm_X = 90,
					.threshold_rpm_Y = 150,
					.threshold_amplitude_X_um = 19600,
					.threshold_amplitude_Y_um = 17500,
				}
			},
			{	.idx = 24,
				.label= "LM70_EM",
				.entry =
				{
					.threshold_rpm_X = 90,
					.threshold_rpm_Y = 150,
					.threshold_amplitude_X_um = 19600,
					.threshold_amplitude_Y_um = 19000,
				}
			},
			{	.idx = 16,
				.label= "LM85",
				.entry =
				{
					.threshold_rpm_X = 100,
					.threshold_rpm_Y = 150,
					.threshold_amplitude_X_um = 14000,
					.threshold_amplitude_Y_um = 10000,
				}
			},
			{	.idx = 17,
				.label= "LM85_EM",
				.entry =
				{
					.threshold_rpm_X = 100,
					.threshold_rpm_Y = 150,
					.threshold_amplitude_X_um = 14000,
					.threshold_amplitude_Y_um = 14600,
				}
			},
#endif
#if def_DE_LONGO_VALUES
			{	.idx = 11,
				.label= "LM85_EM2",
				.entry =
				{
					.threshold_rpm_X = 100,
					.threshold_rpm_Y = 150,
					.threshold_amplitude_X_um = 16000,
					.threshold_amplitude_Y_um = 18000,
				}
			},
#endif
			{	.idx = 9,
				.label= "LM100",
				.entry =
				{
					.threshold_rpm_X = 100,
					.threshold_rpm_Y = 100,
					.threshold_amplitude_X_um = 11500,
					.threshold_amplitude_Y_um = 33500,
				}
			},

			{	.idx = 5,
				.label= "LM100_EM",
				.entry =
				{
					.threshold_rpm_X = 100,
					.threshold_rpm_Y = 100,
					.threshold_amplitude_X_um = 11500,
					.threshold_amplitude_Y_um = 35500,
				}
			},

			{	.idx = 7,
				.label= "LM100_EM2",
				.entry =
				{
					.threshold_rpm_X = 100,
					.threshold_rpm_Y = 100,
					.threshold_amplitude_X_um = 12500,
					.threshold_amplitude_Y_um = 36500,
				}
			},

			{	.idx = 18,
				.label= "LM125",
				.entry =
				{
					.threshold_rpm_X = 100,
					.threshold_rpm_Y = 100,
					.threshold_amplitude_X_um = 11000,
					.threshold_amplitude_Y_um = 32500,
				}
			},

			{	.idx = 10,
				.label= "LM125_EM",
				.entry =
				{
					.threshold_rpm_X = 100,
					.threshold_rpm_Y = 100,
					.threshold_amplitude_X_um = 11000,
					.threshold_amplitude_Y_um = 36000,
				}
			},

			{	.idx = 15,
				.label= "LM125_EM2",
				.entry =
				{
					.threshold_rpm_X = 100,
					.threshold_rpm_Y = 100,
					.threshold_amplitude_X_um = 12500,
					.threshold_amplitude_Y_um = 39000,
				}
			},

			{	.idx = 21,
				.label= "D2W30",
				.entry =
				{
					.threshold_rpm_X = 150,
					.threshold_rpm_Y = 150,
					.threshold_amplitude_X_um = 8500,
					.threshold_amplitude_Y_um = 15500,
				}
			},
			{	.idx = 20,
				.label= "D2W55",
				.entry =
				{
					.threshold_rpm_X = 100,
					.threshold_rpm_Y = 150,
					.threshold_amplitude_X_um = 9400,
					.threshold_amplitude_Y_um = 20000,
				}
			},

	};
#endif

	static const type_code_table * p_last_found = NULL;
	const char *p_get_rom_table_last_found_label(void)
	{
		if (!p_last_found)
		{
			return "";
		}
		return p_last_found->label;
	}

	unsigned int is_OK_get_rom_table_entry(uint8_t idx, type_rom_table_entry *p_dst)
	{
		unsigned int is_OK = 0;
		unsigned int idx_table;
		p_last_found = NULL;
		for (idx_table = 0; !is_OK && idx_table < sizeof(code_table)/sizeof(code_table[0]); idx_table++)
		{
			const type_code_table *p_cur_table = &code_table[idx_table];
			if (p_cur_table->idx == idx)
			{
				is_OK = 1;
				*p_dst = p_cur_table->entry;
				p_last_found = p_cur_table;
			}
		}
		return is_OK;
	}

	unsigned int is_OK_write_rom_table_entry(uint8_t idx, type_rom_table_entry *p_src)
	{
		return 0;
	}
#else
//#error compiling to eeprom+serial board!

	static uint32_t crc32_tab[] = {
		0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
		0xe963a535, 0x9e6495a3,	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
		0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
		0xf3b97148, 0x84be41de,	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
		0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,	0x14015c4f, 0x63066cd9,
		0xfa0f3d63, 0x8d080df5,	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
		0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,	0x35b5a8fa, 0x42b2986c,
		0xdbbbc9d6, 0xacbcf940,	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
		0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
		0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
		0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,	0x76dc4190, 0x01db7106,
		0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
		0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
		0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
		0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
		0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
		0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
		0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
		0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
		0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
		0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
		0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
		0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
		0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
		0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
		0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
		0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
		0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
		0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
		0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
		0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
		0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
		0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
		0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
		0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
		0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
		0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
		0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
		0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
		0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
		0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
		0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
		0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
	};

	uint32_t crc32(const void *buf, size_t size)
	{
		uint32_t crc;
		const uint8_t *p;

		p = buf;
		crc = 0;

		while (size--)
		{
			crc = crc32_tab[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
		}

		return crc;
	}

	unsigned int is_OK_get_rom_table_entry(uint8_t idx, type_rom_table_entry *p_dst)
	{
		unsigned int is_OK = 1;
		uint32_t offset = idx;
		offset *= sizeof(type_rom_table_entry);

		unsigned int idx_byte;
		uint8_t * p_read_dst = (uint8_t *)p_dst;
		for (idx_byte = 0; is_OK && idx_byte < sizeof(type_rom_table_entry); idx_byte++)
		{
			if (!is_OK_read_EEPROM_24LC512_byte(offset + idx_byte, p_read_dst))
			{
				is_OK = 0;
			}
			else
			{
				p_read_dst++;
			}
		}
		if (is_OK)
		{
			uint32_t get_crc = crc32((const void *)(((uint8_t*)&p_dst->crc32) + sizeof(p_dst->crc32)), sizeof(type_rom_table_entry) - sizeof(p_dst->crc32));
			if (get_crc != p_dst->crc32)
			{
				is_OK = 0;
			}
		}
		if (!is_OK)
		{
			memset(p_dst, 0, sizeof(*p_dst));
		}
		return is_OK;
	}

	const char *p_get_rom_table_last_found_label(void)
	{
		return "";
	}

	unsigned int is_OK_write_rom_table_entry(uint8_t idx, type_rom_table_entry *p_src)
	{
		unsigned int is_OK = 1;
		uint32_t offset = idx;
		offset *= sizeof(type_rom_table_entry);
		uint32_t set_crc = crc32((const void *)(((uint8_t*)&p_src->crc32) + sizeof(p_src->crc32)), sizeof(type_rom_table_entry) - sizeof(p_src->crc32));
		p_src->crc32 = set_crc;
		if (!is_OK_write_EEPROM_24LC512_array(offset, (uint8_t*)p_src, sizeof(type_rom_table_entry)))
		{
			is_OK = 0;
		}
		return is_OK;
	}

	void test_table(void)
	{
		type_rom_table_entry test_entry_src;
		test_entry_src.threshold_amplitude_X_um = 150;
		test_entry_src.threshold_amplitude_Y_um = 151;
		test_entry_src.threshold_rpm_X = 107;
		test_entry_src.threshold_rpm_Y = 108;
		unsigned int is_OK = 1;
		unsigned int idx_table;
		for (idx_table = 0; idx_table < 3; idx_table++)
		{
			if (is_OK)
			{
				is_OK = is_OK_write_rom_table_entry(idx_table, &test_entry_src);
			}
			type_rom_table_entry test_entry_dst;
			memset(&test_entry_dst, 0, sizeof(test_entry_dst));
			if (is_OK)
			{
				is_OK = is_OK_get_rom_table_entry(idx_table, &test_entry_dst);
			}
			if (is_OK)
			{
				is_OK = memcmp(&test_entry_dst, &test_entry_src, sizeof(test_entry_dst)) == 0 ? 1 : 0;
			}
		}
		if (!is_OK)
		{
			while(1);
		}
	}
#endif

#if def_local_main
void print_the_table(void)
{
#include <stdio.h>
#include <stdlib.h>
	printf ("Start of %s\r\n", __func__);
	FILE *f = fopen("IM8_1496_table.txt", "wb");
	if (!f)
	{
		printf("error opening the output file\r\n");
		return;
	}
	unsigned int i;
	for (i = 0; i < sizeof(code_table)/sizeof(code_table[0]); i++)
	{
		const type_code_table * p = &code_table[i];
		if (fprintf(f, "%-10.10s %2u, rpmX=%5u, rpmY=%5u, thrX=%5u, thrY=%5u\r\n"
					, p->label
					, p->idx
					, p->entry.threshold_rpm_X
					, p->entry.threshold_rpm_Y
					, p->entry.threshold_amplitude_X_um
					, p->entry.threshold_amplitude_Y_um
				) < 0)
			{
			printf("error doing fprintf\r\n");
			break;
			}
	}
	if (fclose(f) < 0)
	{
		printf ("error doing fclose\r\n");
	}
	printf ("End of %s\r\n", __func__);
}
#endif
