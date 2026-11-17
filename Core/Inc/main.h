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
#include "stm32f1xx_hal.h"

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
#define LED1_Pin GPIO_PIN_1
#define LED1_GPIO_Port GPIOA
#define LED2_Pin GPIO_PIN_2
#define LED2_GPIO_Port GPIOA
#define LED3_Pin GPIO_PIN_3
#define LED3_GPIO_Port GPIOA
#define B1_Pin GPIO_PIN_5
#define B1_GPIO_Port GPIOA
#define B1_EXTI_IRQn EXTI9_5_IRQn
#define LD2_Pin GPIO_PIN_13
#define LD2_GPIO_Port GPIOC

/* USER CODE BEGIN Private defines */

/* Respaldo: si en CubeMX faltan las "User Label" de los pines, CubeMX borra
 * los defines de arriba al regenerar. Este bloque (USER CODE) sobrevive y
 * mantiene compilando el proyecto. Si los labels existen, no tiene efecto.
 *
 * Mapa de pines (Blue Pill):
 *   PA1  LED1 | PA2 LED2 | PA3 LED3   (activos en alto)
 *   PA5  B1   pulsador a GND, pull-up, EXTI5 (flancos de subida y bajada)
 *   PA9  USART1_TX -> RX del USB-TTL | PA10 USART1_RX <- TX del USB-TTL
 *   PC13 LED onboard (activo en bajo)
 */
#ifndef LED1_Pin
#define LED1_Pin GPIO_PIN_1
#define LED1_GPIO_Port GPIOA
#endif
#ifndef LED2_Pin
#define LED2_Pin GPIO_PIN_2
#define LED2_GPIO_Port GPIOA
#endif
#ifndef LED3_Pin
#define LED3_Pin GPIO_PIN_3
#define LED3_GPIO_Port GPIOA
#endif
#ifndef B1_Pin
#define B1_Pin GPIO_PIN_5
#define B1_GPIO_Port GPIOA
#define B1_EXTI_IRQn EXTI9_5_IRQn
#endif

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
