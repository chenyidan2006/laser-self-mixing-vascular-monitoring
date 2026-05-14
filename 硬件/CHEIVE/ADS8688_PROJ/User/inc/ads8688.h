

#ifndef __ADS8688_H__


#define __ADS8688_H__

#include "stm32f4xx.h"

#include <stdbool.h>

#define CONT 0x0000
#define STBY 0x8200
#define PWDN 0x8300
#define RST 0x8500
#define AUTO_RST 0xa000
#define MAN_0 0xc000
#define MAN_1 0xc400
#define MAN_2 0xc800
#define MAN_3 0xcc00
#define MAN_4 0xd000
#define MAN_5 0xd400
#define MAN_6 0xd800
#define MAN_7 0xdc00

// 自动扫描序列寄存器
// 0 = Channel n is not selected for sequencing in AUTO_RST mode
// 1 = Channel n is selected for sequencing in AUTO_RST mode
#define AUTO_SEQ_EN 0x01
// 通道断电寄存器
// 0 = The analog front-end on channel 7 is powered up and channel 7 can be
//  included in the AUTO_RST sequence
// 1 = The analog front-end on channel 7 is powered down and channel 7
// cannot be included in the AUTO_RST sequence
#define CHN_PWRDN 0x02
// 设定菊花链ID,和SDO数据格式SDO[2:0]
#define FEATURE_SELECT 0x03

// 设置通道的采样范围

// 3-0 Range_CHn[3:0] R/W 0h
// 0000 = Input range is set to ±2.5 x VREF
// 0001 = Input range is set to ±1.25 x VREF
// 0010 = Input range is set to ±0.625 x VREF
// 0101 = Input range is set to 0 to 2.5 x VREF
// 0110 = Input range is set to 0 to 1.25 x VREF
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


//修改这里修改缓冲区大小，40000表示采样率为40K，采样率不宜过大，会导致内存不够
//修改完这里以后，一定要修改tim.c里面定时器的中断频率
//定时器的中断频率等于84000000/84/Period
#define SAMPLE_RATE 40000

//这个表示采样多少秒
#define SAMPLE_SECONDS 8
//总采样点数等于采样时间乘以1s的采样点数
#define SAMPLE_TOTAL_CNT (SAMPLE_SECONDS * 40000)
//#define SAMPLE_TOTAL_CNT (10000)

//这个是缓冲区的列大小，跟采样率有关系
#define ARRIY_COLUMN (SAMPLE_RATE / 10000)

typedef struct
{
    uint32_t total_sample_cnt;
    uint16_t array_column;
    uint8_t sample_flag;
    uint8_t current_cnt;

} ADS8688_CTRL_t;

extern ADS8688_CTRL_t ads8688_ctrl;
extern uint16_t raw_data[ARRIY_COLUMN][10000];

//extern uint8_t raw_data[40000];

extern uint8_t ADS8688_Init(void);
extern void ads8688_start_sample(void);
extern void ads8688_stop_sample(void);
extern void usart_tx_task(void);

#endif
