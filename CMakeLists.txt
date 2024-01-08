cmake_minimum_required(VERSION 3.5)
#set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")

# linux platform
option(PLATFORM_LINUX "Linux target platform" OFF)
option(BUILD_EXAMPLE "Build example project" OFF)

# libpico2d default board is adafruit feather rp2040
if (NOT PLATFORM_LINUX AND NOT PICO_BOARD)
    set(PICO_BOARD "adafruit_feather_rp2040" CACHE STRING "" FORCE) # v1.0 rev board
    #set(PICO_BOARD "pico" CACHE STRING "" FORCE) # v1.1 rev board
endif ()

# pico options
option(PICO_STDIO_UART "Pico UART debug" ON)
option(PICO_DISPLAY_ST7789_PIO "Pico ST7789 display driver (pio)" ON)

if (NOT PLATFORM_LINUX)
    # initialize the SDK based on PICO_SDK_PATH
    # note: this must happen before project()
    include(${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/pico/cmake/pico_sdk_import.cmake)

    # for pico_audio_i2s, hardware_sleep
    include(${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/pico/cmake/pico_extras_import.cmake)
endif ()

project(pico2d-skeleton)

if (PLATFORM_LINUX)
    # i want all executable located at root of build directory
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} CACHE STRING "" FORCE)
else ()
    # initialize the Raspberry Pi Pico SDK
    pico_sdk_init()
endif ()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# TODO: why ?
#set(CMAKE_FLAGS_RELEASE_FIX "-fno-move-loop-invariants -fno-devirtualize -fno-devirtualize-speculatively")
set(CMAKE_C_FLAGS_RELEASE "-DNDEBUG -O3 ${CMAKE_FLAGS_RELEASE_FIX}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3 ${CMAKE_FLAGS_RELEASE_FIX}" CACHE STRING "" FORCE)

# set source files
file(GLOB PLATFORM_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c*
        ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/*.c*
        ${CMAKE_CURRENT_SOURCE_DIR}/src/utility/*.c*
        ${CMAKE_CURRENT_SOURCE_DIR}/src/compat/*.c*
        ${CMAKE_CURRENT_SOURCE_DIR}/external/Adafruit-GFX-Library/Adafruit_GFX.cpp
)
set(PLATFORM_INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets
        ${CMAKE_CURRENT_SOURCE_DIR}/src/utility
        ${CMAKE_CURRENT_SOURCE_DIR}/src/compat
        ${CMAKE_CURRENT_SOURCE_DIR}/external/Adafruit-GFX-Library
)

set(PLATFORM_FLAGS "-DNO_PICO_LED -DARDUINO=123 -ffast-math")
if (PICO_STDIO_UART)
    list(APPEND PLATFORM_FLAGS -DPICO_STDIO_UART)
endif ()

# tweeny
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/tweeny tweeny)

# resources
include(${CMAKE_CURRENT_SOURCE_DIR}/src/cmake/add_res.cmake)

if (PLATFORM_LINUX)
    file(GLOB_RECURSE TEMP ${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/linux/*.c*)
    list(APPEND PLATFORM_SOURCES ${TEMP})
    list(APPEND PLATFORM_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/linux)
    # SDL2
    include(FindPkgConfig)
    pkg_search_module(SDL2 REQUIRED sdl2)
    list(APPEND PLATFORM_INCLUDES ${SDL2_INCLUDE_DIRS})
    list(APPEND PLATFORM_FLAGS -DLINUX)
    list(APPEND PLATFORM_LIBS ${SDL2_LIBRARIES})
else ()
    file(GLOB TEMP ${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/pico/*.c*)
    list(APPEND PLATFORM_SOURCES ${TEMP})
    list(APPEND PLATFORM_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/pico)
    list(APPEND PLATFORM_FLAGS
            PARAM_ASSERTIONS_DISABLE_ALL=1
            PICO_ENTER_USB_BOOT_ON_EXIT=1
            PICO_STDIO_ENABLE_CRLF_SUPPORT=0
            PICO_STDIO_DEFAULT_CRLF=0
            PICO_PRINTF_SUPPORT_FLOAT=0
            PICO_PRINTF_SUPPORT_EXPONENTIAL=0
            PICO_PRINTF_SUPPORT_LONG_LONG=1
            PICO_PRINTF_SUPPORT_PTRDIFF_T=0
            PICO_AUDIO_I2S_PIO=1
            PICO_AUDIO_I2S_DMA_IRQ=1
            #PICO_DEFAULT_UART_TX_PIN=12
            #PICO_DEFAULT_UART_RX_PIN=13
    )
    list(APPEND PLATFORM_LIBS
            -Wl,--print-memory-usage
            pico_stdlib
            pico_multicore
            pico_audio_i2s
            hardware_sleep
            hardware_pio
            hardware_dma
            hardware_flash
    )
    # spi sdcard driver
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI build)
    list(APPEND PLATFORM_LIBS FatFs_SPI)
    # display driver
    if (PICO_DISPLAY_ST7789_PIO)
        list(APPEND PLATFORM_SOURCES
                ${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/pico/display/st7789/st7789.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/pico/display/st7789/pico_display.cpp
        )
        list(APPEND PLATFORM_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/pico/display/st7789)
        list(APPEND PLATFORM_LIBS hardware_dma)
    endif ()
    # microboy board revision define
    if (PICO_BOARD STREQUAL "adafruit_feather_rp2040")
        list(APPEND PLATFORM_FLAGS -DMICROBOY_10)
    else ()
        list(APPEND PLATFORM_FLAGS -DMICROBOY_11 "-DPICO_FLASH_SIZE_BYTES=(16 * 1024 * 1024)")
    endif ()
endif ()

####################
# skeleton library
####################
add_library(pico2d-skeleton ${PLATFORM_SOURCES})
target_include_directories(pico2d-skeleton PUBLIC ${PLATFORM_INCLUDES})
target_compile_definitions(pico2d-skeleton PUBLIC ${PLATFORM_FLAGS})
target_link_libraries(pico2d-skeleton ${PLATFORM_LIBS} tweeny)
if (NOT PLATFORM_LINUX)
    if (PICO_DISPLAY_ST7789_PIO)
        pico_generate_pio_header(pico2d-skeleton
                ${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/pico/display/st7789/st7789_lcd.pio)
    endif ()
    # debug output
    if (PICO_STDIO_UART)
        pico_enable_stdio_usb(pico2d-skeleton 0)
        pico_enable_stdio_uart(pico2d-skeleton 1)
    else ()
        pico_enable_stdio_usb(pico2d-skeleton 1)
        pico_enable_stdio_uart(pico2d-skeleton 0)
    endif ()
endif ()

####################
# example
####################
if (BUILD_EXAMPLE)
    add_subdirectory(example)
endif ()
