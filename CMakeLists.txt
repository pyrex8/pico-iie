cmake_minimum_required(VERSION 3.12)

project(pico-iie)

include(pico_sdk_import.cmake)
pico_sdk_init()

add_executable(pico-iie
    main.c
)

target_link_libraries(pico-iie pico_stdlib)

pico_add_extra_outputs(pico-iie)

#pico_define_boot_stage2(slower_boot2 ${PICO_DEFAULT_BOOT_STAGE2_FILE})
#target_compile_definitions(slower_boot2 PRIVATE PICO_FLASH_SPI_CLKDIV=4)
#pico_set_boot_stage2(pico-iie slower_boot2)
