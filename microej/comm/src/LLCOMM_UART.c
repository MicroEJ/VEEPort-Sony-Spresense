#include <termios.h>
#include <fcntl.h>
#include <stdint.h>
#include <stddef.h>
#include <poll.h>
#include <errno.h>
#include <sched.h>
#include <string.h>
#include <nuttx/fs/ioctl.h>

#include "LLCOMM_UART.h"
#define LLCOMM_CUSTOM_CONNECTION_IMPL LLCOMM_UART
#include "LLCOMM_CUSTOM_CONNECTION_impl.h"
#include "LLCOMM.h"

  // #define ECOM_COMM_LLAPI_DEBUG_TRACE

#ifdef ECOM_COMM_LLAPI_DEBUG_TRACE
#define ECOM_COMM_LLAPI_DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define ECOM_COMM_LLAPI_DEBUG_PRINTF(...) ((void)0)
#endif


void LLCOM_UART_new(LLCOMM_UART *env);

/*
 * Array of all static LLCOMM UARTs.
 */
LLCOMM_UART LLCOMM_UARTs[1];

bool uart_opened;
bool rx_enable;
bool shouldPollout = true;
static void LLCOM_UART_IMPL_rxData_polling_func(void);

static int32_t getBaudrate(int32_t baudrate)
{
	switch (baudrate)
	{
		case 50:
			return B50;
		case 75:
			return B75;
		case 110:
			return B110;
		case 134:
			return B134;
		case 150:
			return B150;
		case 200:
			return B200;
		case 300:
			return B300;
		case 600:
			return B600;
		case 1200:
			return B1200;
		case 1800:
			return B1800;
		case 2400:
			return B2400;
		case 4800:
			return B4800;
		case 9600:
			return B9600;
		case 19200:
			return B19200;
		case 38400:
			return B38400;
		case 57600:
			return B57600;
		case 115200:
			return B115200;
		default:
			return LLCOMM_CUSTOM_CONNECTION_ERROR_INIT_UNSUPPORTED_BAUDRATE;
	}
}

/**
 * Configure a serial stream. A stream can be re-configured after it is closed.
 * @param baudrate a positive integer
 * @param bitsperchar {@link LLCOMM_CUSTOM_CONNECTION_DATABITS_5} | {@link LLCOMM_CUSTOM_CONNECTION_DATABITS_6} | {@link LLCOMM_CUSTOM_CONNECTION_DATABITS_7} | {@link LLCOMM_CUSTOM_CONNECTION_DATABITS_8} | {@link LLCOMM_CUSTOM_CONNECTION_DATABITS_9}
 * @param stopbits {@link LLCOMM_CUSTOM_CONNECTION_STOPBITS_1} | {@link LLCOMM_CUSTOM_CONNECTION_STOPBITS_2} | {@link LLCOMM_CUSTOM_CONNECTION_STOPBITS_1_5}
 * @param parity {@link LLCOMM_CUSTOM_CONNECTION_PARITY_NO} | {@link LLCOMM_CUSTOM_CONNECTION_PARITY_EVEN} | {@link LLCOMM_CUSTOM_CONNECTION_PARITY_ODD}
 * @return {@link LLCOMM_CUSTOM_CONNECTION_SUCCESS} if initialization is successful, or negative error code ({@link LLCOMM_CUSTOM_CONNECTION_ERROR_INIT_UNSUPPORTED_BAUDRATE} | {@link LLCOMM_CUSTOM_CONNECTION_ERROR_INIT_UNSUPPORTED_BITSPERCHAR} | {@link LLCOMM_CUSTOM_CONNECTION_ERROR_INIT_UNSUPPORTED_STOPBITS} | {@link LLCOMM_CUSTOM_CONNECTION_ERROR_INIT_UNSUPPORTED_PARITY} | {@link LLCOMM_CUSTOM_CONNECTION_ERROR_CANNOT_OPEN})
 */
