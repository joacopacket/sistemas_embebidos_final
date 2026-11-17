#ifndef PULSADOR_H
#define PULSADOR_H

#include <stdint.h>

/* Inicialización del módulo */
void Pulsador_Init(void);

/* Proceso periódico de detección y antirrebote */
void Pulsador_Proceso(void);

/* Devuelve el contador de operaciones */
uint16_t Pulsador_ObtenerContador(void);

#endif
