/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "I2C_LCD_PCF8574.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SAMPLE_COUNT 500
#define SENSOR_GAIN     1			// NEED TO ADD GAIN FOR VOLTAGE
#define PF_TARGET       0.95f
#define PF_HYST         0.02f
#define PF_FILTER_SIZE  10
#define Q_MIN           2.0f
#define MIN_V           0.05f
#define MIN_I           0.01f
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
volatile uint32_t adc_dma_buffer[2 * SAMPLE_COUNT];
volatile uint8_t buffer_full = 0;
static uint8_t relay_state = 0;
char msg[50];
char lcd_string[17];
float pf = 0;
float pf_corrected = 0;
float pf_buffer[PF_FILTER_SIZE] = {0};
uint8_t pf_index = 0;
uint8_t pf_filled = 0;
uint8_t freq = 60;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
void UART_PutString(char * uart_str);
void CapBankSwitchLogic(float Qc);
void Relay_On(uint8_t relay);
void Relay_Off(uint8_t relay);
float PF_Filter(float pf_corrected);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

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
  MX_ADC1_Init();
  MX_TIM2_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  HAL_Delay(5);
  LCD_Start();
  HAL_Delay(5);
  HAL_TIM_Base_Start_IT(&htim2);
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_dma_buffer, 2*SAMPLE_COUNT);

  HAL_StatusTypeDef ret;

  ret = HAL_I2C_IsDeviceReady(&hi2c1, 0x4E, 3, 100);
  if(ret == HAL_OK)
      UART_PutString("LCD FOUND\r\n");
  else
      UART_PutString("LCD NOT FOUND\r\n");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      if(buffer_full)
      {
          buffer_full = 0;
          float v_mean = 0, i_mean = 0;
          float v_rms = 0, i_rms = 0;
          float v, c, v_real, i_real;
          float real_power = 0, apparent_power = 0;
          float volts_per_count = 3.3f / 4095.0f;

          // MEAN CALCULATION (To remove DC offset / center at zero)
          for(int i = 0; i < SAMPLE_COUNT; i++)
          {
              v_mean += adc_dma_buffer[2*i];
              i_mean += adc_dma_buffer[2*i + 1];
          }
          v_mean /= SAMPLE_COUNT;
          i_mean /= SAMPLE_COUNT;

          // RMS + REAL POWER + APPARENT POWER
          for(int i = 0; i < SAMPLE_COUNT; i++)
          {
              v = adc_dma_buffer[2*i] - v_mean;
              c = adc_dma_buffer[2*i + 1] - i_mean;

              v_real = v * volts_per_count;
              i_real = (c * volts_per_count) / SENSOR_GAIN;		// PRIORITY // CT: Is = (Ip / 1000) // Vs = Is * (50 0hm) // Pot gain is 20, 50/1000 = .05*20 = 1
              	  	  	  	  	  	  	  	  	  	  	  	  	// SENSOR GAIN SHOULD BE 1
              v_rms += (v_real * v_real);
              i_rms += (i_real * i_real);

              real_power += (v_real * i_real);
          }
          v_rms = sqrtf(v_rms / SAMPLE_COUNT);
          i_rms = sqrtf(i_rms / SAMPLE_COUNT);

          // --------------------------------------------------------------------------------
          static uint32_t last_switch_time = 0;

    	  // Emergency_Shutoff
          int Emergency;
          if (HAL_GPIO_ReadPin(Emergency_Shutoff_GPIO_Port, Emergency_Shutoff_Pin) == GPIO_PIN_SET)
          {Emergency = 1;}

          // Over/Under Voltage Protection (Relay 8)
          if (v_rms < 108.0f || v_rms > 132.0f || Emergency)
          {
              Relay_Off(8); // Shut off main
              CapBankSwitchLogic(0);
              last_switch_time = HAL_GetTick();
              UART_PutString("VOLTAGE FAULT \r\n");
          }
          else if (HAL_GetTick() - last_switch_time > 3000)
          {Relay_On(8);}

          // Load 1 / Load 2
          if (HAL_GPIO_ReadPin(Load1_GPIO_Port, Load1_Pin) == GPIO_PIN_SET)
          {Relay_On(6);}
          else if (HAL_GetTick() - last_switch_time > 3000)
          {Relay_Off(6);}
          if (HAL_GPIO_ReadPin(Load2_GPIO_Port, Load2_Pin) == GPIO_PIN_SET)
          {Relay_On(7);}
          else if (HAL_GetTick() - last_switch_time > 3000)
          {Relay_Off(7);}
          // ----------------------------------------------------------------------------------

          real_power /= SAMPLE_COUNT;
          apparent_power = (v_rms * i_rms);

          // SIGNAL VALIDATION + POWER FACTOR
          if(v_rms < MIN_V || i_rms < MIN_I)
          {
              Relay_Off(1);
              Relay_Off(2);
              Relay_Off(3);
              Relay_Off(4);
              Relay_Off(5);

              relay_state = 0;

              UART_PutString("Signal too small \r\n");
              HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_dma_buffer, 2*SAMPLE_COUNT);
              //continue; // Returns to top of loop											UNCOMMENT UNCOMMENT UNCOMMENT UNCOMMENT
          }
          else
          {
              pf = real_power / apparent_power;
              float angle = acosf(pf);					// in radians
              pf_corrected = cosf(angle + 0.349); // 20 degrees*(pi/180) = 0.349 radians // 20 degree offset from CT	// Check again after voltage module
          }

          // Clamp
          if(pf_corrected >= 1) pf_corrected = 0.999f;	 // Prevent acos/tan instability
          if(pf_corrected <= -1) pf_corrected = -0.999f; // Prevent acos/tan instability

          // Filter
          float pf_safe = PF_Filter(pf_corrected);

          // REACTIVE POWER
          float Qcurrent = real_power * tanf(acosf(pf_safe));
          float Qtarget  = real_power * tanf(acosf(PF_TARGET));
          float Qc = Qcurrent - Qtarget;
          float C = (Qc / (2.0f * M_PI * freq * (v_rms * v_rms))) * 1000000.0f;

          // DEAD BAND
          if(pf_safe > (PF_TARGET - PF_HYST) && pf_safe < (PF_TARGET + PF_HYST))
          {
              snprintf(msg, 50, "PF: %.2f (stable) \r\n", pf_safe);
              UART_PutString(msg);
              HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_dma_buffer, 2*SAMPLE_COUNT);
              //continue;																UNCOMMENT UNCOMMENT UNCOMMENT UNCOMMENT
          }

          // RELAY DELAY
          if(HAL_GetTick() - last_switch_time < 3000)
        	  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_dma_buffer, 2*SAMPLE_COUNT);
              //continue;																UNCOMMENT UNCOMMENT UNCOMMENT UNCOMMENT

          // CONTROL LOGIC
          if(pf_safe < (PF_TARGET - PF_HYST))
          {
              // Need more capacitance
              CapBankSwitchLogic(C);
              last_switch_time = HAL_GetTick();
          }
          else if(pf_safe > (PF_TARGET + PF_HYST))
          {
        	  CapBankSwitchLogic(0);
        	  last_switch_time = HAL_GetTick();
          }

          // Print values to terminal
          snprintf(msg, 50, "V: %.2f V, I: %.2f A, PF: %.3f \r\n", v_rms, i_rms, pf_safe);
          UART_PutString(msg);


          // LCD_PutChar(sel+0x30); or just the number you want: 0x3#
          // Print values to LCD
          LCD_ClearDisplay();
          snprintf(lcd_string, 17, "PF: %.2f", pf_safe);
          //LCD_Position(0, 0);
          LCD_PrintString(lcd_string);
          snprintf(lcd_string, 17, "V:%.1f I:%.1f", v_rms, i_rms);
          LCD_Position(1, 0);
          LCD_PrintString(lcd_string);

          HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_dma_buffer, 2*SAMPLE_COUNT);
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1|RCC_PERIPHCLK_ADC12;
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12PLLCLK_DIV1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T2_TRGO;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 2;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.SamplingTime = ADC_SAMPLETIME_19CYCLES_5;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00201D2B;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 79;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 19;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, Relay1_Pin|Relay2_Pin|Relay3_Pin|Relay4_Pin
                          |LED_Pin|Relay7_Pin|Relay8_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, Relay5_Pin|Relay6_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : Relay1_Pin Relay2_Pin Relay3_Pin Relay4_Pin
                           LED_Pin Relay7_Pin Relay8_Pin */
  GPIO_InitStruct.Pin = Relay1_Pin|Relay2_Pin|Relay3_Pin|Relay4_Pin
                          |LED_Pin|Relay7_Pin|Relay8_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : Relay5_Pin Relay6_Pin */
  GPIO_InitStruct.Pin = Relay5_Pin|Relay6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : Load2_Pin Load1_Pin */
  GPIO_InitStruct.Pin = Load2_Pin|Load1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : Emergency_Shutoff_Pin */
  GPIO_InitStruct.Pin = Emergency_Shutoff_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Emergency_Shutoff_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void UART_PutString(char * uart_str)
{
	HAL_UART_Transmit(&huart2, (uint8_t *)uart_str, strnlen(uart_str, 80), -1);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if(hadc->Instance == ADC1)
    {
    	HAL_ADC_Stop_DMA(&hadc1);
        buffer_full = 1;
    }
}

