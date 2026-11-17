
#include "led_testigo.h"

/**
 * @brief Inicializa un LED y lo deja apagado.
 */
void LedTestigo_Init(LedTestigo *led)
{
    HAL_GPIO_WritePin(led->Puerto, led->Pin, GPIO_PIN_RESET);
}

/**
 * @brief Enciende el LED.
 */
void LedTestigo_Encender(LedTestigo *led)
{
    HAL_GPIO_WritePin(led->Puerto, led->Pin, GPIO_PIN_SET);
}

/**
 * @brief Apaga el LED.
 */
void LedTestigo_Apagar(LedTestigo *led)
{
    HAL_GPIO_WritePin(led->Puerto, led->Pin, GPIO_PIN_RESET);
}

/**
 * @brief Invierte el estado actual del LED.
 */
void LedTestigo_CambiarEstado(LedTestigo *led)
{
    HAL_GPIO_TogglePin(led->Puerto, led->Pin);
}
