#ifndef PERIPHERAL_SSP_H_
#define PERIPHERAL_SSP_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define SSP0_FIFOSIZE 	16       /* SPI read and write buffer size */
#define SSP0_DATASIZE	16
#define SSP0_CPOL 		0
#define SSP0_CPHA 		0

#define SSP1_FIFOSIZE	8       /* SPI read and write buffer size */
#define SSP1_DATASIZE	8
#define SSP1_CPOL 		0
#define SSP1_CPHA 		0

void ssp_clock_slow(uint32_t port);
void ssp_clock_fast(uint32_t port);
void ssp_init(uint32_t port);
void ssp_send(uint32_t port, const uint8_t *buf, uint32_t length);
void ssp_receive(uint32_t port, uint8_t *buf, uint32_t length);
void ssp_exchange(uint32_t port, uint8_t *buf, uint32_t length);

void ssp_send_uint16(uint32_t port, const uint16_t *buf, uint32_t length);
void ssp_receive_uint16(uint32_t port, uint16_t *buf, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif /* PERIPHERAL_SSP_H_ */
