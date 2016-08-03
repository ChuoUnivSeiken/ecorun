/*
 * i2c.h
 *
 *  Created on: 2014/06/20
 *      Author: Yoshio
 */

#ifndef I2C_H_
#define I2C_H_

/* If I2C SEEPROM is tested, make sure FAST_MODE_PLUS is 0.
 For board to board test, this flag can be turned on. */

#define FAST_MODE_PLUS	0

#define USART_BUFFER_SIZE             64
#define MAX_TIMEOUT		0x00FFFFFF

#define I2CMASTER		0x01
#define I2CSLAVE		0x02

#define PCF8594_ADDR	0xA0
#define READ_WRITE		0x01

#define RD_BIT			0x01

#define I2C_IDLE			0
#define I2C_STARTED			1
#define I2C_RESTARTED		2
#define I2C_REPEATED_START	3
#define DATA_ACK			4
#define DATA_NACK			5
#define I2C_BUSY              6
#define I2C_NO_DATA           7
#define I2C_NACK_ON_ADDRESS   8
#define I2C_NACK_ON_DATA      9
#define I2C_ARBITRATION_LOST  10
#define I2C_TIME_OUT          11
#define I2C_OK                12

#define I2CONSET_I2EN       (0x1<<6)  /* I2C Control Set Register */
#define I2CONSET_AA         (0x1<<2)
#define I2CONSET_SI         (0x1<<3)
#define I2CONSET_STO        (0x1<<4)
#define I2CONSET_STA        (0x1<<5)

#define I2CONCLR_AAC        (0x1<<2)  /* I2C Control clear Register */
#define I2CONCLR_SIC        (0x1<<3)
#define I2CONCLR_STAC       (0x1<<5)
#define I2CONCLR_I2ENC      (0x1<<6)

#define I2DAT_I2C			0x00000000  /* I2C Data Reg */
#define I2ADR_I2C			0x00000000  /* I2C Slave Address Reg */
#define I2SCLH_SCLH			0x00000180  /* I2C SCL Duty Cycle High Reg */
#define I2SCLL_SCLL			0x00000180  /* I2C SCL Duty Cycle Low Reg */
#define I2SCLH_HS_SCLH		0x00000015  /* Fast Plus I2C SCL Duty Cycle High Reg */
#define I2SCLL_HS_SCLL		0x00000015  /* Fast Plus I2C SCL Duty Cycle Low Reg */

uint32_t i2c_init(uint32_t I2cMode);
uint32_t i2c_start(void);
uint32_t i2c_stop(void);
uint32_t i2c_engine(void);

#endif /* I2C_H_ */
