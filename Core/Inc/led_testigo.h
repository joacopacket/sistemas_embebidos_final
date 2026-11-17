
#ifndef LED_TESTIGO_H
#define LED_TESTIGO_H

#include "stm32f1xx_hal.h"

/* Estructura que representa un LED */
typedef struct
{
    GPIO_TypeDef *Puerto;
    uint16_t Pin;

} LedTestigo;

/* Funciones de control */
void LedTestigo_Init(LedTestigo *led);
void LedTestigo_Encender(LedTestigo *led);
void LedTestigo_Apagar(LedTestigo *led);
void LedTestigo_CambiarEstado(LedTestigo *led);

#endif
