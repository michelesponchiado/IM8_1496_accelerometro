/*
 * i2c.h
 *
 *  Created on: Apr 7, 2017
 *      Author: michele
 */

#ifndef INC_I2C_H_
#define INC_I2C_H_

void init_i2c(void);
int do_i2c_transfer(I2C_XFER_T *pfer);

#endif /* INC_I2C_H_ */
