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
#include "mcu/test.h"
#include "mcu/joystick.h"
#include "mcu/ps2.h"
#include "mcu/menu.h"
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
static uint8_t menu = 0;
static uint8_t menu_data[MENU_CHARACTERS_SIZE];
static uint16_t scan_line;
static int16_t overscan_line;
static uint8_t overscan_line_odd;

static SerialOperation serial_operation;
static Ps2Operation ps2_operation;
static uint8_t operation_data;

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

static const void (*main_ps2_operation[PS2_OPERATIONS_TOTAL]) (uint8_t data) =
{
    [PS2_MAIN_NULL]                 = main_null,
    [PS2_MAIN_PAUSE]                = main_pause,
    [PS2_MAIN_RESUME]               = main_resume,
    [PS2_MAIN_RESET]                = main_reset,
    [PS2_MAIN_MENU]                 = main_menu,
    [PS2_MAIN_REBOOT]               = main_reboot,
};

void main_init(void)
{
    rom_init();
    ram_init();
    video_init();
    c6502_init();
    speaker_init();
    ps2_init();
    menu_init();
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
    menu = 0;
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
    menu = 1;
    menu_data_get(menu_data);
    ram_data_set(MENU_CHARACTERS_SIZE, 0x0400, menu_data);
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
        ps2_update();
    }
}

int main(void)
{
    clock_init();
    test_pin_init();
    serial_init();
    joystick_init();

    main_init();

    multicore_launch_core1(main_core1);

    vga_init();

    while (1)
    {
        
        vga_wait_for_new_overscan_line();
        

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

        serial_update(&serial_operation, &operation_data);
        (*main_serial_operation[serial_operation]) (operation_data);

        ps2_command(&ps2_operation, &operation_data);
        (*main_ps2_operation[ps2_operation]) (operation_data);

        if (ps2_data_ready())
        {
            keyboard_key_code_set(ps2_data_get());
        }

        if (vga_scan_line_get() == 0)
        {
            test_pin_high();
            joystick_update();
            game_btn0_set(joystick_btn0_get());
            game_btn1_set(joystick_btn1_get());
            game_pdl0_set(joystick_pdl0_get());
            game_pdl1_set(joystick_pdl1_get());
            
            serial_state_send();

            if (menu == 1)
            {
                menu_update();
            }

            test_pin_low();
        }
        
    }
}