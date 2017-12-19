/*
 * eeprom.c
 *
 *  Created on: Apr 10, 2017
 *      Author: michele
 */
#include <stdlib.h>
#include <string.h>
#include "board.h"
#include "i2c.h"
#include "eeprom.h"
#include "system_tick.h"

#define def_24LC512_address 0x50


unsigned int is_OK_write_EEPROM_24LC512_byte(unsigned int byte_address, unsigned char byte_value)
{
	I2C_XFER_T xfer={0};
	unsigned char buf_tx[16]={0};

	buf_tx[0]=byte_address>>8;
	buf_tx[1]=byte_address&0xFF;
	buf_tx[2]=byte_value;

	xfer.slaveAddr=def_24LC512_address;
	xfer.txBuff=buf_tx;
	xfer.txSz=3;
	unsigned int ui_transferred_bytes=0;
	ui_transferred_bytes = Chip_I2C_MasterSend(I2C0, xfer.slaveAddr, xfer.txBuff, xfer.txSz);
	// after a write, at least 30ms are needed before the EEPROm can reply
	delay_ms(50);

	if (ui_transferred_bytes!=xfer.txSz)
	{
		return 0;
	}
	return 1;
}
unsigned int is_OK_write_EEPROM_24LC512_array(unsigned int byte_address, unsigned char *byte_array, unsigned char byte_numof)
{
	if (byte_numof > 128)
	{
		return 0;
	}
	I2C_XFER_T xfer={0};
	unsigned char buf_tx[140]={0};

	buf_tx[0]=byte_address>>8;
	buf_tx[1]=byte_address&0xFF;
	memcpy(&buf_tx[2], byte_array, byte_numof);

	xfer.slaveAddr=def_24LC512_address;
	xfer.txBuff=buf_tx;
	xfer.txSz=2 + byte_numof;
	unsigned int ui_transferred_bytes=0;
	ui_transferred_bytes = Chip_I2C_MasterSend(I2C0, xfer.slaveAddr, xfer.txBuff, xfer.txSz);
	// after a write, at least 30ms are needed before the EEPROm can reply
	delay_ms(50);
	if (ui_transferred_bytes!=xfer.txSz)
	{
		return 0;
	}
	return 1;
}


unsigned int is_OK_read_EEPROM_24LC512_byte(unsigned int byte_address, unsigned char* puc_byte_value)
{
	unsigned int retcode=1;
	I2C_XFER_T xfer={0};
	unsigned char buf_tx[16]={0};
	unsigned char buf_rx[16]={0};

	buf_tx[0]=byte_address>>8;
	buf_tx[1]=byte_address&0xFF;

	xfer.slaveAddr=def_24LC512_address;
	xfer.txBuff=buf_tx;
	xfer.txSz=2;
	xfer.rxBuff=buf_rx;
	xfer.rxSz=1;
	int ret_value;
	ret_value=Chip_I2C_MasterTransfer(I2C0, &xfer);
	switch(ret_value)
	{
		case I2C_STATUS_DONE:
		{
			*puc_byte_value=buf_rx[0];
			break;
		}
		default:
		{
			retcode=0;
			break;
		}
	}
	return retcode;
}

void test_eeprom(void)
{

	const uint8_t test_array[4] = {0x12, 0x34, 0x56, 0x78};
	uint8_t test_array_dst[4] = {0, 0, 0, 0};
	unsigned int isOK = 1;
	unsigned int i;
	if (isOK)
	{
		for (i = 0; isOK && i < sizeof(test_array)/ sizeof(test_array[0]); i++)
		{
			if (!is_OK_write_EEPROM_24LC512_byte(i, test_array[i]))
			{
				isOK = 0;
			}
		}
	}
	if (isOK)
	{
		for (i = 0; isOK && i < sizeof(test_array_dst)/ sizeof(test_array_dst[0]); i++)
		{
			if (!is_OK_read_EEPROM_24LC512_byte(i, &test_array_dst[i]))
			{
				isOK = 0;
			}
			else if (test_array_dst[i] != test_array[i])
			{
				isOK = 0;
			}
		}
	}
	if (!isOK)
	{
		while(1);
	}
}
