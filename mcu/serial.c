#include <string.h>
#include "serial.h"
#include "pico/stdlib.h"

#define SERIAL_NAME_LENGTH 27

typedef enum
{
    SERIAL_READY = 0x81,
    SERIAL_BIN = 0x82,
    SERIAL_SIZE_LSB = 0x83,
    SERIAL_SIZE_MSB = 0x84,
    SERIAL_ADDR_LSB = 0x85,
    SERIAL_ADDR_MSB = 0x86,
    SERIAL_REBOOT = 0x87,
    SERIAL_BANK = 0x88,
    SERIAL_NAME = 0x89,
} SerialMode;

static SerialMode serial_loader = SERIAL_READY;
static uint16_t bin_data_length = 0;
static uint16_t bin_data_counter = 0;
static uint16_t name_data_counter = 0;

static uint8_t game_x = 0;
static uint8_t game_y = 0;
static uint8_t button_0 = 0;
static uint8_t button_1 = 0;

void serial_init(void)
{
    uart_init(UART_ID, UART_BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
}

void serial_update(SerialOperation *operation, uint8_t *data)
{
    uint8_t serial_byte = 0;
    *operation = SERIAL_MAIN_NULL;

    if(uart_is_readable(UART_ID))
    {
        serial_byte = uart_getc(UART_ID);

        if(serial_loader == SERIAL_BIN)
        {
            if (bin_data_counter >= bin_data_length)
            {
                serial_loader = SERIAL_READY;
                bin_data_counter = 0;
                *operation = SERIAL_MAIN_START_BIN;
            }
            else
            {
                bin_data_counter++;
                *operation = SERIAL_RAM_BIN_DATA;
            }
        }
        else if(serial_loader == SERIAL_NAME)
        {
            if (name_data_counter >= SERIAL_NAME_LENGTH)
            {
                serial_loader = SERIAL_READY;
                name_data_counter = 0;
                *operation = SERIAL_MAIN_NULL;
            }
            else
            {
                name_data_counter++;
                *operation = SERIAL_NAME_DATA;
            }
        }
        else if(serial_loader == SERIAL_BANK)
        {
            serial_loader = SERIAL_READY;
            *operation = SERIAL_MENU_BANK;
        }
        else if(serial_loader == SERIAL_SIZE_LSB)
        {
            serial_loader = SERIAL_READY;
            bin_data_length &= 0xFF00;
            bin_data_length |= serial_byte;
            *operation = SERIAL_RAM_BIN_SIZE_LSB;
            bin_data_counter = 0;
        }
        else if(serial_loader == SERIAL_SIZE_MSB)
        {
            serial_loader = SERIAL_READY;
            bin_data_length &= 0x00FF;
            bin_data_length |= (((uint16_t)serial_byte) << 8);
            *operation = SERIAL_RAM_BIN_SIZE_MSB;
            bin_data_counter = 0;
        }
        else if(serial_loader == SERIAL_ADDR_LSB)
        {
            serial_loader = SERIAL_READY;
            *operation = SERIAL_RAM_BIN_ADDR_LSB;
        }
        else if(serial_loader == SERIAL_ADDR_MSB)
        {
            serial_loader = SERIAL_READY;
            *operation = SERIAL_RAM_BIN_ADDR_MSB;
        }
        else if(serial_loader == SERIAL_READY)
        {
            if(serial_byte == SERIAL_BIN)
            {
                serial_loader = SERIAL_BIN;
                bin_data_counter = 0;
                *operation = SERIAL_RAM_BIN_RESET;
            }
            else if(serial_byte == SERIAL_SIZE_LSB)
            {
                serial_loader = SERIAL_SIZE_LSB;
                *operation = SERIAL_MAIN_NULL;
            }
            else if(serial_byte == SERIAL_SIZE_MSB)
            {
                serial_loader = SERIAL_SIZE_MSB;
                *operation = SERIAL_MAIN_NULL;
            }
            else if(serial_byte == SERIAL_ADDR_LSB)
            {
                serial_loader = SERIAL_ADDR_LSB;
                *operation = SERIAL_MAIN_NULL;
            }
            else if(serial_byte == SERIAL_ADDR_MSB)
            {
                serial_loader = SERIAL_ADDR_MSB;
                *operation = SERIAL_MAIN_NULL;
            }
            else if(serial_byte == SERIAL_NAME)
            {
                serial_loader = SERIAL_NAME;
                *operation = SERIAL_MAIN_NULL;
            }
            else if(serial_byte == SERIAL_BANK)
            {
                serial_loader = SERIAL_BANK;
                *operation = SERIAL_MAIN_NULL;
            }
            else if(serial_byte == SERIAL_REBOOT)
            {
                *operation = SERIAL_MAIN_REBOOT;
            }
            else
            {
                *operation = SERIAL_MAIN_NULL;
            }
        }
    }
    *data = serial_byte;
}


void serial_state_send(void)
{
    uart_putc(UART_ID, 'A');
}
