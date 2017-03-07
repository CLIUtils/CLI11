cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(CLI11 CXX)

SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(CUR_PROJ ON)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
else()
    set(CUR_PROJ OFF)
endif()

# Be moderately paranoid with flags
if(CMAKE_COMPILER_IS_GNUCC)
    add_definitions("-Wall -Wextra -pedantic")
endif()
if(MSVC)
    add_definitions("/W4")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_library(CLI INTERFACE)
target_include_directories(CLI INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

file(GLOB CLI_headers "${CMAKE_CURRENT_SOURCE_DIR}/include/CLI/*")
# To see in IDE, must be listed for target

# Single file test
option(CLI_SINGLE_FILE "Generate a single header file (and test)" ${CUR_PROJ})
if(CLI_SINGLE_FILE)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include")
    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/include/CLI11.hpp"
        COMMAND python "${CMAKE_CURRENT_SOURCE_DIR}/scripts/MakeSingleHeader.py" "${CMAKE_CURRENT_BINARY_DIR}/include/CLI11.hpp"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/CLI/CLI.hpp" ${CLI_headers}
        )
    add_custom_target(generate_cli_single_file ALL
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/include/CLI11.hpp")

    add_library(CLI_SINGLE INTERFACE)
    target_link_libraries(CLI_SINGLE INTERFACE CLI)
    add_dependencies(CLI_SINGLE generate_cli_single_file)
    target_compile_definitions(CLI_SINGLE INTERFACE -DCLI_SINGLE_FILE)
    target_include_directories(CLI_SINGLE INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/include/")
endif()

option(CLI_SINGLE_FILE_TESTS "Duplicate all the tests for a single file build" OFF)

option(CLI_TESTING "Build the tests and add them" ${CUR_PROJ})
if(CLI_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

option(CLI_EXAMPLES "Build the examples" ${CUR_PROJ})
if(CLI_EXAMPLES)
    add_subdirectory(examples)
endif()

option(CLI_COVERAGE "Add coverage report" OFF)
if(CLI_COVERAGE)
    include(AddCoverage)
    set(COVERAGE_SRCS ${CLI_headers})
    coveralls_setup("${COVERAGE_SRCS}" ON)
endif()
