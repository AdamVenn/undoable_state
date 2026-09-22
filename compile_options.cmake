if(MSVC)
    add_compile_options()

    add_compile_options(
        /W4
        
        /we4172 # Return stack address
        /we4715 # Not all control paths return value
        /we4700 # Uninitialized local variable used
    )
else()
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wsign-conversion
        -Wuninitialized
        -Wunused-parameter

        -Werror=uninitialized # Uninitialized variable used
        -Werror=return-local-addr # Return stack address
        -Werror=return-type # Not all control paths return value
    )
endif()