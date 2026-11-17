
#include "main.h"
#include "control_uart.h"
#include "administrador_leds.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* Handle de la UART definido en main.c (CubeMX con CoupleFile=false) */
extern UART_HandleTypeDef huart1;

/* Tamaño del buffer de recepción */
#define TAM_BUFFER_RX       64

/* Tamaño máximo de un comando */
#define TAM_COMANDO         16

/* Tiempo máximo de espera entre caracteres */
#define TIMEOUT_UART        300

/* Buffer circular de recepción */
static uint8_t BufferRX[TAM_BUFFER_RX];

static volatile uint8_t IndiceEntrada = 0;
static volatile uint8_t IndiceSalida = 0;

/* Variable utilizada para recibir un byte por interrupción */
static uint8_t DatoRecibido;

/* Variables de recepción */
static volatile uint32_t UltimoByteTick = 0;
static volatile uint8_t ErrorBuffer = 0;

/* Buffer del comando */
static char Comando[TAM_COMANDO];
static uint8_t IndiceComando = 0;

static uint8_t RecibioCR = 0;

static uint8_t DescartaComando = 0;

/* Parámetros de la transmisión */
#define TAM_COLA_TX       8
#define TAM_MENSAJE_TX    32

/* Cola circular de transmisión */
static uint8_t BufferTX[TAM_COLA_TX][TAM_MENSAJE_TX];
static uint8_t LongitudTX[TAM_COLA_TX];

static volatile uint8_t IndiceTXEntrada = 0;
static volatile uint8_t IndiceTXSalida = 0;

static volatile uint8_t TransmisionActiva = 0;


/**
 * @brief Inicia la transmisión del siguiente mensaje pendiente.
 */
static void ControlUART_IniciarTransmision(void)
{
    /* Verificar si ya existe una transmisión activa */
    if (TransmisionActiva)
        return;

    /* Verificar si hay mensajes pendientes */
    if (IndiceTXSalida == IndiceTXEntrada)
        return;

    TransmisionActiva = 1;

    /* Iniciar transmisión mediante interrupciones */
    if (HAL_UART_Transmit_IT(
            &huart1,
            BufferTX[IndiceTXSalida],
            LongitudTX[IndiceTXSalida]) == HAL_OK)
    {
        /* El mensaje queda en poder de la UART hasta finalizar */
        IndiceTXSalida = (IndiceTXSalida + 1) % TAM_COLA_TX;
    }
    else
    {
        TransmisionActiva = 0;
    }
}


/**
 * @brief Agrega un mensaje a la cola de transmisión.
 */
static void ControlUART_EnviarTexto(const char *texto)
{
    uint8_t siguienteIndice;
    uint8_t longitud;

    longitud = strlen(texto);

    /* Verificar que el mensaje tenga un tamaño válido */
    if ((longitud == 0) || (longitud >= TAM_MENSAJE_TX))
        return;

    siguienteIndice = (IndiceTXEntrada + 1) % TAM_COLA_TX;

    /* Verificar que exista espacio en la cola */
    if (siguienteIndice == IndiceTXSalida)
        return;

    /* Copiar el mensaje a su posición en la cola */
    memcpy(BufferTX[IndiceTXEntrada], texto, longitud);

    LongitudTX[IndiceTXEntrada] = longitud;

    /* Actualizar índice de entrada */
    IndiceTXEntrada = siguienteIndice;

    /* Intentar iniciar la transmisión */
    ControlUART_IniciarTransmision();
}

/**
 * @brief Comprueba si los caracteres recibidos hasta ahora
 *        todavía pueden formar un comando válido.
 */
