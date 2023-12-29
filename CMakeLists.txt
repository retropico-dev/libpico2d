cmake_minimum_required(VERSION 3.13)
#set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")

# linux platform
option(PLATFORM_LINUX "Linux target platform" OFF)

# libpico2d default board is adafruit feather rp2040
if (NOT PLATFORM_LINUX AND NOT PICO_BOARD)
    set(PICO_BOARD "adafruit_feather_rp2040")
endif ()

# pico options
option(PICO_STDIO_UART "Pico UART debug" ON)
option(PICO_DISPLAY_ST7789_PIO "Pico ST7789 display driver (pio)" ON)

if (NOT PLATFORM_LINUX)
    # initialize the SDK based on PICO_SDK_PATH
    # note: this must happen before project()
    include(${CMAKE_SOURCE_DIR}/src/skeleton/platforms/pico/cmake/pico_sdk_import.cmake)

    # for pico_audio_i2s, hardware_sleep
    include(${CMAKE_SOURCE_DIR}/src/skeleton/platforms/pico/cmake/pico_extras_import.cmake)
endif ()

project(pico2d)

if (PLATFORM_LINUX)
    # i want all executable located at root of build directory
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} CACHE STRING "" FORCE)
else ()
    # initialize the Raspberry Pi Pico SDK
    pico_sdk_init()
endif ()

# microboy-skeleton (cross-platform library)
add_subdirectory(src/skeleton)

# microboy-ui (ui binary)
add_subdirectory(src/example)
