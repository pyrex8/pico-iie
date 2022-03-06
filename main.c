#include <string.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/binary_info.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/structs/mpu.h"
#include "hardware/structs/vreg_and_chip_reset.h"


#include "mcu/clock.h"
#include "mcu/led.h"
#include "mcu/test.h"
#include "mcu/serial.h"
#include "mcu/speaker.h"
#include "mcu/vga.h"

#include "m6502/c6502.h"

#include "common/rom.h"
#include "common/ram.h"
#include "common/keyboard.h"
#include "common/joystick.h"
#include "common/audio.h"
#include "common/video.h"
#include "common/disk.h"


typedef enum
{
    MEMORY_WRITE = 0,
    MEMORY_READ
} MemoryRWStatus;

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
    SERIAL_USER_JOY_X,
    SERIAL_USER_JOY_Y,
} UserState;

static C6502_interface interface_c;
static uint8_t video_line_data[VIDEO_BYTES_PER_LINE] = {0};
static uint16_t video_data_address = 0x2000;

uint16_t scan_line;
int16_t overscan_line;
uint8_t overscan_line_odd;

static SerialMode serial_loader = SERIAL_READY;
static UserState user_state = SERIAL_USER_KEYBOARD;
static uint16_t bin_address = 0;
static uint32_t disk_address = 0;

static uint8_t joystick_x = 0;
static uint8_t joystick_y = 0;
static uint8_t button_0 = 0;
static uint8_t button_1 = 0;

static bool reset = false;

static void scan_line_ram_read(void)
{
    video_buffer_clear();

    if (video_is_mode_text())
    {
        video_data_address = 0x400 +
                            (0x400 * video_page_get()) +
                            (((scan_line>>3) & 0x07) * SCREEN_LINE_OFFSET) +
                            ((scan_line>>6) * VIDEO_SEGMENT_OFFSET);

        ram_data_get(VIDEO_BYTES_PER_LINE, video_data_address, video_line_data);
        video_text_line_update(scan_line, video_line_data);
    }
    else
    {
        video_data_address = 0x2000 +
                             (0x2000 * video_page_get()) +
                             (scan_line & 7) * 0x400 +
                             ((scan_line>>3) & 7) * 0x80 +
                             (scan_line>>6) * 0x28;
        ram_data_get(VIDEO_BYTES_PER_LINE, video_data_address, video_line_data);

        video_hires_line_update(scan_line, video_line_data);
    }
}

void main_init(void)
{
    rom_init();
    ram_init();
    video_init();
    c6502_init();
    speaker_init();
}

void uart_data(void)
{
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
                led_green_low();
            }
            else
            {
                led_green_high();
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
                led_green_low();
            }
            else
            {
                led_green_high();
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
}

void core1_main(void)
{
    while (1)
    {
        if (reset == true)
        {
            reset = false;
            c6502_reset(&interface_c);
        }

        test1_pin_high();
        c6502_update(&interface_c);
        test1_pin_low();

        ram_update(interface_c.rw, interface_c.address, &interface_c.data);
        rom_update(interface_c.rw, interface_c.address, &interface_c.data);
        disk_update(interface_c.rw, interface_c.address, &interface_c.data);
        keyboard_update(interface_c.rw, interface_c.address, &interface_c.data);
        joystick_update(interface_c.rw, interface_c.address, &interface_c.data);
        speaker_update(interface_c.rw, interface_c.address, &interface_c.data);
        video_update(interface_c.rw, interface_c.address, &interface_c.data);
    }
}

int main(void)
{
    clock_init();
    led_red_init();
    led_green_init();
    test0_pin_init();
    test1_pin_init();

    uart_init(UART_ID, UART_BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    main_init();

    multicore_launch_core1(core1_main);

    vga_init();

    while (1)
    {
        vga_wait_for_new_overscan_line();

        test0_pin_high();

        scan_line = vga_scan_line_get();
        video_scan_line_set(scan_line);
        overscan_line_odd = vga_overscan_line_is_odd();

        if (overscan_line_odd)
        {
            video_buffer_get(vga_scan_line_buffer());
        }
        else
        {
            scan_line_ram_read();
        }

        vga_blank_scan_line_set();

        uart_data();

        if (disk_spinning_test())
        {
            led_red_high();
        }
        else
        {
            led_red_low();
        }

        test0_pin_low();
    }
}
