/*
 * pin_config.h
 *
 *  Created on: 2014/07/01
 *      Author: Yoshio
 */

#ifndef PIN_CONFIG_H_
#define PIN_CONFIG_H_

#define PIN_FUNC_RESET	 	0x0
#define PIN_FUNC_PIO0_0	 	0x1

#define PIN_FUNC_PIO0_1 	0x0
#define PIN_FUNC_CLKOUT 	0x1
#define PIN_FUNC_CT32B0_MAT2 0x2

#define PIN_FUNC_PIO0_2	 	0x0
#define PIN_FUNC_SSEL0		0x1
#define PIN_FUNC_CT16B0_CAP0	0x2

#define PIN_FUNC_PIO0_3 	0x0

#define PIN_FUNC_PIO0_4 	0x0	// open drain#define PIN_FUNC_SCL		0x1#define PIN_FUNC_PIO0_5 	0x0	// open drain#define PIN_FUNC_SDA		0x1#define PIN_FUNC_PIO0_6 	0x0#define PIN_FUNC_SCK0		0x2
#define PIN_FUNC_PIO0_7 	0x0
#define PIN_FUNC_CTS		0x1

#define PIN_FUNC_PIO0_8 	0x0
#define PIN_FUNC_MISO0		0x1
#define PIN_FUNC_CT16B0_MAT0	0x2

#define PIN_FUNC_PIO0_9 	0x0
#define PIN_FUNC_MOSI0		0x1
#define PIN_FUNC_CT16B0_MAT1	0x2

#define PIN_FUNC_SWCLK		0x0
#define PIN_FUNC_PIO0_10 	0x1
#define PIN_FUNC_SCK0		0x2
#define PIN_FUNC_CT16B0_MAT2	0x3

#define PIN_FUNC_R			0x0
#define PIN_FUNC_PIO0_11 	0x1
#define PIN_FUNC_AD0		0x2
#define PIN_FUNC_CT32B0_MAT3	0x3

#define PIN_FUNC_PIO1_0		0x1
#define PIN_FUNC_AD1		0x2
#define PIN_FUNC_CT32B1_CAP0	0x3

#define PIN_FUNC_PIO1_1		0x1
#define PIN_FUNC_AD2		0x2
#define PIN_FUNC_CT32B1_MAT0	0x3

#define PIN_FUNC_PIO1_2		0x1
#define PIN_FUNC_AD3		0x2
#define PIN_FUNC_CT32B1_MAT1	0x3

#define PIN_FUNC_SWDIO		0x0
#define PIN_FUNC_PIO1_3		0x1
#define PIN_FUNC_AD4		0x2
#define PIN_FUNC_CT32B1_MAT2	0x3

#define PIN_FUNC_PIO1_4		0x0
#define PIN_FUNC_AD5		0x1
#define PIN_FUNC_CT32B1_MAT3	0x2

#define PIN_FUNC_PIO1_5		0x0
#define PIN_FUNC_RTS		0x1
#define PIN_FUNC_CT32B0_CAP0	0x2

#define PIN_FUNC_PIO1_6		0x0
#define PIN_FUNC_RXD		0x1
#define PIN_FUNC_CT32B0_MAT0	0x3

#define PIN_FUNC_PIO1_7		0x0
#define PIN_FUNC_TXD		0x1
#define PIN_FUNC_CT32B0_MAT1	0x2

#define PIN_FUNC_PIO1_8		0x0
#define PIN_FUNC_CT16B1_CAP0		0x1

#define PIN_FUNC_PIO1_9		0x0
#define PIN_FUNC_CT16B1_MAT0	0x1

#define PIN_MODE_INACTIVE	0x00
#define PIN_MODE_PD			0x01
#define PIN_MODE_PU			0x02
#define PIN_MODE_REPEATER	0x03

void conf_set_func(uint32_t* pin, uint8_t func);

#define PIN_ADMODE_ANALOG 0
#define PIN_ADMODE_DIGITAL 1

void conf_set_admode(uint32_t* pin, uint8_t admode);

#endif /* PIN_CONFIG_H_ */
