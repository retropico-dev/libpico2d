function(add_gpio_definition flags gpio_name gpio_pin)
    if (NOT gpio_name)
        message(FATAL_ERROR "GPIO NAME not specified")
    endif ()

    # Check if the GPIO was defined via command line first
    if (DEFINED ${gpio_name})
        # Use the command line value instead
        list(APPEND ${flags} "-D${gpio_name}=${${gpio_name}}")
        message(STATUS "Using command line GPIO: ${gpio_name} => ${${gpio_name}}")
    else()
        # No command line value, use the provided pin
        if (NOT DEFINED gpio_pin)  # Changed from "if (NOT gpio_pin)"
            message(FATAL_ERROR "GPIO PIN not specified")
        endif ()

        # Set the cache variable for future cmake runs
        set(${gpio_name} "${gpio_pin}" CACHE STRING "${gpio_name}")
        list(APPEND ${flags} "-D${gpio_name}=${gpio_pin}")
        message(STATUS "Added GPIO: ${gpio_name} => ${gpio_pin}")
    endif()

    # Propagate changes to parent scope
    set(${flags} "${${flags}}" PARENT_SCOPE)
endfunction()
