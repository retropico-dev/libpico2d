cmake_minimum_required(VERSION 3.5)
#set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")

# main options
option(BUILD_EXAMPLE "Build example project" OFF)
# pico options
option(PICO_DEBUG_UART "Pico uart debug" OFF)
option(PICO_DEBUG_USB "Pico usb debug" OFF)
option(PICO_DISPLAY_ST7789_PIO "Pico ST7789 display driver (pio)" ON)
set(PICO_DISPLAY_ROTATION "90" CACHE STRING "Pico display rotation (0, 90, 180, 270)")

# too many "problems" to handles
# - requires 6 pins
# - ADC + resistors or shift register for buttons
# - very (very) fast for large read/write, but slow for "pixel drawing"
option(PICO_SPI_PSRAM "Pico psram spi driver (pio)" OFF)

# libpico2d default board is adafruit feather rp2040
if (DEFINED ENV{PICO_SDK_PATH})
    if (NOT PICO_BOARD)
        #set(PICO_BOARD "adafruit_feather_rp2040" CACHE STRING "" FORCE) # v1.0 rev board
        set(PICO_BOARD "waveshare_rp2040_plus_16mb" CACHE STRING "" FORCE) # v1.1 rev board
        #set(PICO_BOARD "pimoroni_pico_plus2_rp2350" CACHE STRING "" FORCE) # v2.0 rev board
    endif ()
    # initialize the SDK based on PICO_SDK_PATH
    # note: this must happen before project()
    include(${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/pico/cmake/pico_sdk_import.cmake)

    # for pico_audio_i2s, hardware_sleep
    include(${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/pico/cmake/pico_extras_import.cmake)
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

# set source files
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
#-DNO_PICO_LED

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
    )

    if (PICO_PLATFORM STREQUAL "rp2040")
        list(APPEND PLATFORM_LIBS hardware_sleep)
    endif ()

    # display driver
    if (PICO_DISPLAY_ST7789_PIO)
        file(GLOB TEMP ${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/pico/display/st7789/*.c*)
        list(APPEND PLATFORM_SOURCES ${TEMP})
        list(APPEND PLATFORM_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/pico/display/st7789)
    endif ()

    if (PICO_SPI_PSRAM)
        # psram module
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/pico/aps6404)
        list(APPEND PLATFORM_FLAGS -DPICO_PSRAM=1)
        list(APPEND PLATFORM_LIBS aps6404)
    endif ()

    # retropico board revision pinout definition
    if (PICO_BOARD STREQUAL "adafruit_feather_rp2040")
        list(APPEND PLATFORM_FLAGS -DRETROPICO_10)
    else ()
        list(APPEND PLATFORM_FLAGS -DRETROPICO_11) # Waveshare rp2040 plus
    endif ()

    # picoprobe debug
    if (PICO_DEBUG_UART)
        list(APPEND PLATFORM_FLAGS -DPICO_DEBUG_UART)
    elseif (PICO_DEBUG_USB)
        list(APPEND PLATFORM_FLAGS -DPICO_DEBUG_USB)
    endif ()

    # custom linker script
    function(set_linker_script TARGET script)
        get_filename_component(SCRIPT_PATH ${script} DIRECTORY)
        target_link_directories(${TARGET} PRIVATE ${SCRIPT_PATH})
        pico_set_linker_script(${TARGET} ${script})
        # Add dependencies on the 'included' linker script
        # so that the target gets rebuilt on changes
        pico_add_link_depend(${TARGET} ${script})
    endfunction()

    # custom linker script (easy version)
    function(set_linker_flash_size_4 TARGET)
        set_linker_script(${TARGET}
                "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/src/platforms/pico/misc/memmap_4mb.ld")
    endfunction()
    # custom linker script (easy version)
    function(set_linker_flash_size_8 TARGET)
        set_linker_script(${TARGET}
                "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/src/platforms/pico/misc/memmap_8mb.ld")
    endfunction()
    # custom linker script (easy version)
    function(set_linker_flash_size_16 TARGET)
        set_linker_script(${TARGET}
                "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/src/platforms/pico/misc/memmap_16mb.ld")
    endfunction()

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
add_library(pico2d-skeleton ${PLATFORM_SOURCES})
target_include_directories(pico2d-skeleton PUBLIC ${PLATFORM_INCLUDES})
target_compile_definitions(pico2d-skeleton PUBLIC ${PLATFORM_FLAGS})
target_link_libraries(pico2d-skeleton PUBLIC ${PLATFORM_LIBS})

if (PICO_BOARD)
    # ST7789 pio
    if (PICO_DISPLAY_ST7789_PIO)
        pico_generate_pio_header(pico2d-skeleton
                ${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/pico/display/st7789/st7789-spi.pio
        )
    endif ()

    # sdcard
    pico_generate_pio_header(pico2d-skeleton
            ${CMAKE_CURRENT_SOURCE_DIR}/src/platforms/pico/storage/spi.pio)

    # debug
    if (PICO_DEBUG_UART)
        pico_enable_stdio_usb(pico2d-skeleton 0)
        pico_enable_stdio_uart(pico2d-skeleton 1)
    elseif (PICO_DEBUG_USB)
        pico_enable_stdio_usb(pico2d-skeleton 1)
        pico_enable_stdio_uart(pico2d-skeleton 0)
    else ()
        pico_enable_stdio_usb(pico2d-skeleton 0)
        pico_enable_stdio_uart(pico2d-skeleton 0)
    endif ()
endif ()

####################
# example
####################
if (BUILD_EXAMPLE)
    add_subdirectory(example)
endif ()