static uint8_t ControlUART_PrefijoValido(void)
{
    uint8_t longitud = IndiceComando;

    /* Comandos LSec y LOff */
    if ((longitud <= 4) &&
        ((strncmp(Comando, "LSec", longitud) == 0) ||
         (strncmp(Comando, "LOff", longitud) == 0)))
    {
        return 1;
    }

    /* Comando individual: Lx,tttt */
    if (longitud <= 7)
    {
        if ((longitud >= 1) && (Comando[0] != 'L'))
            return 0;

        if ((longitud >= 2) &&
            ((Comando[1] < '1') || (Comando[1] > '3')))
            return 0;

        if ((longitud >= 3) && (Comando[2] != ','))
            return 0;

        if ((longitud >= 4) && (Comando[3] < '0' || Comando[3] > '9'))
            return 0;

        if ((longitud >= 5) && (Comando[4] < '0' || Comando[4] > '9'))
            return 0;

        if ((longitud >= 6) && (Comando[5] < '0' || Comando[5] > '9'))
            return 0;

        if ((longitud >= 7) && (Comando[6] < '0' || Comando[6] > '9'))
            return 0;

        return 1;
    }

    return 0;
}
/**
 * @brief Inicia la recepción UART por interrupción.
 */
void ControlUART_Init(void)
{
    IndiceEntrada = 0;
    IndiceSalida = 0;


    IndiceComando = 0;
    RecibioCR = 0;

    DescartaComando = 0;

    ErrorBuffer = 0;
    UltimoByteTick = HAL_GetTick();

    IndiceTXEntrada = 0;
    IndiceTXSalida = 0;
    TransmisionActiva = 0;

    HAL_UART_Receive_IT(&huart1, &DatoRecibido, 1);
}


/**
 * @brief Callback de recepción UART.
 *
 * Se ejecuta cuando se recibe un byte.
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        uint8_t SiguienteIndice;

        SiguienteIndice = (IndiceEntrada + 1) % TAM_BUFFER_RX;

        /* Verificar que exista espacio en el buffer */
        if (SiguienteIndice != IndiceSalida)
        {
            BufferRX[IndiceEntrada] = DatoRecibido;
            IndiceEntrada = SiguienteIndice;
        }
        else
        {
            ErrorBuffer = 1;
        }

        UltimoByteTick = HAL_GetTick();

        /* Preparar la recepción del siguiente byte */
        HAL_UART_Receive_IT(&huart1, &DatoRecibido, 1);
    }
}


/**
 * @brief Callback ejecutado al finalizar una transmisión UART.
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        /* La transmisión actual ha finalizado */
        TransmisionActiva = 0;

        /* Iniciar el siguiente mensaje pendiente */
        ControlUART_IniciarTransmision();
    }
}


/**
 * @brief Callback de error UART (overrun, ruido, framing, paridad).
 *
 * En el HAL de la familia F1 un error de overrun finaliza la recepcion
 * por interrupcion. Sin este callback el modulo dejaria de recibir
 * bytes de forma permanente, por lo que se vuelve a armar la recepcion.
 * Si la recepcion sigue activa (errores no bloqueantes) HAL devuelve
 * HAL_BUSY y la llamada no tiene efecto.
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        HAL_UART_Receive_IT(&huart1, &DatoRecibido, 1);
    }
}


/**
 * @brief Interpreta y ejecuta un comando recibido.
 */
static void ControlUART_InterpretarComando(void)
{
    uint8_t numeroLED;
    uint16_t intervalo;

    /* Comando para iniciar la secuencia */
    if (strcmp(Comando, "LSec") == 0)
    {
        AdministradorLEDs_IniciarSecuencia();

        ControlUART_EnviarTexto("OK\n");
    }

    /* Comando para apagar todos los LED */
    else if (strcmp(Comando, "LOff") == 0)
    {
        AdministradorLEDs_ApagarTodos();

        ControlUART_EnviarTexto("OK\n");
    }

    /* Comando individual: Lx,tttt */
    else if ((Comando[0] == 'L') &&
             (Comando[1] >= '1') &&
             (Comando[1] <= '3') &&
             (Comando[2] == ',') &&
             (strlen(Comando) == 7))
    {
        /* Verificar que los cuatro caracteres sean números */
        if ((Comando[3] >= '0') && (Comando[3] <= '9') &&
            (Comando[4] >= '0') && (Comando[4] <= '9') &&
            (Comando[5] >= '0') && (Comando[5] <= '9') &&
            (Comando[6] >= '0') && (Comando[6] <= '9'))
        {
            numeroLED = Comando[1] - '0';
            intervalo = (uint16_t)atoi(&Comando[3]);

            if (AdministradorLEDs_ConfigurarParpadeo(
                    numeroLED, intervalo))
            {
                ControlUART_EnviarTexto("OK\n");
            }
            else
            {
                ControlUART_EnviarTexto("ERROR\n");
            }
        }
        else
        {
            ControlUART_EnviarTexto("ERROR\n");
        }
    }
    else
    {
        ControlUART_EnviarTexto("ERROR\n");
    }
}


