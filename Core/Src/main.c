/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32f0xx_hal_adc.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
extern ADC_HandleTypeDef hadc;
extern DMA_HandleTypeDef hdma_adc;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DEBUG
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
my_config_s config={.Index=0,.cc=100,.adcSamplingTime=0,.mode=MODE_LOGIC_ANALYZER,.state=STATE_STOP};

void ControlLED(my_switch_e STATE)
{
  switch (STATE)
  {
  case SWITCH_ON:
    HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_SET);
    break;
  case SWITCH_OFF:
    HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_RESET);
    break;
  }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  ControlLED(SWITCH_OFF);
  HAL_UART_Transmit_DMA(&huart1,config.dataBuffer,BUFFER_ZIZE);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  ControlLED(SWITCH_ON);

  if(config.mode==MODE_LOGIC_ANALYZER)
  {
    config.Index=0;
    HAL_TIM_Base_Start_IT(&htim17);
  }
  else if(config.mode==MODE_OSILOSCOP && config.state==STATE_START)
  {
    HAL_ADC_Start_DMA(&hadc,config.dataBuffer,BUFFER_ZIZE);
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if(htim->Instance == TIM17)
  {
    if(config.Index<BUFFER_ZIZE)
    {
      config.dataBuffer[config.Index] = GPIOA->IDR;
      config.Index++;
    }
    else
    {
      HAL_TIM_Base_Stop_IT(&htim17);
      ControlLED(SWITCH_OFF);
      HAL_UART_Transmit_DMA(&huart1,config.dataBuffer,BUFFER_ZIZE);
    }
  }
  else
  {
    HAL_GPIO_TogglePin(TestSignal_GPIO_Port,TestSignal_Pin);
  }
}

void Starting(void)
{
  ControlLED(SWITCH_ON);
  HAL_Delay(200);
  ControlLED(SWITCH_OFF);
  #ifdef DEBUG
  HAL_UART_Transmit(&huart1,(uint8_t*)"Starting\r\n",10,100);
  #endif
}

void ControlLogicSampling(my_switch_e ON_OFF)
{
  if(ON_OFF==SWITCH_ON)
  {
    config.Index=0;
    config.state=STATE_START;
    HAL_TIM_Base_Start_IT(&htim17);
  }
  else
  {
    HAL_TIM_Base_Stop_IT(&htim17);
    HAL_UART_DMAStop(&huart1);
    __HAL_UART_CLEAR_IT(&huart1,UART_CLEAR_TCF);
    ControlLED(SWITCH_OFF);
    config.state=STATE_STOP;
  }
}

void ControlLogicTestPulse(my_switch_e ON_OFF)
{
  if(ON_OFF)
  {
    //HAL_TIM_OC_Start(&htim3,TIM_CHANNEL_4);
    HAL_TIM_Base_Start_IT(&htim3);
  }
  else
  {
    //HAL_TIM_OC_Stop(&htim3,TIM_CHANNEL_4);
    HAL_TIM_Base_Stop_IT(&htim3);
    HAL_GPIO_WritePin(TestSignal_GPIO_Port,TestSignal_Pin,GPIO_PIN_RESET);
  }
}

void ControlADCSampling(my_switch_e ON_OFF)
{
  if(ON_OFF)
  {
    HAL_ADC_Start_DMA(&hadc,config.dataBuffer,BUFFER_ZIZE);
    config.state=STATE_START;
  }
  else
  {
    HAL_ADC_Stop_DMA(&hadc);
    HAL_UART_DMAStop(&huart1);
    __HAL_UART_CLEAR_IT(&huart1,UART_CLEAR_TCF);
    ControlLED(SWITCH_OFF);
    config.state=STATE_STOP;
  }
}

void UpdateADCSampleingTime(uint8_t ST)
{
  ADC_ChannelConfTypeDef sConfig = {0};
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;

  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;

  switch(ST)
  {
    case 0:sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;break;
    case 1:sConfig.SamplingTime = ADC_SAMPLETIME_7CYCLES_5;break;
    case 2:sConfig.SamplingTime = ADC_SAMPLETIME_13CYCLES_5;break;
    case 3:sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES_5;break;
    case 4:sConfig.SamplingTime = ADC_SAMPLETIME_41CYCLES_5;break;
    case 5:sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;break;
    case 6:sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;break;
    case 7:sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;break;
  }

  if(config.state==STATE_START)
  {
    ControlADCSampling(SWITCH_OFF);
    if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
    {
      Error_Handler();
    }
    ControlADCSampling(SWITCH_ON);
  }
  else
  {
    if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
    {
      Error_Handler();
    }
  }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  uint8_t textBuffer[8],C;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
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
  MX_USART1_UART_Init();
  MX_TIM17_Init();
  MX_ADC_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

  Starting();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if(HAL_UART_Receive(&huart1,&C,1,0xffff)==HAL_OK)
    {
      if(C==COMMAND_GET_TIME)
      {
        if(HAL_UART_Receive(&huart1,textBuffer,5,1000)==HAL_OK)
        {
          textBuffer[6]='\0';
          config.cc=atoi((char*)textBuffer);
          htim17.Instance->ARR=config.cc-1;
        }
      }
      else if(C==COMMAND_GET_MODE)
      {
        if(HAL_UART_Receive(&huart1,&C,1,1000)==HAL_OK)
        {
          if(C=='0')config.mode=MODE_LOGIC_ANALYZER;
          else if(C=='1')config.mode=MODE_OSILOSCOP;
        }
      }
      else if(C==COMMAND_GET_ADC_STIME)
      {
        if(HAL_UART_Receive(&huart1,&C,1,1000)==HAL_OK)
        {
          if(C>='0' && C <='9')
          {
            config.adcSamplingTime = C-'0';
            UpdateADCSampleingTime(config.adcSamplingTime);
          }
        }
      }
      else if(C==COMMAND_START)
      {
        if(config.mode==MODE_LOGIC_ANALYZER)
        {
          ControlLogicSampling(SWITCH_ON);
        }
        else if(config.mode==MODE_OSILOSCOP)
        {
          ControlADCSampling(SWITCH_ON);
        }
      }
      else if(C==COMMAND_STOP)
      {
        ControlLogicSampling(SWITCH_OFF);
        ControlADCSampling(SWITCH_OFF);
        ControlLogicTestPulse(SWITCH_OFF);
      }
      else if(C==COMMAND_ENABLE_TEST_PULSE)
      {
        ControlLogicTestPulse(SWITCH_ON);
      }
      else if(C==COMMAND_DISABLE_TEST_PULSE)
      {
        ControlLogicTestPulse(SWITCH_OFF);
      }
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI14;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSI14State = RCC_HSI14_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.HSI14CalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