int32_t LLCOMM_CUSTOM_CONNECTION_IMPL_configureDevice(LLCOMM_CUSTOM_CONNECTION *env, int32_t baudrate, int32_t bitsperchar, int32_t stopbits, int32_t parity)
{
	ECOM_COMM_LLAPI_DEBUG_PRINTF("%s \n", __func__);
	LLCOMM_UART *comm = (LLCOMM_UART *)env;

	//first the baudrate
	int32_t calculatedBaudrate = getBaudrate(baudrate);
	if (calculatedBaudrate == LLCOMM_CUSTOM_CONNECTION_ERROR_INIT_UNSUPPORTED_BAUDRATE)
	{
		// invalid baudrate
		return LLCOMM_CUSTOM_CONNECTION_ERROR_INIT_UNSUPPORTED_BAUDRATE;
	}

	//the control modes flag
	uint32_t c_cflag = CLOCAL | CREAD;

	//second bitperchar
	c_cflag &= ~CSIZE; /* Mask the character size bits */
	switch (bitsperchar)
	{
		case LLCOMM_CUSTOM_CONNECTION_DATABITS_5:
			c_cflag |= CS5; /* Select 5 data bits */
			break;
		case LLCOMM_CUSTOM_CONNECTION_DATABITS_6:
			c_cflag |= CS6; /* Select 6 data bits */
			break;
		case LLCOMM_CUSTOM_CONNECTION_DATABITS_7:
			c_cflag |= CS7; /* Select 7 data bits */
			break;
		case LLCOMM_CUSTOM_CONNECTION_DATABITS_8:
			c_cflag |= CS8; /* Select 8 data bits */
			break;

		default:
			return LLCOMM_CUSTOM_CONNECTION_ERROR_INIT_UNSUPPORTED_BITSPERCHAR; //unsupported bitperchar;
	}

	//third stopbits
	c_cflag &= ~CSTOPB; //stopbits to one, default mode
	switch (stopbits)
	{
		case LLCOMM_CUSTOM_CONNECTION_STOPBITS_1:
			//nothing to do it is the default mode
			break;
		case LLCOMM_CUSTOM_CONNECTION_STOPBITS_2:
			c_cflag |= CSTOPB;
			break;

		case LLCOMM_CUSTOM_CONNECTION_STOPBITS_1_5:
			//not supported => fall back to default
		default:
			return LLCOMM_CUSTOM_CONNECTION_ERROR_INIT_UNSUPPORTED_STOPBITS; //unsupported stopbits;
	}

	//fourth the parity
	c_cflag &= ~(PARENB | PARODD); //disable the parity
	switch (parity)
	{
		case LLCOMM_CUSTOM_CONNECTION_PARITY_NO:
			//nothing to do the parity is disabled
			break;
		case LLCOMM_CUSTOM_CONNECTION_PARITY_EVEN:
			c_cflag |= PARENB; //enable the parity setting
			c_cflag &= ~PARODD;
			break;
		case LLCOMM_CUSTOM_CONNECTION_PARITY_ODD:
			c_cflag |= PARENB; //enable the parity setting
			c_cflag |= PARODD;
			break;
		default:
			return LLCOMM_CUSTOM_CONNECTION_ERROR_INIT_UNSUPPORTED_PARITY; //unsupported parity
	}

	baudrate = calculatedBaudrate;

	struct termios tio;
	tcgetattr(comm->fd, &tio);
	//set the wanted flags
	tio.c_iflag = 0;
	tio.c_lflag = 0;
	tio.c_oflag = 0;
	tio.c_cflag = c_cflag;
	// Configure read to block until one byte is available (see man 3 termios)
	tio.c_cc[VMIN] = 1;
	tio.c_cc[VTIME] = 0;
	int ret_speed = cfsetspeed(&tio, calculatedBaudrate);
	int ret_attr = tcsetattr(comm->fd, TCSANOW, &tio);
	if (ret_speed == -1 || ret_attr == -1)
	{
		return LLCOMM_CUSTOM_CONNECTION_ERROR_CANNOT_OPEN;
	}
	return LLCOMM_CUSTOM_CONNECTION_SUCCESS;
}

/**
 * Initialize the comm driver and hardware to support this stream.
 */
