cmake_minimum_required(VERSION 3.4 FATAL_ERROR)

project(CLI11 CXX)

SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(CUR_PROJ ON)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    # Be moderately paranoid with flags
    if(MSVC)
        add_definitions("/W4")
        add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    else()
        add_definitions("-Wall -Wextra -pedantic")
    endif()
else()
    set(CUR_PROJ OFF)
endif()


if(CMAKE_VERSION VERSION_GREATER 3.6)
# Add clang-tidy if available
    option(CLANG_TIDY_FIX "Perform fixes for Clang-Tidy (resets to OFF)" OFF)
    find_program(
        CLANG_TIDY_EXE
        NAMES "clang-tidy"
        DOC "Path to clang-tidy executable"
    )

    if(CLANG_TIDY_EXE)
        if(CLANG_TIDY_FIX)
            set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}" "-fix")
        else()
            set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}")
        endif()
    endif()
endif()


if(CMAKE_BUILD_TYPE STREQUAL Coverage)
    include(CodeCoverage)
    setup_target_for_coverage(CLI_coverage ctest coverage)
endif()

file(GLOB CLI_headers "${CMAKE_CURRENT_SOURCE_DIR}/include/CLI/*")
# To see in IDE, must be listed for target
add_library(CLI11 INTERFACE)
target_include_directories(CLI11 INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

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

    add_library(CLI11_SINGLE INTERFACE)
    target_link_libraries(CLI11_SINGLE INTERFACE CLI11)
    add_dependencies(CLI11_SINGLE generate_cli_single_file)
    target_compile_definitions(CLI11_SINGLE INTERFACE -DCLI_SINGLE_FILE)
    target_include_directories(CLI11_SINGLE INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/include/")
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

set(CLANG_TIDY_FIX OFF CACHE BOOL "Perform fixes for Clang-Tidy (resets to OFF)" FORCE)
