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

#include "main.h"

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

static C6502_interface interface_c;
static uint8_t video_line_data[VIDEO_BYTES_PER_LINE] = {0};
static uint16_t video_address = 0x2000;
static bool reset = false;
static uint8_t running = 1;
static uint16_t scan_line;
static int16_t overscan_line;
static uint8_t overscan_line_odd;

static SerialOperation serial_operation;
static uint8_t serial_data;
static const void (*main_serial_operation[SERIAL_OPERATIONS_TOTAL]) (uint8_t data) =
{
    [SERIAL_MAIN_NULL]              = main_null,
    [SERIAL_MAIN_RESET]             = main_reset,
    [SERIAL_MAIN_PAUSE]             = main_pause,
    [SERIAL_MAIN_START_BIN]         = main_start_bin,
    [SERIAL_MAIN_START_DISK]        = main_start_disk,
    [SERIAL_KEYBOARD_CODE]          = keyboard_key_code_set,
    [SERIAL_JOYSTICK_BTN0]          = joystick_btn0_set,
    [SERIAL_JOYSTICK_BTN1]          = joystick_btn1_set,
    [SERIAL_JOYSTICK_PDL0]          = joystick_pdl0_set,
    [SERIAL_JOYSTICK_PDL1]          = joystick_pdl1_set,
    [SERIAL_RAM_BIN_RESET]          = ram_bin_reset,
    [SERIAL_RAM_BIN_DATA]           = ram_bin_data_set,
    [SERIAL_DISK_RESET]             = disk_file_reset,
    [SERIAL_DISK_DATA]              = disk_file_data_set,
};

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
            video_buffer_clear();
            video_address = video_address_get();
            ram_data_get(VIDEO_BYTES_PER_LINE, video_address, video_line_data);
            video_line_data_get(video_line_data);
        }

        serial_update(&serial_operation, &serial_data);
        (*main_serial_operation[serial_operation]) (serial_data);

        led_red_set(disk_is_spinning());
        led_green_set(serial_data != 0);

        test0_pin_low();
    }
}
