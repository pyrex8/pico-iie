#ifndef __SERIAL_H__
#define __SERIAL_H__

// Configuration
#define UART_ID uart1
#define UART_BAUD_RATE 230400

#define UART_TX_PIN 20
#define UART_RX_PIN 21

typedef enum
{
    SERIAL_READY = 0x80,
    SERIAL_USER = 0x81,
    SERIAL_BIN = 0x82,
    SERIAL_DISK = 0x83
} SerialMode;

typedef enum
{
    SERIAL_USER_KEYBOARD = 0,
    SERIAL_USER_BTN_0,
    SERIAL_USER_BTN_1,
    SERIAL_USER_JOY_X,
    SERIAL_USER_JOY_Y,
} UserState;

void serial_init(void);
void serial_data(void);

#endif /* __SERIAL_H__ */