void LLCOMM_CUSTOM_CONNECTION_IMPL_initialize(LLCOMM_CUSTOM_CONNECTION *env)
{
	ECOM_COMM_LLAPI_DEBUG_PRINTF("%s \n", __func__);
	LLCOMM_UART *comm = (LLCOMM_UART *)env;
	int fd = open("/dev/ttyS2", O_RDWR | O_NDELAY);
	if (fd > 0)
	{
		comm->fd = fd;
	}

	uart_opened = true;
	rx_enable = true;


	ECOM_COMM_LLAPI_DEBUG_PRINTF("%s -- task_create \n", __func__);
	task_create("UART2_RX", 100, 256, LLCOM_UART_IMPL_rxData_polling_func, NULL);
	ECOM_COMM_LLAPI_DEBUG_PRINTF("%s -- END \n", __func__);
}

void LLCOMM_CUSTOM_CONNECTION_IMPL_released(LLCOMM_CUSTOM_CONNECTION *env)
{
	ECOM_COMM_LLAPI_DEBUG_PRINTF("%s \n", __func__);
	LLCOMM_UART *comm = (LLCOMM_UART *)env;
	uart_opened = false;
	close((comm->fd));
}

int32_t LLCOMM_CUSTOM_CONNECTION_IMPL_initializeRXData(LLCOMM_CUSTOM_CONNECTION *env)
{
	ECOM_COMM_LLAPI_DEBUG_PRINTF("%s \n", __func__);
	return LLCOMM_CUSTOM_CONNECTION_SUCCESS;
}

/**
 * Actually send the given value to the hardware for output on the serial line.
 * The most significant bits of the given value must be ignored, and only the
 * least significant ones used. The number of bits to use is given when configuring the port.
 * @param data
 */
int32_t LLCOMM_CUSTOM_CONNECTION_IMPL_sendData(LLCOMM_CUSTOM_CONNECTION *env, int32_t data)
{
	ECOM_COMM_LLAPI_DEBUG_PRINTF("%s \n", __func__);
	LLCOMM_UART *comm = ((LLCOMM_UART *)env);
	if (comm->enableTx == 1)
	{
		char data_byte[1] = {(char)data};
		int byte_sent = write(comm->fd, (void *)(data_byte), 1);
		shouldPollout = true;
		if (byte_sent == 1)
		{
			return LLCOMM_CUSTOM_CONNECTION_SUCCESS;
		}
		// byte not sent
	}
	return LLCOMM_CUSTOM_CONNECTION_TRANSMISSION_ERROR;
}

/**
 * @return the number of bytes waiting in the RX buffer
 */
int32_t LLCOMM_CUSTOM_CONNECTION_IMPL_isRXDataAvailable(LLCOMM_CUSTOM_CONNECTION *env)
{
	ECOM_COMM_LLAPI_DEBUG_PRINTF("%s \n", __func__);
	LLCOMM_UART *comm = ((LLCOMM_UART *)env);
	if (comm->enableRx == 1)
	{
		// Make sure to flush unread data before reading available byte.
		tcflush(comm->fd, TCIFLUSH);
		int32_t availableBytes = 0;
		//get available bytes
		if (ioctl(comm->fd, FIONREAD, &availableBytes) < 0)
		{
			// an error occurs
			return -1;
		}
		return availableBytes;
	}
	//can not get available bytes when RX is not enabled
	return -1;
}

/**
 * Get the next character from the RX buffer
 */
int32_t LLCOMM_CUSTOM_CONNECTION_IMPL_getNextRXData(LLCOMM_CUSTOM_CONNECTION *env)
{
	ECOM_COMM_LLAPI_DEBUG_PRINTF("%s \n", __func__);
	LLCOMM_UART *comm = ((LLCOMM_UART *)env);
	if (comm->enableRx == 1)
	{
		char bytes[1];
		int32_t nbread = read(comm->fd, (void *)bytes, 1);
		ECOM_COMM_LLAPI_DEBUG_PRINTF("%s : read %i bytes \n", __func__, nbread);
		if (nbread > 0)
		{
			return bytes[0] & 0xff;
		}
		else
		{
			//if (nbread == 0) : something happens during read and nothing has been read
			// this happen when an USB to serial cable is unplugged.
			//else : an error occurs
			//return -1 in both case.
			return -1;
		}
	}
	//read operation is not allowed when RX is not enabled
	return -1;
}