/**
 * @brief Procesa los caracteres recibidos.
 */
void ControlUART_Proceso(void)
{
    uint8_t dato;

    /* Verificar desbordamiento del buffer */
    if (ErrorBuffer)
    {
        ErrorBuffer = 0;
        IndiceEntrada = 0;
        IndiceSalida = 0;
        IndiceComando = 0;
        RecibioCR = 0;
        DescartaComando = 0;

        ControlUART_EnviarTexto("ERROR\n");
        return;
    }

    /* Procesar los bytes pendientes */
    while (IndiceSalida != IndiceEntrada)
    {
        dato = BufferRX[IndiceSalida];

        IndiceSalida = (IndiceSalida + 1) % TAM_BUFFER_RX;

        /* Si el comando ya fue rechazado, descartar
           todo hasta encontrar el fin de línea */
        if (DescartaComando)
        {
            if (dato == '\n')
            {
                DescartaComando = 0;
                IndiceComando = 0;
                RecibioCR = 0;
            }

            continue;
        }

        /* Si ya se recibió CR, ahora esperamos LF */
        if (RecibioCR)
        {
            if (dato == '\n')
            {
                Comando[IndiceComando] = '\0';

                ControlUART_InterpretarComando();

                IndiceComando = 0;
                RecibioCR = 0;
            }
            else
            {
                IndiceComando = 0;
                RecibioCR = 0;
                DescartaComando = 1;

                ControlUART_EnviarTexto("ERROR\n");
            }

            continue;
        }

        /* Detectar retorno de carro */
        if (dato == '\r')
        {
            /* Solo aceptar CR si el comando está completo */
        	if ((IndiceComando == 7) ||
        	    ((IndiceComando == 4) &&
        	     ((strcmp(Comando, "LSec") == 0) ||
        	      (strcmp(Comando, "LOff") == 0))))
            {
                RecibioCR = 1;
            }
            else
            {
                IndiceComando = 0;
                DescartaComando = 1;

                ControlUART_EnviarTexto("ERROR\n");
            }

            continue;
        }

        /* Detectar LF sin CR previo */
        if (dato == '\n')
        {
            IndiceComando = 0;

            ControlUART_EnviarTexto("ERROR\n");
            continue;
        }

        /* Almacenar el carácter recibido */
        if (IndiceComando < (TAM_COMANDO - 1))
        {
            Comando[IndiceComando] = (char)dato;
            IndiceComando++;

            Comando[IndiceComando] = '\0';

            /* Validar inmediatamente el prefijo */
            if (!ControlUART_PrefijoValido())
            {
                IndiceComando = 0;
                DescartaComando = 1;

                ControlUART_EnviarTexto("ERROR\n");
            }
        }
        else
        {
            IndiceComando = 0;
            DescartaComando = 1;

            ControlUART_EnviarTexto("ERROR\n");
        }
    }

    /* Verificar timeout de recepción */
    if ((IndiceComando > 0 || RecibioCR || DescartaComando) &&
        ((HAL_GetTick() - UltimoByteTick) >= TIMEOUT_UART))
    {
        if (!DescartaComando)
        {
            ControlUART_EnviarTexto("Timeout\n");
        }

        IndiceComando = 0;
        RecibioCR = 0;
        DescartaComando = 0;
    }
}
 /** @brief Transmite el contador de operaciones por UART.
 *
 * @param contador Valor actual del contador.
 */
void ControlUART_EnviarContador(uint16_t contador)
{
    char mensaje[24];

    /* Formatear el mensaje */
    snprintf(mensaje, sizeof(mensaje),
             "Contador: %u\r\n",
             (unsigned int)contador);

    /* Transmitir el mensaje */
    ControlUART_EnviarTexto(mensaje);
}

