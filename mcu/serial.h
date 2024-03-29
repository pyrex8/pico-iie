#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <stdint.h>

// Configuration
#define UART_ID uart0
#define UART_BAUD_RATE_AT_MODE 9600
#define UART_BAUD_RATE 115200

#define UART_TX_PIN 16
#define UART_RX_PIN 17

typedef enum
{
    SERIAL_MAIN_NULL = 0,
    SERIAL_MAIN_REBOOT,
    SERIAL_MAIN_START_BIN,
    SERIAL_RAM_BIN_RESET,
    SERIAL_RAM_BIN_SIZE_LSB,
    SERIAL_RAM_BIN_SIZE_MSB,
    SERIAL_RAM_BIN_ADDR_LSB,
    SERIAL_RAM_BIN_ADDR_MSB,
    SERIAL_RAM_BIN_DATA,
    SERIAL_MENU_BANK,
    SERIAL_NAME_DATA,
    SERIAL_MAIN_BIN_STORE,
    SERIAL_OPERATIONS_TOTAL,
} SerialOperation;

void serial_init(void);
void serial_update(SerialOperation *operation, uint8_t *data);
void serial_state_send(void);

#endif /* __SERIAL_H__ */
