

#include "main.h"

#include <stdbool.h>

#include "spi.h"
#include "tim.h"

#define ADC_DAISY_Pin GPIO_PIN_0
#define ADC_DAISY_GPIO_Port GPIOA
#define ADC_RST_Pin GPIO_PIN_1
#define ADC_RST_GPIO_Port GPIOA
#define ADC_SPI_CS_Pin GPIO_PIN_4
#define ADC_SPI_CS_GPIO_Port GPIOA


#define CONT 0x00
#define STBY 0x82
#define PWDN 0x83
#define RST 0x85
#define AUTO_RST 0xa0
#define MAN_0 0xc0
#define MAN_1 0xc4
#define MAN_2 0xc8
#define MAN_3 0xcc
#define MAN_4 0xd0
#define MAN_5 0xd4
#define MAN_6 0xd8
#define MAN_7 0xdc
#define AUTO_SEQ_EN 0x01
#define CHN_PWRDN 0x02
#define FEATURE_SELECT 0x03
#define CHN_0_RANGE 0x05
#define CHN_1_RANGE 0x06
#define CHN_2_RANGE 0x07
#define CHN_3_RANGE 0x08
#define CHN_4_RANGE 0x09
#define CHN_5_RANGE 0x0a
#define CHN_6_RANGE 0x0b
#define CHN_7_RANGE 0x0c
#define CMD_RD_BCK 0x3f
#define CHNS_NUM_READ 8



// 40kHz * 8s = 320000 样本。外部SRAM(512K x16 = 1MB)不足以同时存放原始(2B)与缩放(4B)数据，故仅存原始16位数据，输出时再计算缩放值。
#define SAMPLE_FREQ_HZ 40000
#define SAMPLE_DURATION_SEC 8
#define SAMPLE_COUNT (SAMPLE_FREQ_HZ * SAMPLE_DURATION_SEC) // 320000

extern uint16_t spi_transmit_receive8(SPI_TypeDef *spi,uint8_t *tx,uint8_t *rx,uint16_t size);
extern uint16_t spi_transmit_receive16(SPI_TypeDef *spi,uint16_t *tx,uint16_t *rx,uint16_t size);
// 前向声明：中断中需使用的函数原型，避免隐式声明导致类型不匹配
uint16_t ADS_Read_CH0_Raw(uint16_t *raw);
float ADS8688_ConvertToVoltage(uint16_t raw,uint8_t range);



//just test
uint16_t raw_data[4][10240];
uint32_t total_sample_cnt;
uint8_t current_cnt;
uint16_t sample_cnt;
bool acquisition_flag = false;

// TIM2中断：40kHz采样处理（尽量保持ISR最短，避免浮点运算）
void TIM2_IRQHandler(void)
{
    if (TIM2->SR & TIM_SR_UIF)
    {
        TIM2->SR &= ~TIM_SR_UIF; // 清除更新标志
        if(acquisition_flag == true)
        {
            uint16_t raw_ch0;
            if(ADS_Read_CH0_Raw(&raw_ch0) > 0)
            {
                total_sample_cnt++;
                // if(sample_cnt == 10240)
                // {
                //     sample_cnt = 0;
                //     current_cnt++;
                //     if (current_cnt == 4)
                //     {
                //         current_cnt = 0;
                //     }
                    
                // }
                if (total_sample_cnt == 1024)
                {
                    HAL_TIM_Base_Stop_IT(&htim2);
                }
                

                raw_data[current_cnt][sample_cnt++] = raw_ch0;
                

            }
        }
    }
}




uint16_t ADS_Prog_Read(uint8_t addr, uint8_t *data)
{
    uint16_t r;
    uint8_t tx[4]={0x00,(addr<<1)&0xFE,0,0};
    uint8_t rx[4];
    HAL_GPIO_WritePin(ADS8688_CS_GPIO_Port,ADS8688_CS_Pin,GPIO_PIN_RESET);
    r=spi_transmit_receive8(SPI1,tx,rx,4);
    HAL_GPIO_WritePin(ADS8688_CS_GPIO_Port,ADS8688_CS_Pin,GPIO_PIN_SET);
    data[0]=rx[2];
    data[1]=rx[3];
    return r;
}

uint16_t ADS_Prog_Write(uint8_t addr,uint8_t *data)
{
    uint16_t r;
    uint8_t tx[4]={data[0],(addr<<1)|1,0,0};
    uint8_t rx[4];
    HAL_GPIO_WritePin(ADS8688_CS_GPIO_Port,ADS8688_CS_Pin,GPIO_PIN_RESET);
    r=spi_transmit_receive8(SPI1,tx,rx,4);
    HAL_GPIO_WritePin(ADS8688_CS_GPIO_Port,ADS8688_CS_Pin,GPIO_PIN_SET);
    return r;
}

uint16_t ADS_Cmd_Write(uint8_t cmd,uint8_t *data)
{
    uint16_t r;
    uint8_t tx[4]={0x00,cmd,0,0};
    uint8_t rx[4];
    HAL_GPIO_WritePin(ADS8688_CS_GPIO_Port,ADS8688_CS_Pin,GPIO_PIN_RESET);
    r=spi_transmit_receive8(SPI1,tx,rx,4);
    HAL_GPIO_WritePin(ADS8688_CS_GPIO_Port,ADS8688_CS_Pin,GPIO_PIN_SET);
    data[0]=rx[2];
    data[1]=rx[3];
    return r;
}

uint16_t ADS_Read_All_Raw(uint16_t *data)
{
    uint16_t r;
    uint8_t tmp[2];
    for(int i=0;i<8;i++)
    {
        r=ADS_Cmd_Write(CONT,tmp);
        data[i]=(tmp[1]<<8)|tmp[0];
    }
    return r;
}

