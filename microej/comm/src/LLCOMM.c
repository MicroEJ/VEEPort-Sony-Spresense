#include <LLCOMM_impl.h>
#include <LLCOMM_UART.h>

extern LLCOMM_UART LLCOMM_UARTs[1];
/**
 * This function is the first function called.
 * Some connections can be added using {@link LLCOMM_addConnection}.
 * These connections are considered to be static connections and cannot be removed.
 */
void LLCOMM_IMPL_initialize(void)
{
    // create MicroEJ UART connection
    LLCOMM_UART_new(&LLCOMM_UARTs[0].header);
    // add MicroEJ UART connection into MicroEJ framework as generic connection
    LLCOMM_addConnection((LLCOMM_CONNECTION *)&LLCOMM_UARTs[0].header);
}

/**
 * This function is called before accessing the pool of connections, from initialize/add/remove/open/close operations.
 * If the implementation dynamically adds or removes connections, it must implement a reentrant synchronization mechanism to avoid execution of parallel operations.
 * If the implementation does not dynamically add or remove connections, it can do nothing.
 */
void LLCOMM_IMPL_syncConnectionsEnter(void)
{}

/**
 * This function is called after modifying the array of connections.
 * If the implementation dynamically adds or removes connections, it must implement a reentrant synchronization mechanism to avoid execution of parallel operations.
 * If the implementation does not dynamically add or remove connections, it can do nothing.
 */
void LLCOMM_IMPL_syncConnectionsExit(void)
{}
