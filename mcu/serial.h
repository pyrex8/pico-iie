#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <stdint.h>

// Configuration
#define UART_ID uart1
#define UART_BAUD_RATE 230400

#define UART_TX_PIN 20
#define UART_RX_PIN 21

typedef enum
{
    SERIAL_MAIN_NULL = 0,
    SERIAL_MAIN_REBOOT,
    SERIAL_MAIN_RESET,
    SERIAL_MAIN_PAUSE,
    SERIAL_MAIN_START_BIN,
    SERIAL_MAIN_START_DISK,
    SERIAL_KEYBOARD_CODE,
    SERIAL_JOYSTICK_BTN0,
    SERIAL_JOYSTICK_BTN1,
    SERIAL_JOYSTICK_PDL0,
    SERIAL_JOYSTICK_PDL1,
    SERIAL_RAM_BIN_RESET,
    SERIAL_RAM_BIN_DATA,
    SERIAL_DISK_RESET,
    SERIAL_DISK_DATA,
    SERIAL_OPERATIONS_TOTAL,
} SerialOperation;

void serial_init(void);
void serial_update(SerialOperation *operation, uint8_t *data);

#endif /* __SERIAL_H__ */