// 仅读取通道0，使用手动命令以降低一次采样的SPI传输时间
uint16_t ADS_Read_CH0_Raw(uint16_t *raw)
{
    uint16_t r;
    uint8_t tmp[2];
    r = ADS_Cmd_Write(MAN_0,tmp); // MAN_0 命令触发CH0转换并返回数据
    *raw = (tmp[1]<<8)|tmp[0];
    return r;
}

float ADS8688_ConvertToVoltage(uint16_t raw,uint8_t range)
{
    // 支持常用量程码：
    // 0x05 -> 0..10.24V (unipolar)
    // 0x06 -> 0..5.12V  (unipolar)
    // 0x02 -> -2.5..+2.5V (bipolar)  <-- 新增支持（若你需要不同的 bipolar 满量程，请告知）
    if(range==0x05) return (raw/65535.0f)*10.24f;
    if(range==0x06) return (raw/65535.0f)*5.12f;
    if(range==0x02)
    {
        // 使用无符号到双极线性映射：raw=0 -> -2.56V, raw=65535 -> +2.56V
        return (raw / 65535.0f) * 5.12f - 2.56f;
    }
    return 0;
}

uint8_t ADS8688_Init(void)
{

    uint8_t d[2]={0};

    HAL_GPIO_WritePin(ADS8688_DAISY_GPIO_Port,ADS8688_DAISY_Pin,GPIO_PIN_RESET);
//    HAL_GPIO_WritePin(ADS8688_RST_GPIO_Port,ADS8688_RST_Pin,GPIO_PIN_RESET);
//    HAL_Delay(5);
    HAL_GPIO_WritePin(ADS8688_RST_GPIO_Port,ADS8688_RST_Pin,GPIO_PIN_SET);
    HAL_Delay(5);

    ADS_Cmd_Write(RST,d);
    HAL_Delay(20);

    d[0]=0xFF;
    ADS_Prog_Write(AUTO_SEQ_EN,d);
    HAL_Delay(5);

    d[0]=0x03;
    ADS_Prog_Write(FEATURE_SELECT,d);
    HAL_Delay(5);

    // 将通道0量程改为 0x02（双极 ±2.5V），其余保留原先设置
    uint8_t r[8]={0x02,0x06,0x06,0x06,0x06,0x06,0x06,0x06};
    for(int i=0;i<8;i++)
    {
        d[0]=r[i];
        ADS_Prog_Write(CHN_0_RANGE+i,d);
        
    }

    ADS_Cmd_Write(AUTO_RST,d);
    return 0;
}







#if 0

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_SPI1_Init();
    MX_USART1_UART_Init();
    MX_FMC_SRAM_Init();

#if 0
    // 启动诊断：打印复位原因
    uint32_t csr = RCC->CSR; // 读取复位标志
    char reset_info[128];
    int rn = sprintf(reset_info,"START RESET_FLAGS=0x%08lX ", (unsigned long)csr);
    HAL_UART_Transmit(&huart1,(uint8_t*)reset_info,rn,50);
    if(csr & RCC_CSR_IWDGRSTF) HAL_UART_Transmit(&huart1,(uint8_t*)"IWDG ",5,20);
    if(csr & RCC_CSR_WWDGRSTF) HAL_UART_Transmit(&huart1,(uint8_t*)"WWDG ",5,20);
    if(csr & RCC_CSR_PORRSTF) HAL_UART_Transmit(&huart1,(uint8_t*)"POR/PDR ",8,20);
    if(csr & RCC_CSR_SFTRSTF) HAL_UART_Transmit(&huart1,(uint8_t*)"SOFT ",5,20);
    if(csr & RCC_CSR_BORRSTF) HAL_UART_Transmit(&huart1,(uint8_t*)"BOR ",4,20);
    if(csr & RCC_CSR_PINRSTF) HAL_UART_Transmit(&huart1,(uint8_t*)"PIN ",4,20);
    HAL_UART_Transmit(&huart1,(uint8_t*)"\r\n",2,10);
    __HAL_RCC_CLEAR_RESET_FLAGS();
#endif





    uint32_t tx_start_ms = HAL_GetTick(); // 发送开始时间(ms)
    for(uint32_t i=0;i<SAMPLE_COUNT;i++)
    {
        uint16_t raw = raw_store[i];
        float v = ADS8688_ConvertToVoltage(raw, g_ads.channel_range[0]);
        // 使用 roundf 做正确的四舍五入，避免手工 +/-0.5 导致截断偏差
        int32_t scaled_signed = (int32_t)roundf(v * 10000.0f);
        uint32_t abs_scaled = (uint32_t)(scaled_signed < 0 ? -scaled_signed : scaled_signed);
        uint32_t int_part = abs_scaled / 10000U;
        uint32_t frac_part = abs_scaled % 10000U;
        int n;
        if(scaled_signed < 0)
            n = sprintf(out,"-%lu.%04lu V\r\n", (unsigned long)int_part, (unsigned long)frac_part);
        else
            n = sprintf(out,"%lu.%04lu V\r\n", (unsigned long)int_part, (unsigned long)frac_part);
        HAL_UART_Transmit(&huart1,(uint8_t*)out,n,300);
    }
    uint32_t tx_end_ms = HAL_GetTick();
    uint32_t tx_elapsed_ms = tx_end_ms - tx_start_ms;
    int n_info = sprintf(out,"SRAM SAMPLE DONE\r\nTX_TIME_MS=%lu\r\n", (unsigned long)tx_elapsed_ms);
    HAL_UART_Transmit(&huart1,(uint8_t*)out,n_info,300);


} 
#endif
