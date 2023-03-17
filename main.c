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
#include "hardware/watchdog.h"
#include "hardware/flash.h"

#include "main.h"

#include "mcu/clock.h"
#include "mcu/joystick.h"
#include "mcu/key.h"
#include "mcu/serial.h"
#include "mcu/speaker.h"
#include "mcu/vga.h"

#include "m6502/c6502.h"

#include "common/rom.h"
#include "common/ram.h"
#include "common/keyboard.h"
#include "common/game.h"
#include "common/audio.h"
#include "common/video.h"

static C6502_interface interface_c;
static uint8_t video_line_data[VIDEO_BYTES_PER_LINE] = {0};
static uint16_t video_address = 0x2000;
static bool reset = false;
static uint8_t running = 1;
static uint16_t scan_line;
static int16_t overscan_line;
static uint8_t overscan_line_odd;

static SerialOperation serial_operation;
static KeyOperation key_operation;
static uint8_t operation_data;

static const void (*main_key_operation[KEY_OPERATIONS_TOTAL]) (uint8_t data) =
{
    [KEY_MAIN_NULL]                 = main_null,
    [KEY_MAIN_PAUSE]                = main_pause,
    [KEY_MAIN_RESUME]               = main_resume,
    [KEY_MAIN_RESET]                = main_reset,
    [KEY_MAIN_REBOOT]               = main_reboot,
    [KEY_MAIN_MENU]                 = main_menu,
    [KEY_KEYBOARD_KEY]              = keyboard_key_code_set,
};

static const void (*main_serial_operation[SERIAL_OPERATIONS_TOTAL]) (uint8_t data) =
{
    [SERIAL_MAIN_NULL]              = main_null,
    [SERIAL_MAIN_REBOOT]            = main_reboot,
    [SERIAL_MAIN_START_BIN]         = main_start_bin,
    [SERIAL_RAM_BIN_RESET]          = ram_bin_reset,
    [SERIAL_RAM_BIN_ADDR_LSB]       = ram_bin_addr_lsb,
    [SERIAL_RAM_BIN_ADDR_MSB]       = ram_bin_addr_msb,
    [SERIAL_RAM_BIN_DATA]           = ram_bin_data_set,
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

void main_reboot(uint8_t unused)
{
    watchdog_enable(1, 1);
    while(1);
}

void main_reset(uint8_t unused)
{
    reset = true;
}

void main_pause(uint8_t unused)
{
    running = 0;
}

void main_resume(uint8_t unused)
{
    running = 1;
}

void main_menu(uint8_t unused)
{
}

void main_start_bin(uint8_t unused)
{
    rom_reset_vector_write(ram_bin_addr_get());
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
            c6502_update(&interface_c);

            ram_update(interface_c.rw, interface_c.address, &interface_c.data);
            rom_update(interface_c.rw, interface_c.address, &interface_c.data);
            keyboard_update(interface_c.rw, interface_c.address, &interface_c.data);
            game_update(interface_c.rw, interface_c.address, &interface_c.data);
            speaker_update(interface_c.rw, interface_c.address, &interface_c.data);
            video_update(interface_c.rw, interface_c.address, &interface_c.data);
        }
    }
}

int main(void)
{
    clock_init();
    serial_init();
    joystick_init();
    key_init();

    main_init();

    multicore_launch_core1(main_core1);

    vga_init();

    while (1)
    {
        vga_wait_for_new_overscan_line();

        scan_line = vga_scan_line_get();
        video_scan_line_set(scan_line);
        overscan_line_odd = vga_overscan_line_is_odd();

        key_update();

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

        serial_update(&serial_operation, &operation_data);
        (*main_serial_operation[serial_operation]) (operation_data);

        if (vga_scan_line_get() == 0)
        {
            joystick_update();
            game_btn0_set(joystick_btn0_get());
            game_btn1_set(joystick_btn1_get());
            game_pdl0_set(joystick_pdl0_get());
            game_pdl1_set(joystick_pdl1_get());

            serial_state_send();

            key_operation_get(&key_operation, &operation_data);
            (*main_key_operation[key_operation]) (operation_data);

            uint8_t data = 0xB0;
            ram_update(0, 0x400, &data);
            data = 0xF8;
            ram_update(0, 0x401, &data);
            data = key_ms_nibble_get();
            ram_update(0, 0x402, &data);
            data = key_ls_nibble_get();
            ram_update(0, 0x403, &data);
        }
    }
}
