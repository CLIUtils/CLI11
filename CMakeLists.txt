cmake_minimum_required(VERSION 3.4 FATAL_ERROR)

project(CLI11 CXX)

SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

set(CLI_CXX_STD "11"  CACHE STRING "The CMake standard to require")

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(CUR_PROJ ON)
    set(CMAKE_CXX_STANDARD ${CLI_CXX_STD})
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    # Be moderately paranoid with flags
    if(MSVC)
        add_definitions("/W4")
    else()
        add_definitions("-Wall -Wextra -pedantic")
    endif()
else()
    set(CUR_PROJ OFF)
endif()

# Allow IDE's to group targets into folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(CMAKE_VERSION VERSION_GREATER 3.6)
# Add clang-tidy if available
    option(CLANG_TIDY_FIX "Perform fixes for Clang-Tidy" OFF)
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
if (CUR_PROJ)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/CLI DESTINATION include)
endif()

# Single file test
find_package(PythonInterp)
if(CUR_PROJ AND PYTHONINTERP_FOUND)
    set(CLI_SINGLE_FILE_DEFAULT ON)
else()
    set(CLI_SINGLE_FILE_DEFAULT OFF)
endif()
option(CLI_SINGLE_FILE "Generate a single header file (and test)" ${CLI_SINGLE_FILE_DEFAULT})
if(CLI_SINGLE_FILE)
    find_package(PythonInterp REQUIRED)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include")
    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/include/CLI11.hpp"
        COMMAND "${PYTHON_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/MakeSingleHeader.py" "${CMAKE_CURRENT_BINARY_DIR}/include/CLI11.hpp"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/CLI/CLI.hpp" ${CLI_headers}
        )
    add_custom_target(generate_cli_single_file ALL
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/include/CLI11.hpp")
    set_target_properties(generate_cli_single_file
                          PROPERTIES FOLDER "Scripts")
    if (CUR_PROJ)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/CLI11.hpp DESTINATION include)
    endif()
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
