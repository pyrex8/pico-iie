cmake_minimum_required(VERSION 3.12)

enable_language( C CXX ASM )

project(pico-iie)

include(pico_sdk_import.cmake)
pico_sdk_init()

add_executable(
    pico-iie
    main.c
    mcu/clock.c
    mcu/led.c
    mcu/test.c
    common/audio.c
    common/disk.c
    common/joystick.c
    common/keyboard.c
    common/ram.c
    common/rom.c
    common/video.c
    m6502/c6502
)

pico_generate_pio_header(pico-iie ${CMAKE_CURRENT_LIST_DIR}/parallel.pio)

target_link_libraries(
    pico-iie
    pico_stdlib
    hardware_vreg
    hardware_pio
    hardware_dma
    hardware_pwm
    pico_multicore)

pico_add_extra_outputs(pico-iie)

pico_define_boot_stage2(slower_boot2 ${PICO_DEFAULT_BOOT_STAGE2_FILE})
target_compile_definitions(slower_boot2 PRIVATE PICO_FLASH_SPI_CLKDIV=4)
pico_set_boot_stage2(pico-iie slower_boot2)
