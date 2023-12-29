cmake_minimum_required(VERSION 3.13)
#set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")

project(pico2d-ui)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# set source files
file(GLOB UI_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/*.c*
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/*.c*
        )
set(UI_INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets
        )
set(UI_FLAGS -DMB_UI)

####################
# skeleton library
####################
add_executable(pico2d-ui ${UI_SOURCES})
target_include_directories(pico2d-ui PUBLIC ${UI_INCLUDES})
target_compile_definitions(pico2d-ui PUBLIC ${UI_FLAGS})
target_link_libraries(pico2d-ui ${UI_LIBS} pico2d-skeleton)
if (NOT PLATFORM_LINUX)
    pico_add_uf2_output(pico2d-ui)
    # debug output
    if (PICO_STDIO_UART)
        pico_enable_stdio_usb(pico2d-ui 0)
        pico_enable_stdio_uart(pico2d-ui 1)
    else ()
        pico_enable_stdio_usb(pico2d-ui 1)
        pico_enable_stdio_uart(pico2d-ui 0)
    endif ()
endif ()
