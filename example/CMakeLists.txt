cmake_minimum_required(VERSION 3.13)
#set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")

project(pico2d-example)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

# set source files
file(GLOB UI_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.c*)
set(UI_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR})

####################
# skeleton library
####################
add_executable(pico2d-example ${UI_SOURCES} ${RES})
target_add_romfs(pico2d-example "${CMAKE_CURRENT_SOURCE_DIR}/data")
target_include_directories(pico2d-example PUBLIC ${UI_INCLUDES})
target_compile_definitions(pico2d-example PUBLIC ${UI_FLAGS})
target_link_libraries(pico2d-example ${UI_LIBS} pico2d-skeleton)

if (NOT PLATFORM_LINUX)
    pico_add_uf2_output(pico2d-example)
    # debug output
    if (PICO_STDIO_UART)
        pico_enable_stdio_usb(pico2d-example 0)
        pico_enable_stdio_uart(pico2d-example 1)
    else ()
        pico_enable_stdio_usb(pico2d-example 1)
        pico_enable_stdio_uart(pico2d-example 0)
    endif ()
endif ()
