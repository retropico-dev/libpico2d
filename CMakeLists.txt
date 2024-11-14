cmake_minimum_required(VERSION 3.5)
#set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")

# main options
option(BUILD_EXAMPLE "Build example project" OFF)
# pico options
option(PICO_DEBUG_UART "Pico uart debug" OFF)
option(PICO_DEBUG_USB "Pico usb debug" OFF)
option(PICO_PSRAM "Pico psram support (rp2040 spi / rp2350 qspi)" OFF)
option(PICO_DISPLAY_ST7789 "Pico ST7789 display support (pio)" ON)
set(PICO_DISPLAY_ROTATION "0" CACHE STRING "Pico display rotation (0, 90, 180, 270)")

# libpico2d default board is adafruit feather rp2040
if (DEFINED ENV{PICO_SDK_PATH})
    if (NOT PICO_BOARD)
        #set(PICO_BOARD "adafruit_feather_rp2040" CACHE STRING "" FORCE) # v1.0 rev board
        set(PICO_BOARD "waveshare_rp2040_plus_16mb" CACHE STRING "" FORCE) # v1.1 rev board
    endif ()
    if (PICO_PSRAM OR PICO_BOARD STREQUAL "pimoroni_pico_plus2_rp2350")
        # use a custom malloc wrapper
        set(SKIP_PICO_MALLOC 1 CACHE BOOL "" FORCE)
    endif ()

    # initialize the SDK based on PICO_SDK_PATH
    # note: this must happen before project()
    include(${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/pico/cmake/pico_sdk_import.cmake)

    # for pico_audio_i2s, hardware_sleep
    include(${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/pico/cmake/pico_extras_import.cmake)
else ()
    message(FATAL_ERROR "PICO_SDK_PATH not variable not set: https://github.com/raspberrypi/pico-sdk")
endif ()

project(pico2d-skeleton)

if (PICO_BOARD)
    # initialize the Raspberry Pi Pico SDK
    pico_sdk_init()
else ()
    # i want all executable located at root of build directory
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} CACHE STRING "" FORCE)
endif ()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# TODO: why ?
set(CMAKE_FLAGS_RELEASE_FIX "-fno-move-loop-invariants -fno-devirtualize -fno-devirtualize-speculatively")
set(CMAKE_C_FLAGS_RELEASE "-DNDEBUG -O3 -ffast-math -ffunction-sections -fdata-sections ${CMAKE_FLAGS_RELEASE_FIX}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3 -ffast-math -ffunction-sections -fdata-sections ${CMAKE_FLAGS_RELEASE_FIX}" CACHE STRING "" FORCE)

