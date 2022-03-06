#include "serial.h"
#include "pico/stdlib.h"

static SerialMode serial_loader = SERIAL_READY;
static UserState user_state = SERIAL_USER_KEYBOARD;
static uint16_t bin_address = 0;
static uint32_t disk_address = 0;

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

void serial_data(void)
{

#if 0

    uint8_t serial_byte = 0;

    if(uart_is_readable(UART_ID))
    {
        serial_byte = uart_getc(UART_ID);

        if(serial_loader == SERIAL_USER)
        {
            // 3 bytes keyboard, joystick_x + button_0, joystick_y + button_1
            if (user_state == SERIAL_USER_KEYBOARD)
            {
                if (serial_byte > 0 && serial_byte < 128)
                {
                    keyboard_key_code_set(serial_byte);
                }
                else if (serial_byte == 128)
                {
                    reset = true;
                }
                user_state++;
            }
            else if (user_state == SERIAL_USER_JOY_X)
            {
                joystick_x = serial_byte & 0xFE;
                button_0 = serial_byte & 0x01;
                user_state++;
            }
            else
            {
                joystick_y = serial_byte & 0xFE;
                button_1 = serial_byte & 0x01;
                joystick_state_set(button_0, button_1, joystick_x, joystick_y);
                user_state = SERIAL_USER_KEYBOARD;
                serial_loader = SERIAL_READY;
            }
        }

        if(serial_loader == SERIAL_BIN)
        {
            ram_update(MEMORY_WRITE, (bin_address + 0x803), &serial_byte);
            bin_address++;
            // 32k = 0x8000
            if (bin_address > 0x8000)
            {
                serial_loader = SERIAL_READY;
                bin_address = 0;
                rom_reset_vector_write(0x03, 0x08);
                reset = true;
            }
        }
        if(serial_loader == SERIAL_DISK)
        {
            disk_file_data_set(disk_address, serial_byte);
            disk_address++;
            if (disk_address > 143360)
            {
                serial_loader = SERIAL_READY;
                disk_address = 0;
                main_init();
                disk_init();
                rom_reset_vector_write(0x62, 0xFA);
                reset = true;
            }
        }
        if(serial_loader == SERIAL_READY)
        {
            if(serial_byte == SERIAL_USER)
            {
                serial_loader = SERIAL_USER;
                user_state = SERIAL_USER_KEYBOARD;
            }
            if(serial_byte == SERIAL_BIN)
            {
                serial_loader = SERIAL_BIN;
                bin_address = 0;
            }
            if(serial_byte == SERIAL_DISK)
            {
                serial_loader = SERIAL_DISK;
                disk_address = 0;
            }
        }
    }

#endif

}