/**
 * Initialize the buffer used for sending data.
 * @return {@link LLCOMM_CUSTOM_CONNECTION_SUCCESS} on success, and an implementation defined error code on failure.
 */
int32_t LLCOMM_CUSTOM_CONNECTION_IMPL_initializeTXData(LLCOMM_CUSTOM_CONNECTION *env)
{
	ECOM_COMM_LLAPI_DEBUG_PRINTF("%s \n", __func__);
	return LLCOMM_CUSTOM_CONNECTION_SUCCESS;
}

/**
 * Wait until the transmit buffers (hardware and software) are empty and the
 * transmission unit is idle, or a timeout occurs.
 * @return {@link LLCOMM_CUSTOM_CONNECTION_TRANSMISSION_COMPLETE} when the transmission is complete, {@link LLCOMM_CUSTOM_CONNECTION_TRANSMISSION_TIMEOUT} when a timeout occurs.
 */
int32_t LLCOMM_CUSTOM_CONNECTION_IMPL_isTransmissionComplete(LLCOMM_CUSTOM_CONNECTION *env)
{
	ECOM_COMM_LLAPI_DEBUG_PRINTF("%s \n", __func__);
	LLCOMM_UART *comm = ((LLCOMM_UART *)env);
	if (comm->enableTx == 1)
	{
		int uart_fd = (comm->fd);
		int result = tcdrain(uart_fd);
		if (result < 0)
		{
			// most likely interrupted by a close
			return LLCOMM_CUSTOM_CONNECTION_TRANSMISSION_ERROR;
		}
		return LLCOMM_CUSTOM_CONNECTION_TRANSMISSION_COMPLETE;
	}
	return LLCOMM_CUSTOM_CONNECTION_TRANSMISSION_ERROR;
}

static void LLCOM_UART_IMPL_rxData_polling_func()
{
	ECOM_COMM_LLAPI_DEBUG_PRINTF("%s \n", __func__);
	LLCOMM_UART *comm = &(LLCOMM_UARTs[0]);
	while (uart_opened)
	{
		struct pollfd fds[1];
		memset(fds, 0, sizeof(fds));
		fds[0].fd = comm->fd;
		if(shouldPollout)
		{
			fds[0].events = POLLIN | POLLOUT;
		}
		else
		{
			fds[0].events = POLLIN;
		}
		// Slow the poll down to not miss events
		usleep(10);
		int ret = poll(fds, 1, -1);
		if (ret <= 0)
		{
			if(errno != EINTR)
			{
				// Error
			}
		}
		else {
			if ( comm->enableRxInterrupt && (fds[0].revents & (POLLIN)) )
			{
				LLCOMM_CUSTOM_CONNECTION_dataReceived(&(comm->header));
			}
			else if ( comm->enableTxInterrupt && (fds[0].revents & POLLOUT) )
			{
				shouldPollout = false;
				LLCOMM_CUSTOM_CONNECTION_transmitBufferReady(&(comm->header));
			}
		}
	}
}

void *LLCOMM_CUSTOM_CONNECTION_IMPL_getName(LLCOMM_CUSTOM_CONNECTION *env)
{
	ECOM_COMM_LLAPI_DEBUG_PRINTF("%s \n", __func__);
	return (void *)0;
}

int32_t LLCOMM_CUSTOM_CONNECTION_IMPL_getPlatformId(LLCOMM_CUSTOM_CONNECTION *env)
{
	ECOM_COMM_LLAPI_DEBUG_PRINTF("%s \n", __func__);
	return 18;
}

int32_t LLCOMM_CUSTOM_CONNECTION_IMPL_getNbProperties(LLCOMM_CUSTOM_CONNECTION *env)
{
	ECOM_COMM_LLAPI_DEBUG_PRINTF("%s \n", __func__);
	// This implementation does not provide Device properties
	return 0;
}

void *LLCOMM_CUSTOM_CONNECTION_IMPL_getProperty(LLCOMM_CUSTOM_CONNECTION *env, void *propertyName)
{
	ECOM_COMM_LLAPI_DEBUG_PRINTF("%s \n", __func__);
	// This implementation does not provide Device properties
	return 0;
}