void Relay_On(uint8_t relay)
{
    switch(relay)
    {
        case 1:
            HAL_GPIO_WritePin(GPIOA, Relay1_Pin, GPIO_PIN_SET);
            snprintf(msg, 50, "Relay 1 ON \r\n");
            break;

        case 2:
            HAL_GPIO_WritePin(GPIOA, Relay2_Pin, GPIO_PIN_SET);
            snprintf(msg, 50, "Relay 2 ON \r\n");
            break;

        case 3:
            HAL_GPIO_WritePin(GPIOA, Relay3_Pin, GPIO_PIN_SET);
            snprintf(msg, 50, "Relay 3 ON \r\n");
            break;

        case 4:
            HAL_GPIO_WritePin(GPIOA, Relay4_Pin, GPIO_PIN_SET);
            snprintf(msg, 50, "Relay 4 ON \r\n");
            break;

        case 5:
            HAL_GPIO_WritePin(GPIOB, Relay5_Pin, GPIO_PIN_SET);
            snprintf(msg, 50, "Relay 5 ON \r\n");
            break;

        case 6:
            HAL_GPIO_WritePin(GPIOB, Relay6_Pin, GPIO_PIN_SET);
            snprintf(msg, 50, "Relay 6 ON \r\n");
            break;
        case 7:
            HAL_GPIO_WritePin(GPIOA, Relay7_Pin, GPIO_PIN_SET);
            snprintf(msg, 50, "Relay 7 ON \r\n");
            break;
        case 8:
            HAL_GPIO_WritePin(GPIOA, Relay8_Pin, GPIO_PIN_SET);
            snprintf(msg, 50, "Relay 8 ON \r\n");
            break;
    }
    UART_PutString(msg);
}

