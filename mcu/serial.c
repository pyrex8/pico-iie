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

}



// serial options
// 1. do nothing (0 bytes)
// 2. write keyboard character (1 byte)
// 3. reset (0 bytes)
// 4. pause (1 byte)
// 5. write joystick button 0
// 5. write joystick button 0
// 5. write joystick button 0
// 5. write joystick button 0

// 6. write ram (1 byte) + 16 bit address
// 7. start bin (write reset vector (2 bytes) + reset)
// 8. write disk (1 byte) + 32 bit index
// 9. start disk (main_init(), disk_init(), write reset vector, reset)

#if 0

// main_null(unused);
// keyboard_key_code_set(serial_byte);
// joystick_btn0_set(button_0);
// joystick_btn0_set(button_1);
// joystick_pdl0_set(joystick_x);
// joystick_pdl1_set(joystick_y);
ram_address_low_set(low);
ram_address_high_set(high);
ram_write_increment(byte);
rom_reset_vector_low_set(low);
rom_reset_vector_high_set(high);
disk_file_data_location_reset(unused);
// disk_file_data_set(serial_byte);
// main_start_bin(unused);
// main_reset(unused);
// main_pause(unused);
// main_start_disk(unused);

#endif
