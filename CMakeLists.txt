cmake_minimum_required(VERSION 3.25)
project(UNDOABLE_STATE VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES arm64 x86_64)

include("compile_options.cmake")

add_library(undoable_state_lib INTERFACE)

target_sources(undoable_state_lib INTERFACE
    undoable_state.hpp
)

target_include_directories(undoable_state_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

set_source_files_properties(
    undoable_state.hpp
    PROPERTIES
    HEADER_FILE_ONLY TRUE
)

add_executable(undoable_state_test test/unit_test.cpp)
target_link_libraries(undoable_state_test PRIVATE undoable_state_lib)

enable_testing()
add_test(NAME undoable_state_test COMMAND undoable_state_test)

