/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ads8688.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#include <math.h>

#define SAMPLING_RATE (1000)
#define M_PI 3.14159265358979323846
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

// 生成原始正弦波信号：x(n) = amp * sin(2πfn/fs)

// 核心参数定义（重点：振幅设为0.01）
//#define SAMPLE_RATE    1000    // 采样率：1000Hz
//#define SIGNAL_FREQ    50      // 正弦波频率：50Hz
//#define AMPLITUDE      0.01    // 正弦波振幅，确保范围[-0.01, +0.01]
//#define MAP_MAX_VAL    2.56    // 映射基准上限：+2.56
//#define MAP_MIN_VAL    -2.56   // 映射基准下限：-2.56
//#define DURATION       1.0     // 生成时长：1秒

//// 映射基准的范围跨度 = 2.56 - (-2.56) = 5.12
//#define MAP_RANGE_SPAN (MAP_MAX_VAL - MAP_MIN_VAL)

//float signal_sin[2048];

//void xxx(void)
//{

// for (uint32_t n = 0; n < 2048; n++) 
// {
//       // 计算相位：标准正弦波相位公式
//       float phase = 2 * M_PI * SIGNAL_FREQ * n / SAMPLE_RATE;
//       // 生成正弦波，振幅0.01，范围严格控制在[-0.01, +0.01]
//        signal_sin[n] = (AMPLITUDE * sin(phase)) + 0.01f;

//       // 核心：按[-2.56,+2.56]→[0,65535]映射（公式不变）
//       double mapped_float = (x - MAP_MIN_VAL) / MAP_RANGE_SPAN * 65535.0;
//       // 四舍五入（关键：避免小振幅下的截断误差导致波形畸变）
//				uint32_t	t = (uint32_t)round(mapped_float);

//       // 边界保护：防止浮点精度误差导致越界
//        if (t > 65535) 
//					 signal_sin[n] = 65535;
//					else
//						signal_sin[n] = t;
//					
////					if(t < 0)
////							signal_sin[n] = 0;
//					
//   }


//}

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//uint8_t tx_buffer[10240];
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* 初始化代码（时钟、GPIO、USART等） */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();


  ADS8688_Init();
	//xxx();
  HAL_Delay(1000);


  ads8688_start_sample();

//  for (uint16_t i = 0; i < 2048; i++)
//  {
//            uint8_t buff[100];
//						int len = sprintf(buff, "%.6f\r\n",signal_sin[i]);
//						if(len > 0)
//							Usart_DMA_Init_TX(buff,len);
//            else
//            {
//              while (1)
//              {
//                
//              }
//            }
//						HAL_Delay(1);
//  }


  /* 调用发送函数，间隔500ms发送一个数值 */
  //USART1_Send_1_to_100(500);
  //usart_tx_task();
  while (1)
  {
	// 		if(ads8688_ctrl.total_sample_cnt == 10000)
	// 		{
	// 			uint8_t *p = (uint8_t *)&raw_data[3];
				

				

	// 					for(uint32_t i =0;i<10000;i++)
	// 				{
	// 					uint8_t buff[100];
						
	// 					if((i%2)!= 0)
	// 						continue;
						
	// 					uint32_t q = (p[i] << 8)|p[i+1];
						
	// 					float x=(q/65535.0f)*5.12f-2.56f;
						
	// 					int len = sprintf(buff, "%.6f\r\n",x);
	// 					if(len > 0)
	// 						Usart_DMA_Init_TX(buff,len);
	// 					else
	// 					{
	// 							while(1);
	// 					}
							
	// 						HAL_Delay(1);
	// 				}


	
	// 		ads8688_ctrl.total_sample_cnt = 0;
     usart_tx_task();

  //   // 主循环（发送函数已经是死循环，这里可以留空）
  // }
}
}
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