void *LLCOMM_CUSTOM_CONNECTION_IMPL_getPropertyName(LLCOMM_CUSTOM_CONNECTION *env, int32_t propertyId)
{
	ECOM_COMM_LLAPI_DEBUG_PRINTF("%s \n", __func__);
	// This implementation does not provide Device properties
	while (1)
		;
}

/**
 * Enable RX hardware and start accepting data.
 */
void LLCOMM_CUSTOM_CONNECTION_IMPL_enableRX(LLCOMM_CUSTOM_CONNECTION *env)
{
	ECOM_COMM_LLAPI_DEBUG_PRINTF("%s \n", __func__);
	LLCOMM_UART *comm = ((LLCOMM_UART *)env);
	comm->enableRx = true;
}

/**
 * Disable RX hardware and stop accepting data.
 */
void LLCOMM_CUSTOM_CONNECTION_IMPL_disableRX(LLCOMM_CUSTOM_CONNECTION *env)
{
	ECOM_COMM_LLAPI_DEBUG_PRINTF("%s \n", __func__);
	LLCOMM_UART *comm = ((LLCOMM_UART *)env);
	comm->enableRx = false;
}

/**
 * Enable the interrupt that is triggered when new data is received.
 */
void LLCOMM_CUSTOM_CONNECTION_IMPL_enableRXDeviceInterrupt(LLCOMM_CUSTOM_CONNECTION *env)
{
	ECOM_COMM_LLAPI_DEBUG_PRINTF("%s \n", __func__);
	LLCOMM_UART *comm = ((LLCOMM_UART *)env);
	comm->enableRxInterrupt = true;
}

/**
 * Disable the interrupt that is triggered when new data is received.
 */
void LLCOMM_CUSTOM_CONNECTION_IMPL_disableRXDeviceInterrupt(LLCOMM_CUSTOM_CONNECTION *env)
{
	ECOM_COMM_LLAPI_DEBUG_PRINTF("%s \n", __func__);
	LLCOMM_UART *comm = ((LLCOMM_UART *)env);
	comm->enableRxInterrupt = false;
}

/**
 * Enable TX hardware and start sending data.
 */
void LLCOMM_CUSTOM_CONNECTION_IMPL_enableTX(LLCOMM_CUSTOM_CONNECTION *env)
{
	ECOM_COMM_LLAPI_DEBUG_PRINTF("%s \n", __func__);
	LLCOMM_UART *comm = ((LLCOMM_UART *)env);
	comm->enableTx = true;
}

/**
 * Disable TX hardware and stop sending data.
 */
void LLCOMM_CUSTOM_CONNECTION_IMPL_disableTX(LLCOMM_CUSTOM_CONNECTION *env)
{
	ECOM_COMM_LLAPI_DEBUG_PRINTF("%s \n", __func__);
	LLCOMM_UART *comm = ((LLCOMM_UART *)env);
	comm->enableTx = false;
}

/**
 * Enable the interrupt that is triggered when there is free space in the hardware
 * transmit FIFO or register.
 *
 * When the interrupt occurs the interrupt handler should call transmitBufferReady().
 */
void LLCOMM_CUSTOM_CONNECTION_IMPL_enableTXDeviceInterrupt(LLCOMM_CUSTOM_CONNECTION *env)
{
	ECOM_COMM_LLAPI_DEBUG_PRINTF("%s \n", __func__);
	LLCOMM_UART *comm = ((LLCOMM_UART *)env);
	comm->enableTxInterrupt = true;
}

/**
 * Disable the interrupt that is triggered when there is free space in the hardware
 * transmit FIFO or register.
 */
void LLCOMM_CUSTOM_CONNECTION_IMPL_disableTXDeviceInterrupt(LLCOMM_CUSTOM_CONNECTION *env)
{
	ECOM_COMM_LLAPI_DEBUG_PRINTF("%s \n", __func__);
	LLCOMM_UART *comm = ((LLCOMM_UART *)env);
	comm->enableTxInterrupt = false;
}
