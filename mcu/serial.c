#include "serial.h"
#include "pico/stdlib.h"

typedef enum
{
    SERIAL_READY = 0x80,
    SERIAL_USER = 0x81,
    SERIAL_BIN = 0x82,
    SERIAL_SIZE_LSB = 0x83,
    SERIAL_SIZE_MSB = 0x84,
    SERIAL_ADDR_LSB = 0x85,
    SERIAL_ADDR_MSB = 0x86,
    SERIAL_RESET = 0x87,
    SERIAL_REBOOT = 0x88,
} SerialMode;

typedef enum
{
    SERIAL_USER_KEYBOARD = 0,
    SERIAL_USER_BTN_0,
    SERIAL_USER_BTN_1,
    SERIAL_USER_JOY_X,
    SERIAL_USER_JOY_Y,
} UserState;

static SerialMode serial_loader = SERIAL_READY;
static UserState user_state = SERIAL_USER_KEYBOARD;
static uint16_t bin_data_length = 0;
static uint16_t bin_data_counter = 0;

static uint8_t joystick_x = 0;
static uint8_t joystick_y = 0;
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

        if(serial_loader == SERIAL_USER)
        {
            // 5 bytes keyboard, button_0, button_1, paddle_0, paddle_1
            if (user_state == SERIAL_USER_KEYBOARD)
            {
                if (serial_byte > 0 && serial_byte < 128)
                {
                    *operation = SERIAL_KEYBOARD_CODE;
                }
                else if (serial_byte == 128)
                {
                    *operation = SERIAL_MAIN_RESET;
                }
                else if (serial_byte == 129)
                {
                    *operation = SERIAL_MAIN_PAUSE;
                }
                else if (serial_byte == 130)
                {
                    *operation = SERIAL_MAIN_REBOOT;
                }
                user_state++;
            }
            else if (user_state == SERIAL_USER_BTN_0)
            {
                user_state++;
                *operation = SERIAL_JOYSTICK_BTN0;
            }
            else if (user_state == SERIAL_USER_BTN_1)
            {
                user_state++;
                *operation = SERIAL_JOYSTICK_BTN1;
            }
            else if (user_state == SERIAL_USER_JOY_X)
            {
                user_state++;
                *operation = SERIAL_JOYSTICK_PDL0;
            }
            else
            {
                user_state = SERIAL_USER_KEYBOARD;
                serial_loader = SERIAL_READY;
                *operation = SERIAL_JOYSTICK_PDL1;
            }
        }
        else if(serial_loader == SERIAL_BIN)
        {
            // 48k = 0xC000
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
        else if(serial_loader == SERIAL_SIZE_LSB)
        {
            serial_loader = SERIAL_READY;
            bin_data_length &= 0xFF00;
            bin_data_length |= serial_byte;
            *operation = SERIAL_MAIN_NULL;
            bin_data_counter = 0;
        }
        else if(serial_loader == SERIAL_SIZE_MSB)
        {
            serial_loader = SERIAL_READY;
            bin_data_length &= 0x00FF;
            bin_data_length |= (((uint16_t)serial_byte) << 8);
            *operation = SERIAL_MAIN_NULL;
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
            if(serial_byte == SERIAL_USER)
            {
                serial_loader = SERIAL_USER;
                user_state = SERIAL_USER_KEYBOARD;
                *operation = SERIAL_MAIN_NULL;
            }
            else if(serial_byte == SERIAL_BIN)
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
            else if(serial_byte == SERIAL_RESET)
            {
                *operation = SERIAL_MAIN_RESET;
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
