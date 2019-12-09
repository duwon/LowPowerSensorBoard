/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
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
#include "stm32l0xx_hal.h"
#include "stm32l0xx_hal.h"

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
#define OSC_IN_Pin GPIO_PIN_14
#define OSC_IN_GPIO_Port GPIOC
#define OSC_OUT_Pin GPIO_PIN_15
#define OSC_OUT_GPIO_Port GPIOC
#define ADC_BAT_Pin GPIO_PIN_0
#define ADC_BAT_GPIO_Port GPIOA
#define LD0_Pin GPIO_PIN_1
#define LD0_GPIO_Port GPIOA
#define SW_Pin GPIO_PIN_2
#define SW_GPIO_Port GPIOA
#define DIP0_Pin GPIO_PIN_3
#define DIP0_GPIO_Port GPIOA
#define DIP1_Pin GPIO_PIN_4
#define DIP1_GPIO_Port GPIOA
#define DIP2_Pin GPIO_PIN_5
#define DIP2_GPIO_Port GPIOA
#define DIP3_Pin GPIO_PIN_6
#define DIP3_GPIO_Port GPIOA
#define DIP4_Pin GPIO_PIN_7
#define DIP4_GPIO_Port GPIOA
#define DIP5_Pin GPIO_PIN_0
#define DIP5_GPIO_Port GPIOB
#define DIP6_Pin GPIO_PIN_1
#define DIP6_GPIO_Port GPIOB
#define SENSOR0_Pin GPIO_PIN_8
#define SENSOR0_GPIO_Port GPIOA
#define DEBUG_TX_Pin GPIO_PIN_9
#define DEBUG_TX_GPIO_Port GPIOA
#define DEBUG_RX_Pin GPIO_PIN_10
#define DEBUG_RX_GPIO_Port GPIOA
#define SENSOR1_Pin GPIO_PIN_11
#define SENSOR1_GPIO_Port GPIOA
#define SENSOR2_Pin GPIO_PIN_12
#define SENSOR2_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define COM_NRST_Pin GPIO_PIN_15
#define COM_NRST_GPIO_Port GPIOA
#define COM_RDY_Pin GPIO_PIN_3
#define COM_RDY_GPIO_Port GPIOB
#define CONTROL_DC12V_Pin GPIO_PIN_4
#define CONTROL_DC12V_GPIO_Port GPIOB
#define CONTROL_DC3V_Pin GPIO_PIN_5
#define CONTROL_DC3V_GPIO_Port GPIOB
#define COM_TX_Pin GPIO_PIN_6
#define COM_TX_GPIO_Port GPIOB
#define COM_RX_Pin GPIO_PIN_7
#define COM_RX_GPIO_Port GPIOB
#define BOOT_Pin GPIO_PIN_9
#define BOOT_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
