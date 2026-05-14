
#ifndef    __USER_SPI_H__

#define __USER_SPI_H__


#include "stm32f4xx.h"
#include "stm32f4xx_ll_spi.h"

extern uint16_t spi_transmit_receive8(SPI_TypeDef *spi,uint8_t *tx,uint8_t *rx,uint16_t size);
extern uint16_t spi_transmit_receive16(SPI_TypeDef *spi,uint16_t *tx,uint16_t *rx,uint16_t size);



#endif

