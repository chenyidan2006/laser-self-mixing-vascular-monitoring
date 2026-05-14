
#include "app_usart.h"

#define USART_IDLE      0
#define USART_TXING     1

uint8_t usart_status;

uint8_t Usart_DMA_Init_TX(uint8_t *TxBuffer,uint16_t size)
{
    // 配置DMA发送通道（USART1_TX使用DMA2 Stream7 Channel4）
    // LL_DMA_ConfigTransfer(DMA2, LL_DMA_STREAM_7,
    //                       LL_DMA_DIRECTION_MEMORY_TO_PERIPH |
    //                       LL_DMA_MODE_NORMAL |
    //                       LL_DMA_PERIPH_NOINCREMENT |
    //                       LL_DMA_MEMORY_INCREMENT |
    //                       LL_DMA_PDATAALIGN_BYTE |
    //                       LL_DMA_MDATAALIGN_BYTE |
    //                       LL_DMA_PRIORITY_HIGH);

		if(usart_status != USART_IDLE)
			return 0;
    usart_status = USART_TXING;

    // 设置源地址（发送缓冲区）
    LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_7, (uint32_t)TxBuffer);
    
    // 设置目标地址（USART1数据寄存器）
    LL_DMA_SetPeriphAddress(DMA2, LL_DMA_STREAM_7, (uint32_t)&USART1->DR);
    
    // 选择通道4（USART1_TX）
    LL_DMA_SetChannelSelection(DMA2, LL_DMA_STREAM_7, LL_DMA_CHANNEL_4);


    //配置数据长度
    LL_DMA_SetDataLength(DMA2,LL_DMA_STREAM_7,size);

    // 使能传输完成中断
    LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_7);

        // 使能传输完成中断
    LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_7);
    LL_DMA_EnableIT_TE(DMA2, LL_DMA_STREAM_7);
    
    // 使能USART1的DMA发送
    LL_USART_EnableDMAReq_TX(USART1);

    //使能DMA STREAM  也就是发送数据
    LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_7);
	return 1;
}

uint8_t tx_end_flag;


void DMA2_Stream7_IRQHandler(void)
{
    if(LL_DMA_IsActiveFlag_TC7(DMA2))
    {
    	//清除Stream7 TC中断
        LL_DMA_ClearFlag_TC7(DMA2);
        //关闭DMA2 Stream7
        LL_DMA_DisableStream(DMA2,LL_DMA_STREAM_7);

        tx_end_flag = 1;
        usart_status = USART_IDLE;
    }
    if(LL_DMA_IsActiveFlag_TE7(DMA2))
    {
    	//清除Stream7 TC中断
        LL_DMA_ClearFlag_TE7(DMA2);
        //关闭DMA2 Stream7
        while (1)
        {
            
        }
        
    }
}

void set_dam_tc_flag(uint8_t tc_status)
{

    tx_end_flag = tc_status;
}

uint8_t get_dam_tc_flag(void)
{

    return tx_end_flag;
}

bool usart_is_busy(void)
{
    if (usart_status == USART_TXING)
    {
        return true;
    }
    else
    {
        
        return false;

    }
}
