
#ifndef __APP_USART_H__


#define __APP_USART_H__

#include "stm32f4xx.h"

#include "stm32f4xx_ll_dma.h"

#include "stm32f4xx_ll_usart.h"

#include <stdbool.h>

extern uint8_t Usart_DMA_Init_TX(uint8_t *TxBuffer,uint16_t size);
extern void set_dam_tc_flag(uint8_t tc_status);
extern uint8_t get_dam_tc_flag(void);
extern bool usart_is_busy(void);

#endif 




