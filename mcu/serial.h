#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <stdint.h>

// Configuration
#define UART_ID uart0
#define UART_BAUD_RATE_AT_MODE 9600
#define UART_BAUD_RATE 115200

#define UART_TX_PIN 0
#define UART_RX_PIN 1

typedef enum
{
    SERIAL_MAIN_NULL = 0,
    SERIAL_MAIN_REBOOT,
    SERIAL_MAIN_START_BIN,
    SERIAL_RAM_BIN_RESET,
    SERIAL_RAM_BIN_ADDR_LSB,
    SERIAL_RAM_BIN_ADDR_MSB,
    SERIAL_RAM_BIN_DATA,
    SERIAL_OPERATIONS_TOTAL,
} SerialOperation;

void serial_init(void);
void serial_update(SerialOperation *operation, uint8_t *data);
void serial_test(void);

#endif /* __SERIAL_H__ */
