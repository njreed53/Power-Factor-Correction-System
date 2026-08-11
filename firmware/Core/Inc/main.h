/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
#include "stm32f3xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

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
#define MCO_Pin GPIO_PIN_0
#define MCO_GPIO_Port GPIOF
#define Voltage_Pin GPIO_PIN_0
#define Voltage_GPIO_Port GPIOA
#define Current_Pin GPIO_PIN_1
#define Current_GPIO_Port GPIOA
#define Relay1_Pin GPIO_PIN_4
#define Relay1_GPIO_Port GPIOA
#define Relay2_Pin GPIO_PIN_5
#define Relay2_GPIO_Port GPIOA
#define Relay3_Pin GPIO_PIN_6
#define Relay3_GPIO_Port GPIOA
#define Relay4_Pin GPIO_PIN_7
#define Relay4_GPIO_Port GPIOA
#define Relay5_Pin GPIO_PIN_0
#define Relay5_GPIO_Port GPIOB
#define Relay6_Pin GPIO_PIN_1
#define Relay6_GPIO_Port GPIOB
#define LED_Pin GPIO_PIN_9
#define LED_GPIO_Port GPIOA
#define Relay7_Pin GPIO_PIN_11
#define Relay7_GPIO_Port GPIOA
#define Relay8_Pin GPIO_PIN_12
#define Relay8_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define Load2_Pin GPIO_PIN_3
#define Load2_GPIO_Port GPIOB
#define Load1_Pin GPIO_PIN_4
#define Load1_GPIO_Port GPIOB
#define Emergency_Shutoff_Pin GPIO_PIN_5
#define Emergency_Shutoff_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
