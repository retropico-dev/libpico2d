cmake_minimum_required(VERSION 3.13)
#set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")

project(pico2d-example)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# set source files
file(GLOB UI_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.c*)
set(UI_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR})
set(UI_RESOURCES data/girl_120x120.bmp data/star.bmp)

####################
# skeleton library
####################
add_executable(pico2d-example ${UI_SOURCES})
target_add_resources(pico2d-example ${UI_RESOURCES})
target_include_directories(pico2d-example PUBLIC ${UI_INCLUDES})
target_compile_definitions(pico2d-example PUBLIC ${UI_FLAGS})
target_link_libraries(pico2d-example ${UI_LIBS} pico2d-skeleton)

# custom linker script for 16mb flash
if (PICO_BOARD STREQUAL "waveshare_rp2040_plus_16mb")
    set_linker_flash_size_16(pico2d-example)
endif ()

if (PICO_BOARD)
    pico_add_uf2_output(pico2d-example)
endif ()
