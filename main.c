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

static bool reset = false;
static uint8_t running = 1;

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

void main_null(uint8_t unused)
{
}

void main_reset(uint8_t unused)
{
    reset = true;
}

void main_pause(uint8_t unused)
{
    running ^= 1;
}

void main_start_bin(uint8_t unused)
{
    rom_reset_vector_write(0x03, 0x08);
    main_reset(0);
}

void main_start_disk(uint8_t unused)
{
    main_init();
    disk_init();
    rom_reset_vector_write(0x62, 0xFA);
    main_reset(0);
}

void uart_data(void)
{
    uint8_t serial_byte = 0;

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
                    keyboard_key_code_set(serial_byte);
                }
                else if (serial_byte == 128)
                {
                    main_reset(0);
                }
                else if (serial_byte == 129)
                {
                    main_pause(0);
                }
                user_state++;
            }
            else if (user_state == SERIAL_USER_BTN_0)
            {
                user_state++;
                joystick_btn0_set(serial_byte);
            }
            else if (user_state == SERIAL_USER_BTN_1)
            {
                user_state++;
                joystick_btn1_set(serial_byte);
            }
            else if (user_state == SERIAL_USER_JOY_X)
            {
                user_state++;
                joystick_pdl0_set(serial_byte);
            }
            else
            {
                user_state = SERIAL_USER_KEYBOARD;
                serial_loader = SERIAL_READY;
                joystick_pdl1_set(serial_byte);
            }
        }
        else if(serial_loader == SERIAL_BIN)
        {
            // 32k = 0x8000
            if (bin_address >= 0x8000)
            {
                serial_loader = SERIAL_READY;
                bin_address = 0;
                main_start_bin(0);
            }
            else
            {
                bin_address++;
                ram_update(MEMORY_WRITE, (bin_address + 0x803 - 1), &serial_byte);
            }
        }
        else if(serial_loader == SERIAL_DISK)
        {
            if (disk_address > 143360)
            {
                serial_loader = SERIAL_READY;
                disk_address = 0;
                main_start_disk(0);
            }
            else
            {
                disk_address++;
                disk_file_data_set(disk_address - 1, serial_byte);
            }
        }
        else if(serial_loader == SERIAL_READY)
        {
            if(serial_byte == SERIAL_USER)
            {
                serial_loader = SERIAL_USER;
                user_state = SERIAL_USER_KEYBOARD;
                main_null(0);
            }
            else if(serial_byte == SERIAL_BIN)
            {
                serial_loader = SERIAL_BIN;
                bin_address = 0;
                main_null(0);
            }
            else if(serial_byte == SERIAL_DISK)
            {
                serial_loader = SERIAL_DISK;
                disk_address = 0;
                main_null(0);
            }
            else
            {
                main_null(0);
            }
        }
        else
        {
            main_null(0);
        }
    }
}

void main_core1(void)
{
    while (1)
    {
        if (reset == true)
        {
            reset = false;
            c6502_reset(&interface_c);
        }

        if (running)
        {
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
}

int main(void)
{
    clock_init();
    led_red_init();
    led_green_init();
    test0_pin_init();
    test1_pin_init();
    serial_init();

    main_init();

    multicore_launch_core1(main_core1);

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
        led_red_set(disk_is_spinning());
        led_green_set(serial_loader != SERIAL_READY);

        test0_pin_low();
    }
}