# common source files
file(GLOB PLATFORM_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c*
        ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/*.c*
        ${CMAKE_CURRENT_SOURCE_DIR}/src/utility/*.c*
        ${CMAKE_CURRENT_SOURCE_DIR}/src/compat/*.c*
        ${CMAKE_CURRENT_SOURCE_DIR}/src/fatfs/*.c*
        ${CMAKE_CURRENT_SOURCE_DIR}/external/Adafruit-GFX-Library/Adafruit_GFX.cpp
)
set(PLATFORM_INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets
        ${CMAKE_CURRENT_SOURCE_DIR}/src/utility
        ${CMAKE_CURRENT_SOURCE_DIR}/src/compat
        ${CMAKE_CURRENT_SOURCE_DIR}/src/fatfs
        ${CMAKE_CURRENT_SOURCE_DIR}/src/romfs
        ${CMAKE_CURRENT_SOURCE_DIR}/external/Adafruit-GFX-Library
)

list(APPEND PLATFORM_FLAGS -DPICO_DISPLAY_ROTATION=${PICO_DISPLAY_ROTATION} -DARDUINO=123)

# tweeny
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/tweeny tweeny)
list(APPEND PLATFORM_LIBS tweeny)

# resources (romfs)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CMakeRC.cmake)
list(APPEND PLATFORM_INCLUDES ${CMAKE_BINARY_DIR}/_cmrc/include)
function(target_add_resources target)
    cmrc_add_resource_library(${target}-resources NAMESPACE pico2d ${ARGN})
    target_link_libraries(${target} ${target}-resources)
    target_sources(${target} PRIVATE ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/src/romfs/romfs.cpp)
endfunction()

# gpio pinout configuration
include(cmake/GpioConfig.cmake)

if (PICO_BOARD)
    file(GLOB TEMP
            ${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/pico/*.c*
            ${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/pico/storage/*.c*
    )
    list(APPEND PLATFORM_SOURCES ${TEMP})
    list(APPEND PLATFORM_INCLUDES
            ${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/pico
            ${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/pico/misc
            ${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/pico/storage
    )
    list(APPEND PLATFORM_FLAGS
            PARAM_ASSERTIONS_DISABLE_ALL=1
            PICO_ENTER_USB_BOOT_ON_EXIT=1
            PICO_STDIO_ENABLE_CRLF_SUPPORT=0
            PICO_STDIO_DEFAULT_CRLF=0
            PICO_PRINTF_SUPPORT_FLOAT=1
            PICO_PRINTF_SUPPORT_EXPONENTIAL=0
            PICO_PRINTF_SUPPORT_LONG_LONG=1
            PICO_PRINTF_SUPPORT_PTRDIFF_T=0
            PICO_AUDIO_I2S_PIO=1
            PICO_AUDIO_I2S_DMA_IRQ=1
            PICO_DEFAULT_UART_TX_PIN=16
            PICO_DEFAULT_UART_RX_PIN=17
    )
    list(APPEND PLATFORM_LIBS
            -Wl,--print-memory-usage
            pico_stdlib
            pico_multicore
            pico_audio_i2s
            hardware_pio
            hardware_dma
            hardware_flash
            hardware_pwm
            hardware_adc
            #hardware_interp
    )

    # LCD GPIO
    add_gpio_definition(PLATFORM_FLAGS "GPIO_PIN_LCD_CLK" "5")
    add_gpio_definition(PLATFORM_FLAGS "GPIO_PIN_LCD_DIN" "4")
    add_gpio_definition(PLATFORM_FLAGS "GPIO_PIN_LCD_DC" "2")
    add_gpio_definition(PLATFORM_FLAGS "GPIO_PIN_LCD_CS" "1")
    add_gpio_definition(PLATFORM_FLAGS "GPIO_PIN_LCD_RST" "3")
    add_gpio_definition(PLATFORM_FLAGS "GPIO_PIN_LCD_BL" "0")
    # SD GPIO
    add_gpio_definition(PLATFORM_FLAGS "GPIO_PIN_SD_CS" "26")
    add_gpio_definition(PLATFORM_FLAGS "GPIO_PIN_SD_CLK" "22")
    add_gpio_definition(PLATFORM_FLAGS "GPIO_PIN_SD_MOSI" "21")
    add_gpio_definition(PLATFORM_FLAGS "GPIO_PIN_SD_MISO" "27")
    # AUDIO GPIO
    add_gpio_definition(PLATFORM_FLAGS "GPIO_PIN_AUDIO_DATA" "20")
    add_gpio_definition(PLATFORM_FLAGS "GPIO_PIN_AUDIO_CLOCK" "18")
    add_gpio_definition(PLATFORM_FLAGS "GPIO_PIN_AUDIO_LRC" "19")
    # INPUTS GPIO
    add_gpio_definition(PLATFORM_FLAGS "GPIO_PIN_BTN_VOL_UP" "6")
    add_gpio_definition(PLATFORM_FLAGS "GPIO_PIN_BTN_VOL_DOWN" "7")
    add_gpio_definition(PLATFORM_FLAGS "GPIO_PIN_BTN_UP" "8")
    add_gpio_definition(PLATFORM_FLAGS "GPIO_PIN_BTN_LEFT" "9")
    add_gpio_definition(PLATFORM_FLAGS "GPIO_PIN_BTN_DOWN" "10")
    add_gpio_definition(PLATFORM_FLAGS "GPIO_PIN_BTN_RIGHT" "11")
    add_gpio_definition(PLATFORM_FLAGS "GPIO_PIN_BTN_SELECT" "12")
    add_gpio_definition(PLATFORM_FLAGS "GPIO_PIN_BTN_START" "13")
    add_gpio_definition(PLATFORM_FLAGS "GPIO_PIN_BTN_A" "15")
    add_gpio_definition(PLATFORM_FLAGS "GPIO_PIN_BTN_B" "14")
    add_gpio_definition(PLATFORM_FLAGS "GPIO_PIN_BTN_SLEEP" "17")

    if (PICO_PLATFORM STREQUAL "rp2040")
        list(APPEND PLATFORM_LIBS hardware_sleep)
    endif ()

    # display driver
    if (PICO_DISPLAY_ST7789)
        file(GLOB TEMP ${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/pico/display/st7789/*.c*)
        list(APPEND PLATFORM_SOURCES ${TEMP})
        list(APPEND PLATFORM_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/pico/display/st7789)
    endif ()

    if (PICO_BOARD STREQUAL "pimoroni_pico_plus2_rp2350")
        set(PICO_PSRAM ON CACHE BOOL "" FORCE)
    endif ()

    # psram module
    # TODO: rp2350 PIMORONI_PICO_PLUS2_PSRAM_CS_PIN 47
    if (PICO_PSRAM)
        if (PICO_PLATFORM STREQUAL "rp2040")
            message(FATAL_ERROR "TODO: enable rp2040 psram code back")
            add_subdirectory(src/platforms/pico/psram/aps6404)
            list(APPEND PLATFORM_FLAGS -DPICO_PSRAM_RP2040=1)
            list(APPEND PLATFORM_LIBS aps6404)
        else ()
            list(APPEND PLATFORM_SOURCES
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/pico/psram/psram.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/pico/psram/malloc-wrapper.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/external/tlsf/tlsf.c
            )
            list(APPEND PLATFORM_INCLUDES
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/pico/psram
                    ${CMAKE_CURRENT_SOURCE_DIR}/external/tlsf/include
            )
            add_gpio_definition(PLATFORM_FLAGS "GPIO_PIN_PSRAM_CS" "47")
        endif ()
    endif ()

    # picoprobe debug
    if (PICO_DEBUG_UART)
        list(APPEND PLATFORM_FLAGS -DPICO_DEBUG_UART)
    elseif (PICO_DEBUG_USB)
        list(APPEND PLATFORM_FLAGS -DPICO_DEBUG_USB)
    endif ()

    # pico2d stuff (custom linker script, ...)
    if (NOT PICO_FLASH_SIZE_BYTES)
        if (PICO_DEFAULT_FLASH_SIZE_BYTES)
            set(PICO_FLASH_SIZE_BYTES ${PICO_DEFAULT_FLASH_SIZE_BYTES})
        else ()
            set(PICO_FLASH_SIZE_BYTES "2 * 1024 * 1024")
            message(WARNING "PICO_FLASH_SIZE_BYTES not set, using defaults (${PICO_FLASH_SIZE_BYTES})")
        endif ()
    endif ()
    function(target_set_pico2d_stuff TARGET)
        if (PICO_BOARD)
            set(PICO_FLASH_SIZE_BYTES_STRING "${PICO_FLASH_SIZE_BYTES}")
            set(PICO_PSRAM_SIZE_BYTES_STRING "(8 * 1024 * 1024)")
            if (PICO_PLATFORM STREQUAL "rp2040")
                configure_file(
                        ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/src/platforms/pico/misc/memmap_rp2040.ld
                        ${CMAKE_BINARY_DIR}/memmap.ld
                )
            else () # rp2350
                if (PICO_PSRAM)
                    configure_file(
                            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/src/platforms/pico/misc/memmap_rp2350_psram.ld
                            ${CMAKE_BINARY_DIR}/memmap.ld
                    )
                else ()
                    configure_file(
                            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/src/platforms/pico/misc/memmap_rp2350.ld
                            ${CMAKE_BINARY_DIR}/memmap.ld
                    )
                endif ()
            endif ()
            # add include path for linker scripts
            target_link_options(${TARGET} INTERFACE "LINKER:-L${CMAKE_BINARY_DIR}")
            pico_set_linker_script(${TARGET} ${CMAKE_BINARY_DIR}/memmap.ld)
            # add dependencies on the 'included' linker script
            # so that the target gets rebuilt on changes
            pico_add_link_depend(${TARGET} ${CMAKE_BINARY_DIR}/memmap.ld)

            # psram
            if (PICO_PSRAM)
                pico_wrap_function(${TARGET} malloc)
                pico_wrap_function(${TARGET} calloc)
                pico_wrap_function(${TARGET} realloc)
                pico_wrap_function(${TARGET} free)
            endif ()

            # debug
            pico_add_uf2_output(${TARGET})
            if (PICO_DEBUG_UART)
                pico_enable_stdio_usb(${TARGET} 0)
                pico_enable_stdio_uart(${TARGET} 1)
            elseif (PICO_DEBUG_USB)
                pico_enable_stdio_usb(${TARGET} 1)
                pico_enable_stdio_uart(${TARGET} 0)
            else ()
                pico_enable_stdio_usb(${TARGET} 0)
                pico_enable_stdio_uart(${TARGET} 0)
            endif ()
        endif ()
    endfunction()
else ()
    file(GLOB_RECURSE TEMP
            ${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/linux/*.c*
            ${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/linux/storage/*.c*
    )
    list(APPEND PLATFORM_SOURCES ${TEMP})
    list(APPEND PLATFORM_INCLUDES
            ${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/linux
            ${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/linux/storage
    )
    # SDL2
    include(FindPkgConfig)
    pkg_search_module(SDL2 REQUIRED sdl2)
    list(APPEND PLATFORM_INCLUDES ${SDL2_INCLUDE_DIRS})
    list(APPEND PLATFORM_FLAGS -DLINUX)
    list(APPEND PLATFORM_LIBS ${SDL2_LIBRARIES})
endif ()

####################
# skeleton library
####################
add_library(pico2d-skeleton INTERFACE)
target_sources(pico2d-skeleton INTERFACE ${PLATFORM_SOURCES})
target_include_directories(pico2d-skeleton INTERFACE ${PLATFORM_INCLUDES})
target_compile_definitions(pico2d-skeleton INTERFACE ${PLATFORM_FLAGS})
target_link_libraries(pico2d-skeleton INTERFACE ${PLATFORM_LIBS})

if (PICO_BOARD)
    # psram
    if (PICO_PSRAM)
        pico_wrap_function(pico2d-skeleton malloc)
        pico_wrap_function(pico2d-skeleton calloc)
        pico_wrap_function(pico2d-skeleton realloc)
        pico_wrap_function(pico2d-skeleton free)
    endif ()

    # ST7789 pio
    if (PICO_DISPLAY_ST7789)
        pico_generate_pio_header(pico2d-skeleton
                ${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/pico/display/st7789/st7789-spi.pio
        )
    endif ()

    # sdcard
    pico_generate_pio_header(pico2d-skeleton
            ${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/pico/storage/spi.pio)
endif ()

####################
# example
####################
if (BUILD_EXAMPLE)
    add_subdirectory(example)
endif ()
