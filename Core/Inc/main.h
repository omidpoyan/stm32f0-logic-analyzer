/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

#define BUFFER_ZIZE 2000

typedef enum{
  STATE_STOP=0,
  STATE_START,
  STATE_ERROR
} my_state_e;

typedef enum{
  MODE_LOGIC_ANALYZER=0,
  MODE_OSILOSCOP
} my_mode_e;

typedef enum{
  SWITCH_OFF=0,
  SWITCH_ON
}my_switch_e;

typedef struct
{
  my_state_e state;
  my_mode_e mode;
  uint16_t cc;
  uint8_t dataBuffer[BUFFER_ZIZE];
  __IO int Index;
}my_config_s;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED1_Pin GPIO_PIN_0
#define LED1_GPIO_Port GPIOF
#define LED2_Pin GPIO_PIN_1
#define LED2_GPIO_Port GPIOF
/* USER CODE BEGIN Private defines */

#define COMMAIND_START              's'
#define COMMAIND_STOP               'q'
#define COMMAIND_GET_TIME           't'
#define COMMAIND_GET_MODE           'm'
#define COMMAIND_ENABLE_TEST_PULSE  'g'
#define COMMAIND_DISABLE_TEST_PULSE 'h'

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
