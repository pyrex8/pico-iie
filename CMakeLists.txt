cmake_minimum_required(VERSION 3.12)

project(pico-iie)

include(pico_sdk_import.cmake)
pico_sdk_init()

add_executable(pico-iie
    main.c
)

pico_generate_pio_header(pico-iie ${CMAKE_CURRENT_LIST_DIR}/parallel.pio)

target_link_libraries(pico-iie pico_stdlib hardware_pio hardware_dma hardware_pwm)

pico_add_extra_outputs(pico-iie)
