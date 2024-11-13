set(DRIVER_NAME aps6404)
add_library(${DRIVER_NAME} INTERFACE)

target_sources(${DRIVER_NAME} INTERFACE
        aps6404.cpp
        aps6404_perf_critical.cpp
)

pico_generate_pio_header(${DRIVER_NAME} aps6404.pio)

target_include_directories(${DRIVER_NAME} INTERFACE .)

target_link_libraries(${DRIVER_NAME} INTERFACE pico_stdlib hardware_pio hardware_dma hardware_irq)