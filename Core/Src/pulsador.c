#include "main.h"
#include "pulsador.h"

/* Tiempo de antirrebote */
#define TIEMPO_ANTIRREBOTE  30

/* Variables internas del módulo */
static volatile uint8_t EventoPendiente = 0;
static volatile uint32_t UltimoCambioTick = 0;

static GPIO_PinState EstadoAnterior;
/* Indica que existe una presión pendiente de liberación */
static uint8_t PulsacionEnCurso = 0;

static volatile uint16_t ContadorOperaciones = 0;


/**
 * @brief Inicializa el módulo del pulsador.
 */
void Pulsador_Init(void)
{
    /* Leer el estado inicial del pulsador */
    EstadoAnterior = HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);

    EventoPendiente = 0;
    UltimoCambioTick = HAL_GetTick();

    ContadorOperaciones = 0;

    PulsacionEnCurso = 0;
}


/**
 * @brief Callback de interrupción externa.
 *
 * Se ejecuta cuando se detecta un flanco en PA5 (B1).
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == B1_Pin)
    {
        /* Registrar el instante del último cambio */
        UltimoCambioTick = HAL_GetTick();

        /* Indicar que hay un evento pendiente */
        EventoPendiente = 1;
    }
}


/**
 * @brief Procesa los eventos del pulsador y aplica antirrebote.
 *
 * Debe ejecutarse periódicamente desde el bucle principal.
 */
void Pulsador_Proceso(void)
{
    GPIO_PinState EstadoActual;
    uint32_t TiempoActual = HAL_GetTick();

    /* Verificar si existe un evento pendiente */
    if (EventoPendiente)
    {
        /* Esperar a que transcurra el tiempo de antirrebote */
        if ((TiempoActual - UltimoCambioTick) >=
            TIEMPO_ANTIRREBOTE)
        {
            /* Leer el estado actual del pulsador */
            EstadoActual = HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);

            /* Verificar si realmente cambió de estado */
            if (EstadoActual != EstadoAnterior)
            {
                EstadoAnterior = EstadoActual;

                /* Detectar presión del pulsador */
                if (EstadoActual == GPIO_PIN_RESET)
                {
                    PulsacionEnCurso = 1;
                }

                /* Detectar liberación del pulsador */
                else if ((EstadoActual == GPIO_PIN_SET) &&
                         (PulsacionEnCurso == 1))
                {
                    /* Completar una operación */
                    ContadorOperaciones++;

                    PulsacionEnCurso = 0;
                }
            }

            /* Finalizar el procesamiento del evento */
            EventoPendiente = 0;
        }
    }
}


/**
 * @brief Obtiene el contador de operaciones.
 *
 * @return Contador de 16 bits.
 */
uint16_t Pulsador_ObtenerContador(void)
{
    return ContadorOperaciones;
}
