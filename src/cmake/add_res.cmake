function(add_resource_binary out_var)
    set(result)
    foreach (in_f ${ARGN})
        set(out_f "${PROJECT_BINARY_DIR}/${in_f}.o")
        get_filename_component(in_path ${CMAKE_CURRENT_SOURCE_DIR}/${in_f} DIRECTORY)
        get_filename_component(in_file ${CMAKE_CURRENT_SOURCE_DIR}/${in_f} NAME)
        add_custom_command(OUTPUT ${out_f}
                WORKING_DIRECTORY ${in_path}
                COMMAND ${CMAKE_LINKER} -r -b binary -o ${out_f} ${in_file}
                DEPENDS ${in_f}
                COMMENT "Building binary object ${out_f}"
                VERBATIM
        )
        list(APPEND result ${out_f})
    endforeach ()
    set(${out_var} "${result}" PARENT_SCOPE)
endfunction()
