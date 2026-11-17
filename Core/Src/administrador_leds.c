
#include "main.h"
#include "led_testigo.h"
#include "administrador_leds.h"

/* Cantidad de LED */
#define CANTIDAD_LEDS          3

/* Intervalo de cada paso de la secuencia */
#define INTERVALO_SECUENCIA    500

/* Límites permitidos para el parpadeo individual */
#define INTERVALO_MINIMO       100
#define INTERVALO_MAXIMO       1000

/* Estado individual de cada LED */
typedef struct
{
    uint8_t Activo;
    uint8_t Encendido;
    uint16_t Intervalo;
    uint32_t UltimoTick;

} EstadoParpadeo;

/* Definición de los tres LED físicos */
static LedTestigo leds[CANTIDAD_LEDS] =
{
    {LED1_GPIO_Port, LED1_Pin},
    {LED2_GPIO_Port, LED2_Pin},
    {LED3_GPIO_Port, LED3_Pin}
};

/* Estados de parpadeo individual */
static EstadoParpadeo parpadeos[CANTIDAD_LEDS];

/* Variables de la secuencia */
static uint8_t SecuenciaActiva = 0;
static uint8_t PasoSecuencia = 0;
static uint32_t UltimoTickSecuencia = 0;


/**
 * @brief Inicializa los tres LED y sus estados.
 */
void AdministradorLEDs_Init(void)
{
    uint8_t i;

    SecuenciaActiva = 0;
    PasoSecuencia = 0;

    for (i = 0; i < CANTIDAD_LEDS; i++)
    {
        LedTestigo_Init(&leds[i]);

        parpadeos[i].Activo = 0;
        parpadeos[i].Encendido = 0;
        parpadeos[i].Intervalo = 500;
        parpadeos[i].UltimoTick = HAL_GetTick();
    }
}


/**
 * @brief Configura el parpadeo independiente de un LED.
 *
 * @param numeroLED Número de LED (1 a 3).
 * @param intervalo_ms Intervalo de parpadeo en milisegundos.
 *
 * @return 1 si la configuración es válida, 0 si no lo es.
 */
uint8_t AdministradorLEDs_ConfigurarParpadeo(uint8_t numeroLED,
                                             uint16_t intervalo_ms)
{
    uint8_t indice;

    /* Verificar número de LED */
    if ((numeroLED < 1) || (numeroLED > CANTIDAD_LEDS))
    {
        return 0;
    }

    /* Verificar intervalo permitido */
    if ((intervalo_ms < INTERVALO_MINIMO) ||
        (intervalo_ms > INTERVALO_MAXIMO))
    {
        return 0;
    }

    /* Si estaba activa la secuencia, detenerla */
    if (SecuenciaActiva)
    {
        AdministradorLEDs_ApagarTodos();
    }

    indice = numeroLED - 1;

    parpadeos[indice].Intervalo = intervalo_ms;
    parpadeos[indice].Activo = 1;
    parpadeos[indice].Encendido = 1;
    parpadeos[indice].UltimoTick = HAL_GetTick();

    LedTestigo_Encender(&leds[indice]);

    return 1;
}


/**
 * @brief Inicia la secuencia cíclica de los tres LED.
 */
void AdministradorLEDs_IniciarSecuencia(void)
{
    uint8_t i;

    /* Detener los parpadeos individuales */
    for (i = 0; i < CANTIDAD_LEDS; i++)
    {
        parpadeos[i].Activo = 0;
        parpadeos[i].Encendido = 0;

        LedTestigo_Apagar(&leds[i]);
    }

    /* Iniciar secuencia desde LED1 */
    SecuenciaActiva = 1;
    PasoSecuencia = 0;
    UltimoTickSecuencia = HAL_GetTick();

    LedTestigo_Encender(&leds[0]);
}


/**
 * @brief Apaga todos los LED y detiene sus procesos.
 */
void AdministradorLEDs_ApagarTodos(void)
{
    uint8_t i;

    SecuenciaActiva = 0;
    PasoSecuencia = 0;

    for (i = 0; i < CANTIDAD_LEDS; i++)
    {
        parpadeos[i].Activo = 0;
        parpadeos[i].Encendido = 0;

        LedTestigo_Apagar(&leds[i]);
    }
}


/**
 * @brief Actualiza los estados de los LED.
 *
 * Debe ejecutarse periódicamente desde el bucle principal.
 */
void AdministradorLEDs_Proceso(void)
{
    uint8_t i;
    uint32_t tiempoActual = HAL_GetTick();

    /* Si está activa la secuencia, tiene prioridad */
    if (SecuenciaActiva)
    {
        if ((tiempoActual - UltimoTickSecuencia) >=
            INTERVALO_SECUENCIA)
        {
            UltimoTickSecuencia = tiempoActual;

            /* Apagar el paso anterior */
            LedTestigo_Apagar(&leds[0]);
            LedTestigo_Apagar(&leds[1]);
            LedTestigo_Apagar(&leds[2]);

            /* Avanzar al siguiente paso */
            PasoSecuencia++;

            if (PasoSecuencia > 3)
            {
                PasoSecuencia = 0;
            }

            /* Encender LED correspondiente */
            if (PasoSecuencia < 3)
            {
                LedTestigo_Encender(&leds[PasoSecuencia]);
            }
            /* Paso 3: todos apagados */
        }

        return;
    }

    /* Procesar los parpadeos individuales */
    for (i = 0; i < CANTIDAD_LEDS; i++)
    {
        if (parpadeos[i].Activo)
        {
            if ((tiempoActual - parpadeos[i].UltimoTick) >=
                parpadeos[i].Intervalo)
            {
                parpadeos[i].UltimoTick = tiempoActual;

                parpadeos[i].Encendido =
                    !parpadeos[i].Encendido;

                if (parpadeos[i].Encendido)
                {
                    LedTestigo_Encender(&leds[i]);
                }
                else
                {
                    LedTestigo_Apagar(&leds[i]);
                }
            }
        }
    }
}
