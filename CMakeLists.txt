cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(pico-iie C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(pico-iie
    main.c
    mcu/clock.c
    mcu/led.c
    mcu/test.c
    mcu/serial.c
    mcu/speaker.c
    mcu/vga.c
    common/audio.c
    common/disk.c
    common/joystick.c
    common/keyboard.c
    common/ram.c
    common/rom.c
    common/video.c
    m6502/c6502.c
)

pico_generate_pio_header(pico-iie ${CMAKE_CURRENT_LIST_DIR}/parallel.pio)

target_link_libraries(
    pico-iie
    pico_stdlib
    pico_multicore
    hardware_vreg
    hardware_pio
    hardware_dma
    hardware_pwm
    hardware_irq
    hardware_structs
    )

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")

    add_compile_options(-Wall
            -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
            -Wno-unused-function # we have some for the docs that aren't called
            -Wno-maybe-uninitialized
            )

pico_add_extra_outputs(pico-iie)

pico_define_boot_stage2(slower_boot2 ${PICO_DEFAULT_BOOT_STAGE2_FILE})
target_compile_definitions(slower_boot2 PRIVATE PICO_FLASH_SPI_CLKDIV=4)
pico_set_boot_stage2(pico-iie slower_boot2)