void Relay_Off(uint8_t relay)
{
    switch(relay)
    {
        case 1:
            HAL_GPIO_WritePin(GPIOA, Relay1_Pin, GPIO_PIN_RESET);
            snprintf(msg, 50, "Relay 1 OFF \r\n");
            break;

        case 2:
            HAL_GPIO_WritePin(GPIOA, Relay2_Pin, GPIO_PIN_RESET);
            snprintf(msg, 50, "Relay 2 OFF \r\n");
            break;

        case 3:
            HAL_GPIO_WritePin(GPIOA, Relay3_Pin, GPIO_PIN_RESET);
            snprintf(msg, 50, "Relay 3 OFF \r\n");
            break;

        case 4:
            HAL_GPIO_WritePin(GPIOA, Relay4_Pin, GPIO_PIN_RESET);
            snprintf(msg, 50, "Relay 4 OFF \r\n");
            break;

        case 5:
            HAL_GPIO_WritePin(GPIOB, Relay5_Pin, GPIO_PIN_RESET);
            snprintf(msg, 50, "Relay 5 OFF \r\n");
            break;

        case 6:
            HAL_GPIO_WritePin(GPIOB, Relay6_Pin, GPIO_PIN_RESET);
            snprintf(msg, 50, "Relay 6 OFF \r\n");
            break;
        case 7:
            HAL_GPIO_WritePin(GPIOA, Relay7_Pin, GPIO_PIN_RESET);
            snprintf(msg, 50, "Relay 7 OFF \r\n");
            break;
        case 8:
            HAL_GPIO_WritePin(GPIOA, Relay8_Pin, GPIO_PIN_RESET);
            snprintf(msg, 50, "Relay 8 OFF \r\n");
            break;
    }
    UART_PutString(msg);
}

void CapBankSwitchLogic(float C)
{
    uint8_t new_state = 0;
    		// Relay: 1		2	  3		4	   5
    float caps[5] = {1.0f, 2.0f, 5.0f, 10.0f, 20.0f};

    // Start from largest to smallest
    for(int i = 4; i >= 0; i--)
    {
        if(C >= caps[i])
        {
            new_state |= (1 << i);	// MSB is relay (5) with largest reactive power
            C -= caps[i];
        }
    }

    // Apply only changes
    for(int i = 0; i < 5; i++)
    {
        uint8_t mask = (1 << i);

        if((new_state & mask) && !(relay_state & mask))
            Relay_On(i+1);

        else if(!(new_state & mask) && (relay_state & mask))
            Relay_Off(i+1);
    }

    relay_state = new_state;

    snprintf(msg, 50, "C: %.1f State: %d \r\n", C, relay_state);					//STATE NEEDS CHANGED FROM 8 BITS (DISPAYING AS DECIMAL EX. 31) TO ACTUAL BINARY
    UART_PutString(msg);
}

float PF_Filter(float pf_corrected)
{
    pf_buffer[pf_index++] = pf_corrected;

    if(pf_index >= PF_FILTER_SIZE)
    {
        pf_index = 0;
        pf_filled = 1;
    }

    float sum = 0;
    float count = pf_filled ? PF_FILTER_SIZE : pf_index;

    if (count == 0) return pf_corrected;

    for(int i = 0; i < count; i++)
        sum += pf_buffer[i];

    return sum / count;
}
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
