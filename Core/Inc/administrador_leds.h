
#ifndef ADMINISTRADOR_LEDS_H
#define ADMINISTRADOR_LEDS_H

#include <stdint.h>

/* Inicialización de los tres LED */
void AdministradorLEDs_Init(void);

/* Configura el parpadeo de un LED individual */
uint8_t AdministradorLEDs_ConfigurarParpadeo(uint8_t numeroLED,
                                             uint16_t intervalo_ms);

/* Inicia la secuencia LED1 -> LED2 -> LED3 -> OFF */
void AdministradorLEDs_IniciarSecuencia(void);

/* Apaga los tres LED y detiene todos los procesos */
void AdministradorLEDs_ApagarTodos(void);

/* Proceso periódico de actualización de los LED */
void AdministradorLEDs_Proceso(void);

#endif
