
#include "LLCOMM_BUFFERED_CONNECTION.h"
#include <stdbool.h>
/* Defines and Macros --------------------------------------------------------*/

/*
 * Defines the software FIFO size for transmission used by MicroEJ framework.
 * Default size is 5 bytes.
 */
#ifndef LLCOMM_UART_TX_BUFFER_SIZE
#define LLCOMM_UART_TX_BUFFER_SIZE 5
#endif

/*
 * Defines the software FIFO size for reception used by MicroEJ framework
 * Default size is 1024 bytes.
 */
#ifndef LLCOMM_UART_RX_BUFFER_SIZE
#define LLCOMM_UART_RX_BUFFER_SIZE 1 * 1024
#endif

typedef struct  LLCOMM_UART
{
    // header
    struct LLCOMM_BUFFERED_CONNECTION header;

    // File descriptor pointing to uart2
    int fd;

    // Software FIFO size for transmission used by MicroEJ framework
    uint8_t txBuffer[LLCOMM_UART_TX_BUFFER_SIZE];

    // Software FIFO size for reception used by MicroEJ framework
    uint8_t rxBuffer[LLCOMM_UART_RX_BUFFER_SIZE];

    bool enableTx;

    bool enableTxInterrupt;

    bool enableRx;

    bool enableRxInterrupt;

} LLCOMM_UART;