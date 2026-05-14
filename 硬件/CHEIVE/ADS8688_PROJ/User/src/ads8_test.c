

#include "main.h"

#include "ads8688.h"

#include "tim.h"
#include "user_spi.h"
#include "app_usart.h"

#include "string.h"



#define ADS8688_CS_LOW() HAL_GPIO_WritePin(ADS8688_CS_GPIO_Port, ADS8688_CS_Pin, GPIO_PIN_RESET)

#define ADS8688_CS_HIGH() HAL_GPIO_WritePin(ADS8688_CS_GPIO_Port, ADS8688_CS_Pin, GPIO_PIN_SET);






ADS8688_CTRL_t ads8688_ctrl;
uint16_t raw_data[ARRIY_COLUMN][10000];
//uint8_t raw_data[40000];

/**
 * @brief 从ADS8688芯片读取指定地址的数据
 * @param addr 要读取的寄存器地址
 * @return uint8_t 返回从指定地址读取的数据
 */
uint8_t ADS_Prog_Read(uint8_t addr)
{

    // 定义发送缓冲区和接收缓冲区
    uint8_t tx[4] = {((addr << 1) & 0xFE), 0x00, 0x00, 00}; // 发送数据数组，包含地址和空白数据
    uint8_t rx[4] = {0};                                    // 接收数据数组，初始化为0

    ADS8688_CS_LOW();

    spi_transmit_receive8(SPI1, tx, rx, 4);

    ADS8688_CS_HIGH();

    // 返回接收到的第三个字节（rx[2]）作为读取结果
    return rx[2];
}

bool ADS_Prog_Write(uint8_t addr, uint8_t data)
{
    uint8_t tx[4] = {((addr << 1) | 0x01), data, 0x00, 00};
    uint8_t rx[4] = {0};

    ADS8688_CS_LOW();

    spi_transmit_receive8(SPI1, tx, rx, 4);
    ADS8688_CS_HIGH();

    if (rx[2] == data)
    {
        return true;
    }

    return false;
}

uint16_t ADS_Cmd_Write(uint16_t cmd)
{

    uint8_t tx[4] = {cmd >> 8, cmd & 0xFF, 0, 0};
    uint8_t rx[4];

    ADS8688_CS_LOW();

    spi_transmit_receive8(SPI1, tx, rx, 4);

    ADS8688_CS_HIGH();

    return ((rx[2] << 8) | rx[3]);
}

// switch (range)
// {
// case 0x00: // ±2.5*VREF (±10.24V)
//     voltage = (raw / 65535.0f) * 20.48f - 10.24f;
//     break;
// case 0x01: // ±1.25*VREF (±5.12V)
//     voltage = (raw / 65535.0f) * 10.24f - 5.12f;
//     break;
// case 0x02: // ±0.625*VREF (±2.56V)
//     voltage = (raw / 65535.0f) * 5.12f - 2.56f;
//     break;
// case 0x05: // 0-2.5*VREF (0-10.24V)
//     voltage = (raw / 65535.0f) * 10.24f;
//     break;
// case 0x06: // 0-1.25*VREF (0-5.12V)
//     voltage = (raw / 65535.0f) * 5.12f;
//     break;
// default: // 默认处理
//     voltage = (raw / 65535.0f) * 20.48f - 10.24f;
// }

uint8_t ADS8688_Init(void)
{

    HAL_GPIO_WritePin(ADS8688_DAISY_GPIO_Port, ADS8688_DAISY_Pin, GPIO_PIN_RESET);

    // HAL_GPIO_WritePin(ADS8688_DAISY_GPIO_Port, ADS8688_DAISY_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(ADS8688_RST_GPIO_Port, ADS8688_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(50);

    // soft reset
    ADS_Cmd_Write(RST);

    HAL_Delay(1000);
    // enter continue operation
    ADS_Cmd_Write(CONT);

    // configuration channel
    ADS_Prog_Write(CHN_2_RANGE, 0x02);

    // check configuration is successful
    //ADS_Prog_Read(CHN_2_RANGE);

    // ADS_Prog_Write(CHN_PWRDN,0xFB);

    // ADS_Prog_Read(CHN_PWRDN);

    // ADS_Prog_Write(AUTO_SEQ_EN,0x04);

    // ADS_Prog_Read(AUTO_SEQ_EN);

    // enter manual mode

    return 0;
}

#if 0
//只能传递像    MAN_2的宏定义
uint16_t read_ch(uint16_t ch)
{

    return ADS_Cmd_Write(ch);
}
#else

