cmake_minimum_required(VERSION 3.5)
#set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")

project(pico2d-example)

# set source files
file(GLOB UI_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.c*)
set(UI_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR})
set(UI_RESOURCES romfs/girl_120x120.bmp romfs/star.bmp)

####################
# skeleton library
####################
add_executable(pico2d-example ${UI_SOURCES})
target_add_resources(pico2d-example ${UI_RESOURCES})
target_include_directories(pico2d-example PUBLIC ${UI_INCLUDES})
target_compile_definitions(pico2d-example PUBLIC ${UI_FLAGS})
target_link_libraries(pico2d-example ${UI_LIBS} pico2d-skeleton)

# pico2d stuff (custom linker script, ...)
target_set_pico2d_stuff(pico2d-example)
