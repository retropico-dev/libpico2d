cmake_minimum_required(VERSION 3.13)
#set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")

project(pico2d-example)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# set source files
file(GLOB UI_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.c*)
set(UI_INCLUDES .)
set(UI_FLAGS -DMB_UI)

# resources
# Creates C resources file from files in given directory
function(create_resources dir output)
    # Create empty output file
    file(WRITE ${output} "")
    # Collect input files
    file(GLOB bins ${dir}/*)
    # Iterate through input files
    foreach (bin ${bins})
        # Get short filename
        string(REGEX MATCH "([^/]+)$" filename ${bin})
        # Replace filename spaces & extension separator for C compatibility
        string(REGEX REPLACE "\\.| |-" "_" filename ${filename})
        # Read hex data from file
        file(READ ${bin} filedata HEX)
        # Convert hex data for C compatibility
        string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," filedata ${filedata})
        # Append data to output file
        file(APPEND ${output} "const unsigned char ${filename}[] = {${filedata}};\nconst unsigned ${filename}_size = sizeof(${filename});\n")
    endforeach ()
endfunction()

create_resources(data ${CMAKE_BINARY_DIR}/data.c)

####################
# skeleton library
####################
add_executable(pico2d-example ${UI_SOURCES} ${CMAKE_BINARY_DIR}/data.c)
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