uint16_t read_ch(uint8_t flag)
{

    if (flag == 0)
    {
        return ADS_Cmd_Write(MAN_2);
    }
    else
    {
        return ADS_Cmd_Write(CONT);
    }
    //return ADS_Cmd_Write(AUTO_RST);
    
}

#endif

// TIM2中断：40kHz采样处理（尽量保持ISR最短，避免浮点运算）
void TIM2_IRQHandler(void)
{
    if (TIM2->SR & TIM_SR_UIF)
    {
        TIM2->SR &= ~TIM_SR_UIF; // 清除更新标志

        if ((ads8688_ctrl.total_sample_cnt % 10000) == 0)
        {
            if (ads8688_ctrl.total_sample_cnt > 0)
            {
                ads8688_ctrl.array_column++;
							
				if(ads8688_ctrl.array_column == ARRIY_COLUMN)
					ads8688_ctrl.array_column = 0;
									
            }

        }

        // uint16_t *p = (uint16_t*)&raw_data[ads8688_ctrl.total_sample_cnt*2];

        // *p = read_ch(ads8688_ctrl.total_sample_cnt);
        raw_data[ads8688_ctrl.array_column][ads8688_ctrl.total_sample_cnt % 10000] = read_ch(ads8688_ctrl.total_sample_cnt);
        ads8688_ctrl.total_sample_cnt++;

        if (ads8688_ctrl.total_sample_cnt == SAMPLE_TOTAL_CNT)
        {

            HAL_TIM_Base_Stop_IT(&htim2);
        }

        if ((ads8688_ctrl.total_sample_cnt % 2000) == 0)
        {
            ads8688_ctrl.current_cnt++;

            if (ads8688_ctrl.current_cnt == 1)
            {
                ads8688_ctrl.sample_flag = 0x11;
            }
            else if (ads8688_ctrl.current_cnt == 2)
            {
                ads8688_ctrl.sample_flag = 0x22;
            }
            else if (ads8688_ctrl.current_cnt == 3)
            {
                ads8688_ctrl.sample_flag = 0x33;
            }
            else if (ads8688_ctrl.current_cnt == 4)
            {
                ads8688_ctrl.sample_flag = 0x44;
            }
            else if (ads8688_ctrl.current_cnt == 5)
            {
                ads8688_ctrl.sample_flag = 0x55;
                ads8688_ctrl.current_cnt = 0;
            }
        }
    }
}


void usart_tx_task(void)
{
	#if 1
    if (ads8688_ctrl.sample_flag == 0x11)
    {
        
        if(Usart_DMA_Init_TX((uint8_t *)&raw_data[ads8688_ctrl.array_column][0], 4000))
					ads8688_ctrl.sample_flag = 0;
    }
    else if (ads8688_ctrl.sample_flag == 0x22)
    {
        
        if(Usart_DMA_Init_TX((uint8_t *)&raw_data[ads8688_ctrl.array_column][2000], 4000))
					ads8688_ctrl.sample_flag = 0;
    }
    else if (ads8688_ctrl.sample_flag == 0x33)
    {
        
        if(Usart_DMA_Init_TX((uint8_t *)&raw_data[ads8688_ctrl.array_column][4000], 4000))
					ads8688_ctrl.sample_flag = 0;
    }
    else if (ads8688_ctrl.sample_flag == 0x44)
    {
        
        if(Usart_DMA_Init_TX((uint8_t *)&raw_data[ads8688_ctrl.array_column][6000], 4000))
					ads8688_ctrl.sample_flag = 0;
    }
    else if (ads8688_ctrl.sample_flag == 0x55)
    {
        
        if(Usart_DMA_Init_TX((uint8_t *)&raw_data[ads8688_ctrl.array_column][8000], 4000))
					ads8688_ctrl.sample_flag = 0;
    }
		#else
		Usart_DMA_Init_TX((uint8_t *)&raw_data[ads8688_ctrl.array_column][0], 4000);
		HAL_Delay(1000);
		#endif
		
}


void ads8688_start_sample(void)
{

    memset(&ads8688_ctrl,0,sizeof(ADS8688_CTRL_t));

    for (uint8_t  i = 0; i < ARRIY_COLUMN; i++)
    {
        for (uint32_t j = 0; j < 10000; j++)
        {
            raw_data[i][j] = j;
        }
    }

    HAL_TIM_Base_Start_IT(&htim2);
}

void ads8688_stop_sample(void)
{
    HAL_TIM_Base_Stop_IT(&htim2);
}
