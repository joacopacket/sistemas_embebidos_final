
#ifndef CONTROL_UART_H
#define CONTROL_UART_H

#include <stdint.h>

/* Inicialización del módulo UART */
void ControlUART_Init(void);

/* Procesamiento de los datos recibidos */
void ControlUART_Proceso(void);

/* Transmisión del contador de operaciones */
void ControlUART_EnviarContador(uint16_t contador);

#endif
