/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SW1_Pin GPIO_PIN_1
#define SW1_GPIO_Port GPIOA
#define SW1_EXTI_IRQn EXTI1_IRQn
#define SW2_Pin GPIO_PIN_4
#define SW2_GPIO_Port GPIOA
#define LDR1_Pin GPIO_PIN_4
#define LDR1_GPIO_Port GPIOC
#define COIL_A_Pin GPIO_PIN_5
#define COIL_A_GPIO_Port GPIOC
#define SW3_Pin GPIO_PIN_0
#define SW3_GPIO_Port GPIOB
#define SW3_EXTI_IRQn EXTI0_IRQn
#define LDR2_Pin GPIO_PIN_1
#define LDR2_GPIO_Port GPIOB
#define COIL_D_Pin GPIO_PIN_2
#define COIL_D_GPIO_Port GPIOB
#define COIL_C_Pin GPIO_PIN_7
#define COIL_C_GPIO_Port GPIOC
#define COIL_B_Pin GPIO_PIN_12
#define COIL_B_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define RCLK_GPIO_Port GPIOB 	// Shift Register Latch
#define RCLK_Pin GPIO_PIN_7
#define SRCLK_GPIO_Port GPIOC 	// Shift Register Serial Clock
#define SRCLK_Pin GPIO_PIN_0
#define SER_GPIO_Port GPIOB 	// Shift Register Data In
#define SER_Pin GPIO_PIN_15
#define D1_GPIO_Port GPIOB		// LED 1
#define D1_Pin GPIO_PIN_10
#define D2_GPIO_Port GPIOB		// LED 2
#define D2_Pin GPIO_PIN_4
#define D3_GPIO_Port GPIOB		// LED 3
#define D3_Pin GPIO_PIN_5
#define D4_GPIO_Port GPIOB		// LED 4
#define D4_Pin GPIO_PIN_3
#define LD2_GPIO_Port GPIOA		// LED 1
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA		// LD 2
#define LD2_Pin GPIO_PIN_5
//#define LDR1_GPIO_Port GPIOC	// LDR R24
//#define LDR1_Pin GPIO_PIN_4
//#define LDR2_GPIO_Port GPIOB	// LDR R32
//#define LDR2_Pin GPIO_PIN_1


/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
